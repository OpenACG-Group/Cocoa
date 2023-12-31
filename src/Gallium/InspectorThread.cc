/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */
 
#include <utility>
#include <cstring>

#include "uv.h"
#include "libwebsockets.h"
#include "fmt/format.h"
#include "json/json.h"

#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Core/ApplicationInfo.h"
#include "Core/Data.h"
#include "Core/UUIDGenerator.h"
#include "Gallium/InspectorThread.h"
#include "Gallium/ModuleImportURL.h"
#include "CRPKG/ResourceManager.h"
#include "CRPKG/VirtualDisk.h"
GALLIUM_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.InspectorThread)

struct InspectorThread::ThreadInitInfo
{
    ThreadInitInfo() : ready_sem{} { uv_sem_init(&ready_sem, 0); }

    void Post() { uv_sem_post(&ready_sem); }
    void PostError(const std::string& error_) {
        error = error_;
        uv_sem_post(&ready_sem);
    }

    std::optional<std::string> Wait() {
        uv_sem_wait(&ready_sem);
        return error;
    }

    std::optional<std::string> error;
    uv_sem_t ready_sem;
};

InspectorThread::MessageBuffer::Ptr
InspectorThread::MessageBuffer::Allocate(Type type, size_t payload_size)
{
    size_t size = sizeof(MessageBuffer) + payload_size;
    auto *addr = static_cast<MessageBuffer*>(malloc(size));
    CHECK(addr && "Memory allocation failed");
    addr->type = type;
    addr->payload_size = payload_size;

    // A unique_ptr with deleter.
    return { addr, +[](MessageBuffer *ptr) { free(ptr); } };
}

std::unique_ptr<InspectorThread> InspectorThread::Start(uv_loop_t *loop,
                                                        int32_t port,
                                                        EventHandler *handler)
{
    CHECK(handler);
    auto thread = std::make_unique<InspectorThread>(loop, port, handler);

    ThreadInitInfo init_info;
    thread->thread_ = std::thread(&InspectorThread::IoThreadRoutine,
                                  thread.get(), &init_info);
    if (auto error = init_info.Wait())
    {
        QLOG(LOG_ERROR, "Failed to start inspector IO thread: {}", *error);
        return nullptr;
    }

    return thread;
}

InspectorThread::InspectorThread(uv_loop_t *loop, int32_t port, EventHandler *handler)
    : disposed_(false)
    , event_loop_(loop)
    , port_(port)
    , event_handler_(handler)
    , has_active_session_(false)
    , recv_queue_(loop, [this](MessageBuffer::Ptr buf, MessageQueue*) {
        // trampoline function
        OnMainThreadRecvMessage(std::move(buf));
    })
{
}

InspectorThread::~InspectorThread()
{
    Dispose();
}

void InspectorThread::Dispose()
{
    if (disposed_)
        return;

    // Notify the thread to exit
    if (send_queue_)
        send_queue_->Enqueue(MessageBuffer::Allocate(MessageBuffer::kExit, 0));

    recv_queue_.SetNonBlocking(true);
    if (thread_.joinable())
        thread_.join();
}

void InspectorThread::Send(const std::string& message)
{
    if (!has_active_session_ || !send_queue_)
        return;

    // As the libwebsocket requires, we MUST have LWS_PRE bytes available
    // BEFORE the actual data. Those bytes will be used for protocol header
    // by libwebsocket internally.
    auto buf = MessageBuffer::Allocate(MessageBuffer::kPayload, LWS_PRE + message.length());
    std::memcpy(buf->payload + LWS_PRE, message.data(), message.length());

    send_queue_->Enqueue(std::move(buf));
}

namespace {

struct WsContext
{
    int port;
    uv_loop_t loop;
    lws_context *lws_context;
    InspectorThread *self;

    InspectorThread::MessageQueue *main_thread_recv_queue;

    // Owned and maintained by the IO thread, not shared with any other
    // threads. The messages coming from `InspectorThread::send_queue_`
    // will be transferred into this queue.
    std::queue<InspectorThread::MessageBuffer::Ptr> write_queue;

    std::string session_uuid;

    // Current active WebSocket connection with the debugger frontend,
    // `nullptr` if there is no active connection now.
    lws *current_ws_conn;
};

std::tuple<int, std::string_view> http_get_uri_and_method(struct lws *wsi)
{
    char *uri;
    int length;
    int method = lws_http_get_uri_and_method(wsi, &uri, &length);
    if (method < 0)
        return {-1, {}};
    return {method, std::string_view(uri, length)};
}

int confirm_http_protocol_upgrade(struct lws *wsi, WsContext *ctx)
{
    CHECK(wsi);
    CHECK(ctx);

    auto reject_response = [wsi](const std::string_view& reason) {
        auto resp_buf = std::make_unique<uint8_t[]>(LWS_PRE + 512);
        uint8_t *buf_start = resp_buf.get() + LWS_PRE, *buf_end = buf_start + 512;
        uint8_t *buf_cur = buf_start;
        // https://developer.mozilla.org/en-US/docs/Web/HTTP/Protocol_upgrade_mechanism
        // The server should send a regular response to reject the protocol upgrading
        // request.
        // Here we send a 200 OK response with a string representing the reason why
        // the request is rejected.
        CHECK(lws_add_http_common_headers(wsi, 200, "text/plain",
                                          reason.length(), &buf_cur, buf_end) >= 0);
        CHECK(lws_finalize_http_header(wsi, &buf_cur, buf_end) >= 0);
        CHECK(reason.length() <= buf_end - buf_cur);
        std::memcpy(buf_cur, reason.data(), reason.length());
        buf_cur += reason.length();

        lws_write(wsi, buf_start, buf_cur - buf_start, LWS_WRITE_HTTP);
        return 1;
    };

    if (ctx->current_ws_conn)
        return reject_response("There is already an active debugging session");

    // Memory is managed by lws, so the pointer does not need freeing.
    auto [req_method, req_uri] = http_get_uri_and_method(wsi);
    if (req_method < 0)
        return reject_response("Request missing URL or method");
    if (req_method != LWSHUMETH_GET)
        return reject_response("Request has wrong method");

    if (req_uri != "/" + ctx->session_uuid)
        return reject_response("Invalid WebSocket URL");

    // Accept the upgrading request. Lws will handle the rest of things automatically.
    return 0;
}

void handle_normal_http_request(struct lws *wsi, WsContext *ctx)
{
    CHECK(wsi);
    CHECK(ctx);

    auto respond = [wsi](int code, const char *content_type,
                         const std::shared_ptr<Data>& data) {
        size_t buf_total_size = LWS_PRE + 256;
        if (data)
            buf_total_size += data->size();
        auto resp_buf = std::make_unique<uint8_t[]>(buf_total_size);
        uint8_t *buf_start = resp_buf.get() + LWS_PRE, *buf_end = buf_start + buf_total_size;
        uint8_t *buf_cur = buf_start;

        if (!data)
            CHECK(lws_add_http_header_status(wsi, code, &buf_cur, buf_end) >= 0);
        else
        {
            CHECK(lws_add_http_common_headers(wsi, code, content_type,
                                              data->size(), &buf_cur, buf_end) >= 0);
        }
        CHECK(lws_finalize_http_header(wsi, &buf_cur, buf_end) >= 0);

        if (data)
        {
            CHECK(data->size() <= buf_end - buf_cur);
            data->read(buf_cur, data->size());
            buf_cur += data->size();
        }

        lws_write(wsi, buf_start, buf_cur - buf_start, LWS_WRITE_HTTP);
    };

    auto [req_method, req_uri] = http_get_uri_and_method(wsi);
    if (req_method != LWSHUMETH_GET)
    {
        respond(HTTP_STATUS_BAD_REQUEST, nullptr, {});
        return;
    }

    // https://chromedevtools.github.io/devtools-protocol/
    // Implement the HTTP endpoint of the devtools protocol, which is used to provide
    // information for debug frontends like VSCode and Chrome DevTools.
    if (req_uri == "/json/version")
    {
        Json::Value obj;
        obj["Browser"] = "Cocoa";
        obj["Protocol-Version"] = "1.1";

        Json::FastWriter writer;
        respond(HTTP_STATUS_OK, "application/json; charset=UTF-8",
                Data::MakeFromString(writer.write(obj), true));
    }
    else if (req_uri == "/json" || req_uri == "/json/list")
    {
        Json::Value obj;
        obj["description"] = "Cocoa instance";
        obj["faviconUrl"] = "http://localhost:{}/favicon";

        // Used to lead Chrome to open the DevTools page
        obj["devtoolsFrontendUrl"] = fmt::format(
                "devtools://devtools/bundled/js_app.html?experiments=true&v8only=true&ws=localhost:{}/{}",
                ctx->port, ctx->session_uuid);
        obj["devtoolsFrontendUrlCompat"] = fmt::format(
                "devtools://devtools/bundled/inspector.html?experiments=true&v8only=true&ws=localhost:{}/{}",
                ctx->port, ctx->session_uuid);

        obj["id"] = ctx->session_uuid;

        // Debugger frontend will not use the given URL to load scripts.
        // V8 directly deliveries the script contents via the inspector protocol.
        const std::string& script_name = ApplicationInfo::Ref().js_first_script_name;
        obj["title"] = fmt::format("Cocoa [{}]", script_name);
        auto script_resolved = ModuleImportURL::Resolve(
                nullptr, script_name, ModuleImportURL::ResolvedAs::kUserExecute);
        if (script_resolved)
            obj["url"] = script_resolved->toString();

        obj["type"] = "node";
        obj["webSocketDebuggerUrl"] = fmt::format("ws://localhost:9005/{}", ctx->session_uuid);

        Json::Value root(Json::ValueType::arrayValue);
        root.append(obj);

        Json::FastWriter writer;
        respond(HTTP_STATUS_OK, "application/json; charset=UTF-8",
                Data::MakeFromString(writer.write(root), true));
    }
    else if (req_uri == "/favicon" || req_uri == "/favicon.ico")
    {
        auto vfs = crpkg::ResourceManager::Ref().GetResource("@internal");
        auto storage = vfs->GetStorage("/favicon.ico");
        CHECK(storage.has_value());
        respond(HTTP_STATUS_OK, "image/vnd.microsoft.icon",
                Data::MakeFromPtrWithoutCopy(
                        const_cast<uint8_t*>(storage->addr), storage->size, false));
    }
    else
    {
        respond(HTTP_STATUS_NOT_FOUND, nullptr, {});
        return;
    }
}

// NOLINTNEXTLINE
int ws_protocol_callback(struct lws *wsi,
                         enum lws_callback_reasons reason,
                         void *user,
                         void *in,
                         size_t len)
{
    if (!wsi)
        return 0;

    lws_context *ctx = lws_get_context(wsi);
    auto *ws_context = reinterpret_cast<WsContext*>(lws_context_user(ctx));

    using MsgBuffer = InspectorThread::MessageBuffer;

    switch (reason)
    {
    case LWS_CALLBACK_HTTP_CONFIRM_UPGRADE:
        return confirm_http_protocol_upgrade(wsi, ws_context);

    case LWS_CALLBACK_HTTP:
        handle_normal_http_request(wsi, ws_context);
        // To close and free up the connection
        return 1;

    case LWS_CALLBACK_ESTABLISHED:
        ws_context->current_ws_conn = wsi;
        if (!ws_context->write_queue.empty())
            lws_callback_on_writable(wsi);
        ws_context->main_thread_recv_queue->Enqueue(
                MsgBuffer::Allocate(MsgBuffer::kConnect, 0));
        break;

    case LWS_CALLBACK_RECEIVE:
    {
        auto buf = MsgBuffer::Allocate(MsgBuffer::kPayload, len);
        std::memcpy(buf->payload, in, len);
        ws_context->main_thread_recv_queue->Enqueue(std::move(buf));
        break;
    }

    case LWS_CALLBACK_CLOSED:
        ws_context->current_ws_conn = nullptr;
        ws_context->main_thread_recv_queue->Enqueue(
                MsgBuffer::Allocate(MsgBuffer::kDisconnect, 0));
        break;

    case LWS_CALLBACK_SERVER_WRITEABLE:
        while (!ws_context->write_queue.empty())
        {
            auto msg = std::move(ws_context->write_queue.front());
            ws_context->write_queue.pop();

            size_t data_size = msg->payload_size - LWS_PRE;
            int sent = lws_write(wsi, msg->payload + LWS_PRE, data_size, LWS_WRITE_TEXT);
            if (sent != data_size)
                QLOG(LOG_ERROR, "WebSocket: Failed to send message to the inspector frontend");
        }
        break;

    default:
        break;
    }

    return 0;
}

lws_protocols g_lws_protocols[] = {
    { "ws", ws_protocol_callback, 0, 0, 0, nullptr, 0 },
    LWS_PROTOCOL_LIST_TERM
};

bool setup_websockets_context(WsContext &ctx, int port)
{
    lws_set_log_level(0, nullptr);

    uv_loop_init(&ctx.loop);

    lws_context_creation_info info{};
    std::memset(&info, 0, sizeof(info));

    info.user = &ctx;
    info.port = port;
    info.iface = nullptr;
    info.protocols = g_lws_protocols;
    info.gid = -1;
    info.uid = -1;
    info.options = LWS_SERVER_OPTION_LIBUV;

    void *foreign_loops[1] = { &ctx.loop };
    info.foreign_loops = foreign_loops;

    // 60 seconds util connection is suspicious
    info.ka_time = 60;
    // 10 probes after `info.ka_time` seconds
    info.ka_probes = 10;
    // 10s interval for sending probes
    info.ka_interval = 10;

    ctx.lws_context = lws_create_context(&info);
    if (!ctx.lws_context)
        return false;

    return true;
}

} // namespace anonymous

void InspectorThread::IoThreadRoutine(ThreadInitInfo *thread_init_info)
{
    pthread_setname_np(pthread_self(), "InspectorIO");

    WsContext ws_context{};
    ws_context.session_uuid = GenerateRandomUUID();
    ws_context.port = port_;

    if (!setup_websockets_context(ws_context, port_))
    {
        thread_init_info->PostError("failed to initialize libwebsockets");
        return;
    }
    ws_context.self = this;
    ws_context.current_ws_conn = nullptr;
    ws_context.main_thread_recv_queue = &recv_queue_;

    send_queue_.emplace(&ws_context.loop, [&ws_context](MessageBuffer::Ptr msg, MessageQueue *queue) {
        if (msg->type == MessageBuffer::kExit)
        {
            CHECK(msg->payload_size == 0);
            lws_context_destroy(ws_context.lws_context);
            queue->SetNonBlocking(true);
            return;
        }

        CHECK(msg->type == MessageBuffer::kPayload);

        ws_context.write_queue.emplace(std::move(msg));

        // Notify LWS that we want ot write something.
        // Later the protocol callback will be called with `LWS_CALLBACK_SERVER_WRITEABLE`.
        // Data filled into `write_queue` will be sent to the client (debugger frontend)
        // by that callback.
        if (ws_context.current_ws_conn)
            lws_callback_on_writable(ws_context.current_ws_conn);
    });

    thread_init_info->Post();
    // Never use this pointer anymore as it has been a dangling pointer.
    thread_init_info = nullptr;

    lws_service(ws_context.lws_context, 0);

    send_queue_.reset();
    uv_loop_close(&ws_context.loop);
    QLOG(LOG_INFO, "Inspector IO thread has exited");
}

void InspectorThread::OnMainThreadRecvMessage(MessageBuffer::Ptr buffer)
{
    switch (buffer->type)
    {
    case MessageBuffer::kConnect:
        has_active_session_ = true;
        recv_queue_.SetNonBlocking(false);
        event_handler_->OnConnect();
        break;

    case MessageBuffer::kDisconnect:
        has_active_session_ = false;
        recv_queue_.SetNonBlocking(true);
        event_handler_->OnDisconnect();
        break;

    case MessageBuffer::kPayload:
        event_handler_->OnMessage(std::move(buffer));
        break;

    default:
        MARK_UNREACHABLE("unexpected message type");
    }
}

void InspectorThread::WaitOnce()
{
    OnMainThreadRecvMessage(recv_queue_.WaitOnce());
}

GALLIUM_NS_END
