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

#ifndef COCOA_GALLIUM_INSPECTOR_H
#define COCOA_GALLIUM_INSPECTOR_H

#include <utility>
#include <string>

#include "include/v8.h"
#include "uv.h"

#include "Core/ConcurrentTaskQueue.h"
#include "Gallium/Gallium.h"
GALLIUM_NS_BEGIN

class InspectorThread;
class InspectorClient;

class Inspector
{
public:
    struct AsyncMessageDelivery
    {
        enum Type
        {
            kDisconnected,
            kMessage
        };
        explicit AsyncMessageDelivery(std::string message)
            : type_(kMessage), message_(std::move(message)) {}
        AsyncMessageDelivery()
            : type_(kDisconnected) {}
        AsyncMessageDelivery(AsyncMessageDelivery&& rhs) noexcept
            : type_(rhs.type_), message_(std::move(rhs.message_)) {}

        Type type_;
        std::string message_;
    };

    Inspector(uv_loop_t *loop,
              v8::Isolate *isolate,
              v8::Local<v8::Context> context,
              int32_t port);
    ~Inspector();

    void WaitForConnection();

    void SendMessageToFrontend(const std::string& message);
    std::string WaitAndTakeFrontendMessage();

    void ScheduleModuleEvaluation(const std::string& url);

private:
    void DisconnectedFromFrontend();

    static void AsyncHandler(uv_async_t *async);

    void AsyncMessageCallback(const std::string& message);
    void DisconnectedCallback();
    void ConnectedCallback();

    uv_loop_t *event_loop_;
    v8::Isolate *isolate_;
    v8::Global<v8::Context> context_;
    std::unique_ptr<InspectorThread> io_thread_;
    std::unique_ptr<InspectorClient> client_;
    uv_async_t async_handle_;
    ConcurrentTaskQueue<AsyncMessageDelivery> message_queue_;
    bool has_connected_;
    uv_barrier_t connected_barrier_;
    std::string scheduled_module_eval_url_;
};

GALLIUM_NS_END
#endif //COCOA_INSPECTOR_H
