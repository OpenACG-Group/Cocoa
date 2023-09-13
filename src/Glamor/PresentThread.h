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

#ifndef COCOA_GLAMOR_PRESENTTHREAD_H
#define COCOA_GLAMOR_PRESENTTHREAD_H

#include <queue>
#include <functional>
#include <list>

#include "Core/EventLoop.h"
#include "Core/AsyncMessageQueue.h"
#include "Core/UniquePersistent.h"
#include "Glamor/Glamor.h"
#include "Glamor/PresentMessage.h"
#include "Glamor/PresentRemoteHandle.h"
#include "Glamor/PresentThreadTaskRunner.h"
GLAMOR_NAMESPACE_BEGIN

class Display;
class RemoteDestroyablesCollector;

class PresentThread
{
public:
    using Queue = AsyncMessageQueue<PresentMessage>;

    /**
     * A thread-local context in present thread.
     * It helps `PresentRemoteHandle` to emit signals.
     * Get its instance for current thread using `GetCurrent()`
     * static method.
     */
    class LocalContext : public ThreadLocalUniquePersistent<LocalContext>
    {
    public:
        explicit LocalContext(uv_loop_t *event_loop,
                              std::shared_ptr<Queue> main_thread_queue,
                              std::shared_ptr<RemoteDestroyablesCollector> collector);
        ~LocalContext() = default;

        void EnqueueSignal(const std::shared_ptr<PresentRemoteHandle>& emitter,
                           PresentRemoteHandle::SignalCode signal_code,
                           PresentSignal signal_info,
                           bool has_local_listeners);

        g_nodiscard uv_loop_t *GetEventLoop() const {
            return event_loop_;
        }

        void AddActiveDisplay(std::shared_ptr<Display> display);
        void RemoveActiveDisplay(const std::shared_ptr<Display>& display);

        std::string TraceResourcesJSON();

    private:
        uv_loop_t                          *event_loop_;
        std::shared_ptr<Queue>              main_thread_queue_;
        uv::IdleHandle                      idle_handle_;
        std::queue<PresentSignalMessage>    local_signal_queue_;
        std::list<std::shared_ptr<Display>> active_displays_;
        std::shared_ptr<RemoteDestroyablesCollector>
                                            remote_destroyables_collector_;
    };

    static std::unique_ptr<PresentThread> Start(uv_loop_t *loop);

    PresentThread(std::weak_ptr<Queue> present_thread_queue,
                  std::shared_ptr<Queue> main_thread_queue,
                  pthread_t present_thread,
                  std::shared_ptr<RemoteDestroyablesCollector> collector);

    g_nodiscard RemoteDestroyablesCollector *GetRemoteDestroyablesCollector() const {
        return remote_destroyables_collector_.get();
    }

    void Dispose();

    void EnqueueRemoteCall(std::shared_ptr<PresentRemoteHandle> receiver,
                           PresentRemoteCall call_info,
                           PresentRemoteCallResultCallback result_callback);

    template<typename Ret>
    void SubmitTask(std::function<Ret(void)> task_func,
                    std::function<void(Ret)> result_callback,
                    std::function<void(std::string)> caught_callback);

    void SubmitTaskNoRet(std::function<void()> task_func,
                         std::function<void()> result_callback,
                         std::function<void(std::string)> caught_callback);

    // TODO(sora): implement message queue profiling API

private:
    void OnMainThreadMessage(Queue::Message message);

    std::weak_ptr<Queue>        present_thread_queue_;
    std::shared_ptr<Queue>      main_thread_queue_;
    pthread_t                   present_thread_;
    bool                        thread_has_exited_;
    std::shared_ptr<PresentThreadTaskRunner>
                                task_runner_;
    std::shared_ptr<RemoteDestroyablesCollector>
                                remote_destroyables_collector_;
};

template<typename Ret>
void PresentThread::SubmitTask(std::function<Ret(void)> task_func,
                               std::function<void(Ret)> result_callback,
                               std::function<void(std::string)> caught_callback)
{
    PresentThreadTaskRunner::Task task = [func = std::move(task_func)]() -> std::any {
        return func();
    };
    task_runner_->Invoke(GLOP_TASKRUNNER_RUN, nullptr,
        [func = std::move(result_callback), caught = std::move(caught_callback)]
        (PresentRemoteCallReturn &ret) {
            if (ret.GetReturnStatus() == PresentRemoteCall::Status::kCaught)
                caught(ret.GetCaughtException());
            else if (func)
                func(ret.GetReturnValue<Ret>());
        },
        task
    );
}

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_PRESENTTHREAD_H
