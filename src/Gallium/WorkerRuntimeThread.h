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

#ifndef COCOA_GALLIUM_WORKERRUNTIMETHREAD_H
#define COCOA_GALLIUM_WORKERRUNTIMETHREAD_H

#include <memory>
#include <mutex>
#include <queue>

#include "uv.h"

#include "Gallium/Gallium.h"
#include "Gallium/WorkerMessage.h"
GALLIUM_NS_BEGIN

class ParentThreadDelegate;
class Platform;

class WorkerRuntimeThread
{
public:
    struct Options
    {
    };

    struct CreateResult
    {
        std::unique_ptr<WorkerRuntimeThread> worker;
        std::optional<std::string> error;
    };

    static CreateResult Create(ParentThreadDelegate *parent_thread_delegate,
                               const std::string& url,
                               std::shared_ptr<Platform> platform,
                               const Options& options);

    explicit WorkerRuntimeThread(ParentThreadDelegate *delegate);
    ~WorkerRuntimeThread();

    void PostMessageToWorker(std::unique_ptr<WorkerMessage> message);

    void SetMessageReceiveCallback(std::function<void(v8::Local<v8::Value>)> func) {
        receive_cb_ = std::move(func);
    }

    g_nodiscard g_inline ParentThreadDelegate *GetParentDelegate() {
        return delegate_;
    }

private:
    static void WorkerEntrypoint(void *args);

    ParentThreadDelegate       *delegate_;
    uv_thread_t                 thread_;
    uv_loop_t                   thread_loop_;
    std::optional<uv::AsyncHandle>
                                message_async_;
    std::queue<std::unique_ptr<WorkerMessage>>
                                message_queue_;
    std::mutex                  message_queue_lock_;
    std::function<void(v8::Local<v8::Value>)> receive_cb_;
};

GALLIUM_NS_END
#endif //COCOA_GALLIUM_WORKERRUNTIMETHREAD_H
