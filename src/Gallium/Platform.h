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

#ifndef COCOA_GALLIUM_PLATFORM_H
#define COCOA_GALLIUM_PLATFORM_H

#include <unordered_map>
#include <queue>
#include <mutex>

#include "include/v8-platform.h"
#include "uv.h"

#include "Core/ConcurrentTaskQueue.h"
#include "Core/EventLoop.h"
#include "Gallium/Gallium.h"
GALLIUM_NS_BEGIN

class PerIsolateData : public v8::TaskRunner
                     , public std::enable_shared_from_this<PerIsolateData>
{
public:
    struct WrappedTask
    {
        using Deleter = void(*)(WrappedTask*);
        using Pointer = std::unique_ptr<WrappedTask, WrappedTask::Deleter>;

        std::unique_ptr<v8::Task>   task;
        bool                        is_delayed;
        int64_t                     delay_milliseconds;
        uv_timer_t                  delay_timer;
        PerIsolateData             *per_isolate;
    };

    PerIsolateData(v8::Isolate *isolate, EventLoop *main_loop);
    ~PerIsolateData() override;

    void PostTask(std::unique_ptr<v8::Task> task) override;
    void PostNonNestableTask(std::unique_ptr<v8::Task> task) override;
    void PostDelayedTask(std::unique_ptr<v8::Task> task,
                         double delay_in_seconds) override;
    void PostNonNestableDelayedTask(std::unique_ptr<v8::Task> task,
                                    double delay_in_seconds) override;
    void PostIdleTask(std::unique_ptr<v8::IdleTask> task) override;

    g_nodiscard g_inline bool IdleTasksEnabled() override {
        return false;
    }

    g_nodiscard g_inline bool NonNestableTasksEnabled() const override {
        return true;
    }
    g_nodiscard g_inline bool NonNestableDelayedTasksEnabled() const override {
        return true;
    }

    void Dispose();

    g_private_api void RemoveScheduledDelayedTask(WrappedTask *ptr);
    g_private_api bool PerformForegroundTasks();

private:
    static void OnTaskNotified(uv_async_t *handle);

    bool                                disposed_;
    // Hold a reference of the object itself to avoid being
    // destructed during disposing.
    std::shared_ptr<PerIsolateData>     self_ptr_;
    v8::Isolate                        *isolate_;
    EventLoop                          *main_loop_;
    uv_async_t                          tasks_notifier_;
    ConcurrentTaskQueue<WrappedTask>    foreground_tasks_queue_;
    std::vector<WrappedTask::Pointer>   scheduled_delayed_tasks_;
};

class Platform : public v8::Platform
{
public:
    class WorkerThreadsPool;
    class DelayedTaskScheduler;

    Platform(EventLoop *loop, int32_t workers, v8::TracingController *tc);
    ~Platform() override;

    static std::unique_ptr<Platform> Make(EventLoop *main_loop,
                                          int32_t workers,
                                          v8::TracingController *tracing_controller);

    void RegisterIsolate(v8::Isolate *isolate);
    void UnregisterIsolate(v8::Isolate *isolate);

    /**
     * Drain tasks on the specified `isolate`.
     * To drain tasks means to consume all the pending tasks in the queue,
     * including delayed tasks.
     */
    void DrainTasks(v8::Isolate *isolate);

    /* v8 implementations */

    int NumberOfWorkerThreads() override;

    std::shared_ptr<v8::TaskRunner> GetForegroundTaskRunner(v8::Isolate *isolate) override;


    void PostTaskOnWorkerThreadImpl(v8::TaskPriority priority,
                                    std::unique_ptr<v8::Task> task,
                                    const v8::SourceLocation &location) override;

    void PostDelayedTaskOnWorkerThreadImpl(v8::TaskPriority priority,
                                           std::unique_ptr<v8::Task> task,
                                           double delay_in_seconds,
                                           const v8::SourceLocation &location) override;

    bool IdleTasksEnabled(v8::Isolate *isolate) override;

    double MonotonicallyIncreasingTime() override;
    double CurrentClockTimeMillis() override;
    StackTracePrinter GetStackTracePrinter() override;
    v8::TracingController *GetTracingController() override;

    std::unique_ptr<v8::JobHandle> CreateJobImpl(v8::TaskPriority priority,
                                                 std::unique_ptr<v8::JobTask> job_task,
                                                 const v8::SourceLocation& location) override;

    v8::PageAllocator * GetPageAllocator() override;

private:
    std::shared_ptr<PerIsolateData>& GetPerIsolateData(v8::Isolate *isolate);

    EventLoop                       *main_loop_;
    v8::TracingController           *tracing_controller_;
    std::unordered_map<v8::Isolate*, std::shared_ptr<PerIsolateData>>
                                     per_isolate_datas_;
    std::unique_ptr<WorkerThreadsPool> worker_threads_pool_;
    std::unique_ptr<DelayedTaskScheduler> delayed_task_scheduler_;
};

GALLIUM_NS_END
#endif //COCOA_GALLIUM_PLATFORM_H
