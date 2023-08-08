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

#ifndef COCOA_GALLIUM_PARENTTHREADDELEGATE_H
#define COCOA_GALLIUM_PARENTTHREADDELEGATE_H

#include <queue>
#include <mutex>

#include "uv.h"

#include "Core/EventLoop.h"
#include "Gallium/Gallium.h"
#include "Gallium/WorkerMessage.h"
GALLIUM_NS_BEGIN

class WorkerRuntimeThread;
class RuntimeBase;

class ParentThreadDelegate
{
public:
    explicit ParentThreadDelegate(RuntimeBase *runtime);
    ~ParentThreadDelegate();

    g_nodiscard uv_loop_t *GetEventLoop() const;

    void PostMessageToMainThread(std::unique_ptr<WorkerMessage> message);

    void NotifyNewWorkerThreadCreated();

private:
    void OnReceiveMessage();

    RuntimeBase                     *runtime_;
    std::optional<uv::AsyncHandle>   message_async_;
    std::queue<std::unique_ptr<WorkerMessage>>
                                     message_queue_;
    std::mutex                       message_queue_lock_;
};

GALLIUM_NS_END
#endif //COCOA_GALLIUM_PARENTTHREADDELEGATE_H
