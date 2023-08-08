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

#ifndef COCOA_GALLIUM_BINDINGS_WORKERS_MESSAGEPORT_H
#define COCOA_GALLIUM_BINDINGS_WORKERS_MESSAGEPORT_H

#include <utility>
#include <queue>
#include <mutex>

#include "include/v8.h"
#include "uv.h"

#include "Core/EventLoop.h"
#include "Gallium/Gallium.h"
#include "Gallium/bindings/workers/Types.h"
#include "Gallium/bindings/ExportableObjectBase.h"
GALLIUM_BINDINGS_WORKERS_NS_BEGIN

class MessagePort
{
public:
    struct Message
    {
        template<typename T>
        using SpVec = std::vector<std::shared_ptr<T>>;

        using PayloadDeleter = std::function<void(const uint8_t*)>;
        using PayloadArray = std::unique_ptr<const uint8_t[], PayloadDeleter>;

        PayloadArray                                payload;
        size_t                                      payload_size;
        SpVec<v8::BackingStore>                     array_buffers;
        SpVec<v8::BackingStore>                     shared_array_buffers;
        std::vector<v8::CompiledWasmModule>         wasm_modules;
        SpVec<ExportableObjectBase::FlattenedData>  flattened_objects;
    };

    using ReceiveCallback = std::function<void(v8::Local<v8::Value>)>;
    using ErrorCallback = std::function<void(const std::string&)>;

    template<typename T>
    using SamePair = std::pair<T, T>;
    using PortPair = SamePair<std::shared_ptr<MessagePort>>;

    /**
     * Create a pair of connected message ports attaching to the
     * specified event loop. The two message ports are completely
     * equivalent (symmetric). Both of them can post messages to
     * and receive messages from each other.
     *
     * @param loop Event loop to detach to; maybe `nullptr` to create
     *             a detached pair of message ports.
     */
    static PortPair MakeConnectedPair(uv_loop_t *loop);

    explicit MessagePort(uv_loop_t *loop);
    ~MessagePort() = default;

    void SetReceiveCallback(ReceiveCallback callback);
    void SetErrorCallback(ErrorCallback callback);

    g_nodiscard g_inline bool IsDetached() const {
        return port_detached_;
    }

    /**
     * Send a message, with optional transferable objects, to the peer
     * message port.
     */
    v8::Maybe<bool> PostMessage(v8::Local<v8::Value> message,
                                const std::vector<v8::Local<v8::Value>>& transfer_list);

    /**
     * Detach current message port from its attached event loop,
     * not affecting its peer message port. Once a port is detached,
     * messages that are delivered to this port will be dropped until
     * it is attached to a new event loop via `AttachToEventLoop()`.
     * This operation does NOT clear registered callbacks.
     */
    void DetachFromEventLoop();

    /**
     * Attach current message port to a specified event loop,
     * for detached message port only, not affecting its peer port.
     *
     * Attached event loop affects the thread where the `ReceiveCallback`
     * will be called. `ReceiveCallback` will be called on the thread
     * that runs the attached event loop. An `async` handle will be
     * added to the event loop, preventing it exiting until message port
     * is detached from the event loop or destructed.
     */
    bool AttachToEventLoop(uv_loop_t *event_loop);

private:
    static bool PostSerializedMessage(const std::shared_ptr<MessagePort>& peer,
                                      std::unique_ptr<Message> message);

    void OnMessageReceive();
    void ReceiveSerializedMessage(Message& message);
    void HandleCaughtError(v8::Isolate *isolate, v8::TryCatch& try_catch);

    bool                                    port_detached_;
    std::weak_ptr<MessagePort>              peer_port_;
    std::optional<uv::AsyncHandle>          message_notifier_;
    std::queue<std::unique_ptr<Message>>    recv_queue_;
    std::mutex                              queue_lock_;
    ReceiveCallback                         receive_callback_;
    ErrorCallback                           error_callback_;
};

GALLIUM_BINDINGS_WORKERS_NS_END
#endif //COCOA_GALLIUM_BINDINGS_WORKERS_MESSAGEPORT_H
