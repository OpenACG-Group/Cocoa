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

#include "json/json.h"

#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Gallium/Runtime.h"
#include "Gallium/Inspector.h"
#include "Gallium/InspectorThread.h"
#include "Gallium/InspectorClient.h"
#include "Gallium/InspectorChannel.h"
GALLIUM_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.Inspector)

Inspector::Inspector(uv_loop_t *loop,
                     v8::Isolate *isolate,
                     v8::Local<v8::Context> context,
                     int32_t port)
    : event_loop_(loop)
    , isolate_(isolate)
    , async_handle_{}
    , has_connected_(false)
    , connected_barrier_{}
{
    CHECK(event_loop_ && isolate_);

    context_.Reset(isolate_, context);

    // Main thread and the inspector thread will reach this barrier.
    uv_barrier_init(&connected_barrier_, 2);

    uv_async_init(event_loop_, &async_handle_, AsyncHandler);
    async_handle_.data = this;

    // Create `InspectorThread` starts the inspector thread.
    // The inspector thread will start a WebSocket server and wait for connection
    // asynchronously. When a connection comes, we will be notified by `ConnectedCallback`
    // function.
    io_thread_ = std::make_unique<InspectorThread>(
        port,
        [this](const std::string& message) { this->AsyncMessageCallback(message); },
        [this]() { this->DisconnectedCallback(); },
        [this]() { this->ConnectedCallback(); }
    );

    client_ = std::make_unique<InspectorClient>(isolate, context, this);

    QLOG(LOG_INFO, "Started V8 inspector, listening on ws://127.0.0.1:{}", port);
}

Inspector::~Inspector()
{
}

void Inspector::WaitForConnection()
{
    if (has_connected_)
        return;

    QLOG(LOG_INFO, "Inspector is waiting for connection from frontend");
    uv_barrier_wait(&connected_barrier_);
    has_connected_ = true;
}

void Inspector::ConnectedCallback()
{
    // Called in the inspector IO thread.

    uv_barrier_wait(&connected_barrier_);
    QLOG(LOG_INFO, "Connected with inspector frontend, debugging was started");
}

void Inspector::AsyncMessageCallback(const std::string& message)
{
    // Called in the inspector IO thread.

    message_queue_.Push(std::make_unique<AsyncMessageDelivery>(message));

    // The main thread may process messages by explicitly waiting on the queue,
    // and it consumes all the messages in the queue.
    // When main thread later enters the event loop, it will be notified again
    // by `AsyncHandler` callback, but it is fine as the event queue has been emptied.
    uv_async_send(&async_handle_);
}

void Inspector::DisconnectedCallback()
{
    // Called in the inspector thread.

    message_queue_.Push(std::make_unique<AsyncMessageDelivery>());
    uv_async_send(&async_handle_);
}

namespace {

bool should_evaluate_startup_script(const std::string& message)
{
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(message, root, false))
    {
        // FIXME(sora): Is there any better way to handle parsing error?
        return false;
    }

    Json::Value& method = root["method"];
    if (!method.isString())
        return false;

    return (method.asString() == "Runtime.runIfWaitingForDebugger");
}

} // namespace anonymous

void Inspector::AsyncHandler(uv_async_t *async)
{
    CHECK(async && async->data);
    auto *inspector = reinterpret_cast<Inspector*>(async->data);

    while (auto front = inspector->message_queue_.Pop())
    {
        if (front->type_ == AsyncMessageDelivery::kDisconnected)
        {
            inspector->DisconnectedFromFrontend();
            break;
        }

        inspector->client_->DispatchMessage(front->message_);
        if (!inspector->has_connected_)
            break;

        // Perform the scheduled evaluation of startup script.
        if (!inspector->scheduled_module_eval_url_.empty() &&
            should_evaluate_startup_script(front->message_))
        {
            std::string url = inspector->scheduled_module_eval_url_;
            Runtime *runtime = Runtime::GetBareFromIsolate(inspector->isolate_);
            if (runtime->getOptions().inspector_startup_brk)
            {
                QLOG(LOG_INFO, "Inspector inserted a startup-breakpoint automatically");
                inspector->client_->SchedulePauseOnNextStatement("startup");
            }

            try
            {
                v8::HandleScope handle_scope(inspector->isolate_);
                v8::Context::Scope context_scope(inspector->isolate_->GetCurrentContext());

                v8::Local<v8::Value> result;
                bool success = runtime->EvaluateModule(url).ToLocal(&result);
                if (!success)
                    throw std::runtime_error("Failed to evaluate module " + url);
            }
            catch (const std::exception& e)
            {
                QLOG(LOG_ERROR, "An exception occurred when evaluating module {}: {}",
                     inspector->scheduled_module_eval_url_, e.what());
                // TODO(sora): error handling
            }

            // Startup script should be evaluated only once
            inspector->scheduled_module_eval_url_.clear();
        }
    }
}

void Inspector::DisconnectedFromFrontend()
{
    has_connected_ = false;
    client_->DisconnectedFromFrontend();

    context_.Reset();

    io_thread_.reset();
    client_.reset();

    uv_barrier_destroy(&connected_barrier_);
    uv_close(reinterpret_cast<uv_handle_t*>(&async_handle_),
             [](uv_handle_t*) {});

    QLOG(LOG_INFO, "Inspector frontend has disconnected");
}

std::string Inspector::WaitAndTakeFrontendMessage()
{
    std::unique_ptr<AsyncMessageDelivery> message = message_queue_.WaitPop();

    if (message->type_ == AsyncMessageDelivery::kDisconnected)
        DisconnectedFromFrontend();

    return std::move(message->message_);
}

void Inspector::SendMessageToFrontend(const std::string& message)
{
    // fmt::print("\033[33;1mSend:\033[0m {}\n", message);
    io_thread_->SendFrontendMessage(message);
}

void Inspector::ScheduleModuleEvaluation(const std::string& url)
{
    CHECK(!url.empty());
    scheduled_module_eval_url_ = url;
}

GALLIUM_NS_END
