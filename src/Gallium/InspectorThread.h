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

#include "Core/ConcurrentTaskQueue.h"
#include "Gallium/Gallium.h"
GALLIUM_NS_BEGIN

class InspectorThread
{
public:
    using MessageNotifier = std::function<void(const std::string&)>;
    using DisconnectNotifier = std::function<void()>;
    using ConnectNotifier = std::function<void()>;

    InspectorThread(int32_t port,
                    MessageNotifier message_notifier,
                    DisconnectNotifier disconnect_notifier,
                    ConnectNotifier connect_notifier);
    ~InspectorThread();

    void SendFrontendMessage(const std::string& message);

    g_private_api void NotifyConnected();
    g_private_api void NotifyDisconnected();
    g_private_api void NotifyMessage(const std::string& message);

private:
    void IoThreadRoutine();

    int32_t                 port_;
    std::thread             thread_;
    MessageNotifier         message_notifier_;
    DisconnectNotifier      disconnect_notifier_;
    ConnectNotifier         connect_notifier_;
    ConcurrentTaskQueue<std::string> send_queue_;
    std::atomic<bool>       disconnected_;
    uv_async_t              message_queue_async_;
};

GALLIUM_NS_END
#endif //COCOA_GALLIUM_INSPECTORTHREAD_H
