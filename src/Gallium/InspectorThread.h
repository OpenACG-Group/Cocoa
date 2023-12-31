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

#ifndef COCOA_GALLIUM_INSPECTORTHREAD_H
#define COCOA_GALLIUM_INSPECTORTHREAD_H

#include <thread>
#include <functional>
#include <atomic>

#include "Core/AsyncMessageQueue.h"
#include "Core/EventLoop.h"
#include "Gallium/Gallium.h"
GALLIUM_NS_BEGIN

class InspectorThread
{
public:
    struct MessageBuffer
    {
        enum Type { kExit, kConnect, kDisconnect, kPayload };

        using Ptr = std::unique_ptr<MessageBuffer, void(*)(MessageBuffer*)>;
        static Ptr Allocate(Type type, size_t payload_size);

        Type type;
        size_t payload_size;
        uint8_t payload[];
    };
    using MessageQueue = AsyncMessageQueue<MessageBuffer, MessageBuffer::Ptr>;

    class EventHandler
    {
    public:
        virtual void OnMessage(MessageBuffer::Ptr message) = 0;
        virtual void OnDisconnect() = 0;
        virtual void OnConnect() = 0;
    };

    static std::unique_ptr<InspectorThread> Start(
            uv_loop_t *loop, int32_t port, EventHandler *handler);

    InspectorThread(uv_loop_t *loop, int32_t port, EventHandler *handler);
    ~InspectorThread();

    g_nodiscard uv_loop_t *GetEventLoop() const {
        return event_loop_;
    }

    void Dispose();

    void Send(const std::string& message);
    void WaitOnce();

    struct ThreadInitInfo;

private:
    void IoThreadRoutine(ThreadInitInfo *thread_init_info);

    void OnMainThreadRecvMessage(MessageBuffer::Ptr buffer);

    bool                                disposed_;
    uv_loop_t                          *event_loop_;
    int32_t                             port_;
    std::thread                         thread_;
    EventHandler                       *event_handler_;
    bool                                has_active_session_;
    MessageQueue                        recv_queue_;
    std::optional<MessageQueue>         send_queue_;
};

GALLIUM_NS_END
#endif //COCOA_GALLIUM_INSPECTORTHREAD_H
