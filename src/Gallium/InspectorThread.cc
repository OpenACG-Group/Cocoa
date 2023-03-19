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

#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Gallium/InspectorThread.h"
GALLIUM_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.InspectorThread)

InspectorThread::InspectorThread(int32_t port,
                                 MessageNotifier message_notifier,
                                 DisconnectNotifier disconnect_notifier,
                                 ConnectNotifier connect_notifier)
    : port_(port)
    , thread_(&InspectorThread::IoThreadRoutine, this)
    , message_notifier_(std::move(message_notifier))
    , disconnect_notifier_(std::move(disconnect_notifier))
    , connect_notifier_(std::move(connect_notifier))
    , disconnected_(false)
    , message_queue_async_{}
{
}

InspectorThread::~InspectorThread()
{
    if (thread_.joinable())
        thread_.join();
}

void InspectorThread::SendFrontendMessage(const std::string& message)
{
    if (disconnected_)
        return;

    // As the libwebsocket requires, we MUST have LWS_PRE bytes available
    // BEFORE the actual data. Those bytes will be used for protocol header
    // by libwebsocket internally.
    auto buffer = std::make_unique<uint8_t[]>(message.length() + LWS_PRE);
    std::memcpy(buffer.get() + LWS_PRE, message.data(), message.length());

    // Push the message into queue, and it will be sent when the socket is writable
    // automatically by libwebsocket.
    send_queue_.Push(std::make_unique<SendBuffer>(std::move(buffer), message.length()));

    uv_async_send(&message_queue_async_);
}

namespace {

struct WsContext
{
    ConcurrentTaskQueue<InspectorThread::SendBuffer> *message_queue;
    uv_loop_t loop;
    lws_context *lws_context;
    InspectorThread *self;
    lws *current_wsi;

    // Indicate that the main thread has notified us to send a message,
    // but the `current_wsi` was not available.
    bool has_pending_write_req;
};

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

    ws_context->current_wsi = wsi;
    if (ws_context->has_pending_write_req)
    {
        lws_callback_on_writable(wsi);
        ws_context->has_pending_write_req = false;
    }

    switch (reason)
    {
    case LWS_CALLBACK_ESTABLISHED:
        ws_context->self->NotifyConnected();
        lws_callback_on_writable(wsi);
        break;

    case LWS_CALLBACK_RECEIVE:
        ws_context->self->NotifyMessage(std::string(reinterpret_cast<const char*>(in), len));
        break;

    case LWS_CALLBACK_CLOSED:
        ws_context->self->NotifyDisconnected();
        lws_context_destroy(ws_context->lws_context);
        break;

    case LWS_CALLBACK_SERVER_WRITEABLE:
        while (auto message = ws_context->message_queue->Pop())
        {
            int sent = lws_write(wsi, message->ptr.get() + LWS_PRE, message->data_size, LWS_WRITE_TEXT);
            if (sent != message->data_size)
            {
                QLOG(LOG_ERROR, "Websocket: Failed to send message to the inspector frontend");
            }
        }
        break;

    default:
        break;
    }

    return 0;
}

lws_protocols g_lws_protocols[] = {
    {
        .name = "/",
        .callback = ws_protocol_callback,
        .per_session_data_size = 0,
        // 0 for unlimited size
        .rx_buffer_size = 0
    },

    // As terminator
    {nullptr, nullptr, 0, 0}
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

void InspectorThread::IoThreadRoutine()
{
    pthread_setname_np(pthread_self(), "Inspector I/O");

    WsContext ws_context{};

    setup_websockets_context(ws_context, port_);
    ws_context.message_queue = &send_queue_;
    ws_context.self = this;
    ws_context.current_wsi = nullptr;

    uv_async_init(&ws_context.loop, &message_queue_async_, [](uv_async_t *async) {
        CHECK(async && async->data);
        auto *context = reinterpret_cast<WsContext*>(async->data);
        if (context->current_wsi)
            lws_callback_on_writable(context->current_wsi);
        else
            context->has_pending_write_req = true;
    });
    message_queue_async_.data = &ws_context;

    // The main thread should keep blocking until the connection established.
    // Main threads will enter blocked state by `Inspector::WaitForConnection`,
    // which will wait for the thread barrier.

    lws_service(ws_context.lws_context, 0);

    uv_loop_close(&ws_context.loop);

    lws_context_destroy(ws_context.lws_context);

    QLOG(LOG_INFO, "Inspector I/O thread has been exited");
}

void InspectorThread::NotifyConnected()
{
    connect_notifier_();
}

void InspectorThread::NotifyDisconnected()
{
    disconnect_notifier_();
    disconnected_ = true;
    uv_close(reinterpret_cast<uv_handle_t*>(&message_queue_async_),
             [](uv_handle_t *) {});
}

void InspectorThread::NotifyMessage(const std::string& message)
{
    message_notifier_(message);
}

GALLIUM_NS_END
