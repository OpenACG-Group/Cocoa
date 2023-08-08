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

#include <cstdlib>
#include <thread>
#include <unordered_set>

#include "include/libplatform/libplatform.h"

#include "Core/Utils.h"
#include "Core/Journal.h"
#include "Core/Errors.h"
#include "Core/ConcurrentTaskQueue.h"
#include "Core/TraceEvent.h"
#include "Gallium/Platform.h"
GALLIUM_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.Platform)

#define DEFAULT_THREAD_POOL_SIZE     4

class Platform::WorkerThreadsPool
{
public:
    explicit WorkerThreadsPool(int32_t pool_size)
        : workers_ready_barrier_{}
    {
        uv_barrier_init(&workers_ready_barrier_, pool_size + 1);
        for (int32_t i = 0; i < pool_size; i++)
        {
            // Worker thread is started since here
            worker_threads_.emplace_back(&WorkerThreadsPool::WorkerEntrypoint, this, i);
        }
        uv_barrier_wait(&workers_ready_barrier_);
        uv_barrier_destroy(&workers_ready_barrier_);
    }

    ~WorkerThreadsPool()
    {
        task_queue_.Dispose();
        for (auto& thread : worker_threads_)
        {
            if (thread.joinable())
                thread.join();
        }
    }

    void EnqueueTask(std::unique_ptr<v8::Task> task)
    {
        task_queue_.Push(std::move(task));
    }

    void WaitDrainTasks()
    {
        task_queue_.WaitDrain();
    }

private:
    void WorkerEntrypoint(int32_t worker_index)
    {
        utils::SetThreadName(fmt::format("V8Worker#{}", worker_index).c_str());
        uv_barrier_wait(&workers_ready_barrier_);

        while (std::unique_ptr<v8::Task> task = task_queue_.WaitPop())
        {
            QLOG(LOG_DEBUG, "worker#{}: performing asynchronous task on the worker thread", worker_index);
            task->Run();
            task_queue_.NotifyOfCompletion();
        }
    }

    std::vector<std::thread>        worker_threads_;
    uv_barrier_t                    workers_ready_barrier_;
    ConcurrentTaskQueue<v8::Task>   task_queue_;
};

class Platform::DelayedTaskScheduler
{
public:
    // This scheduler shares worker thread pool with the `Platform`
    // object. A `Platform` object has only one scheduler associated
    // with it.
    explicit DelayedTaskScheduler(WorkerThreadsPool *worker_thread_pool)
        : disposed_(false)
        , ready_barrier_{}
        , scheduler_loop_{}
        , task_notify_{}
        , worker_thread_pool_(worker_thread_pool)
    {
        CHECK(worker_thread_pool_);

        // Main thread and scheduler thread are waiting for the barrier
        uv_barrier_init(&ready_barrier_, 2);
        scheduler_thread_ = std::thread(&DelayedTaskScheduler::Run, this);
        uv_barrier_wait(&ready_barrier_);
        uv_barrier_destroy(&ready_barrier_);
    }

    ~DelayedTaskScheduler()
    {
        CHECK(disposed_ && "DelayedTaskScheduler should be disposed before destructing");
    }

    void EnqueueDelayedTask(std::unique_ptr<v8::Task> task, double delay_seconds)
    {
        CHECK(!disposed_);
        queue_.Push(std::make_unique<ScheduleTask>(this, std::move(task), delay_seconds));
        uv_async_send(&task_notify_);
    }

    void Dispose()
    {
        CHECK(!disposed_ && "Dispose for multiple times");
        queue_.Push(std::make_unique<DisposeTask>(this));
        uv_async_send(&task_notify_);
        disposed_ = true;
        if (scheduler_thread_.joinable())
            scheduler_thread_.join();
    }

private:
    void Run()
    {
        utils::SetThreadName("V8TaskScheduler");

        // Initialize thread event loop and notifier
        uv_loop_init(&scheduler_loop_);
        uv_async_init(&scheduler_loop_, &task_notify_, TaskNotified);
        scheduler_loop_.data = this;
        task_notify_.data = this;

        // Notify main thread that scheduler is ready to receive tasks
        uv_barrier_wait(&ready_barrier_);

        uv_run(&scheduler_loop_, UV_RUN_DEFAULT);
        if (uv_loop_close(&scheduler_loop_) == 0)
            return;

        // `scheduler_loop_` cannot be closed correctly because there are still
        // some active handles
        uv_print_all_handles(&scheduler_loop_, stderr);
        CHECK_FAILED("Could not close event loop: above handles are still active");
    }

    static void TaskNotified(uv_async_t *handle)
    {
        CHECK(handle && handle->data);
        auto *sched = reinterpret_cast<DelayedTaskScheduler*>(handle->data);
        while (std::unique_ptr<v8::Task> task = sched->queue_.Pop())
        {
            TRACE_EVENT("main", "SchedulerTask");
            task->Run();
        }
    }

    static void OnTimerExpired(uv_timer_t *timer)
    {
        CHECK(timer && timer->data);
        CHECK(timer->loop && timer->loop->data);

        auto *sched = reinterpret_cast<DelayedTaskScheduler*>(timer->loop->data);
        auto task = reinterpret_cast<v8::Task*>(timer->data);

        // Task queue takes the ownership of `task`, and `timer->data`
        // should not refer to it anymore
        sched->worker_thread_pool_->EnqueueTask(std::unique_ptr<v8::Task>(task));
        timer->data = nullptr;

        // Expired timer should be removed from timers set and be closed
        // properly. If the scheduler disposed before all the timers expiring,
        // those pending timers will be closed by `DisposeTask::Run`.
        sched->timers_set_.erase(timer);
        uv_close(reinterpret_cast<uv_handle_t*>(timer), [](uv_handle_t *handle) {
            delete reinterpret_cast<uv_timer_t*>(handle);
        });
    }

    class ScheduleTask : public v8::Task
    {
    public:
        ScheduleTask(DelayedTaskScheduler *sched,
                     std::unique_ptr<v8::Task> task,
                     double delay_seconds)
            : sched_(sched), task_(std::move(task)), delay_seconds_(delay_seconds) {}
        ~ScheduleTask() override = default;

        void Run() override
        {
            CHECK(sched_);

            uint64_t delay_millis = std::llround(delay_seconds_ * 1000);

            std::unique_ptr<uv_timer_t> timer(new uv_timer_t);
            // Take over the ownership of `Task` object; we will release it manually later.
            timer->data = task_.release();
            uv_timer_init(&sched_->scheduler_loop_, timer.get());
            uv_timer_start(timer.get(), OnTimerExpired, delay_millis, 0);

            // Take over the ownership of timer
            sched_->timers_set_.insert(timer.release());
        }

    private:
        DelayedTaskScheduler            *sched_;
        std::unique_ptr<v8::Task>        task_;
        double                           delay_seconds_;
    };

    class DisposeTask : public v8::Task
    {
    public:
        explicit DisposeTask(DelayedTaskScheduler *sched) : sched_(sched) {}
        ~DisposeTask() override = default;

        void Run() override
        {
            CHECK(sched_);
            for (uv_timer_t *timer : sched_->timers_set_)
            {
                uv_timer_stop(timer);
                if (timer->data)
                {
                    // It is our responsibility to free the task associated with
                    // the timer.
                    delete reinterpret_cast<v8::Task*>(timer->data);
                }
                // A handle must not be released until we close it.
                uv_close(reinterpret_cast<uv_handle_t*>(timer), [](uv_handle_t *handle) {
                    delete reinterpret_cast<uv_timer_t*>(handle);
                });
            }
            sched_->timers_set_.clear();

            uv_close(reinterpret_cast<uv_handle_t*>(&sched_->task_notify_),
                     [](uv_handle_t *) {});
        }

    private:
        DelayedTaskScheduler *sched_;
    };

    bool                                disposed_;
    uv_barrier_t                        ready_barrier_;
    uv_loop_t                           scheduler_loop_;
    uv_async_t                          task_notify_;
    std::thread                         scheduler_thread_;
    WorkerThreadsPool                  *worker_thread_pool_;
    ConcurrentTaskQueue<v8::Task>       queue_;
    std::unordered_set<uv_timer_t*>     timers_set_;
};

// v8::TaskRunner implementation

PerIsolateData::PerIsolateData(v8::Isolate *isolate, EventLoop *main_loop)
    : disposed_(false)
    , isolate_(isolate)
    , main_loop_(main_loop)
    , tasks_notifier_{}
{
    CHECK(isolate_ && main_loop_);
    uv_async_init(main_loop->handle(), &tasks_notifier_, OnTaskNotified);
    tasks_notifier_.data = this;
    uv_unref(reinterpret_cast<uv_handle_t*>(&tasks_notifier_));
}

PerIsolateData::~PerIsolateData()
{
    CHECK(disposed_ && "PerIsolateData must be disposed before destructing");
}

void PerIsolateData::Dispose()
{
    if (disposed_)
        return;

    foreground_tasks_queue_.PopAll();
    scheduled_delayed_tasks_.clear();

    // `uv_close` performs the cleanup function in the next loop tick.
    // Consequently, we must make sure the object itself will not be destructed
    // before the cleanup function is called.
    self_ptr_ = shared_from_this();

    uv_close(reinterpret_cast<uv_handle_t*>(&tasks_notifier_),
             [](uv_handle_t *handle) {
        CHECK(handle && handle->data);
        auto per_isolate = reinterpret_cast<PerIsolateData*>(handle->data);
        // Now `PerIsolateData` can be destructed.
        // Maybe it is still referenced by other scopes and will not be destructed immediately,
        // but this should not be a problem in practice.
        per_isolate->self_ptr_.reset();
    });

    disposed_ = true;
}

void PerIsolateData::OnTaskNotified(uv_async_t *handle)
{
    CHECK(handle && handle->data);
    auto per_isolate = reinterpret_cast<PerIsolateData*>(handle->data);
    per_isolate->PerformForegroundTasks();
}

void PerIsolateData::RemoveScheduledDelayedTask(WrappedTask *ptr)
{
    auto itr = std::find_if(scheduled_delayed_tasks_.begin(),
                            scheduled_delayed_tasks_.end(),
                            [ptr](const WrappedTask::Pointer& task) {
        return (ptr == task.get());
    });

    if (itr != scheduled_delayed_tasks_.end())
        scheduled_delayed_tasks_.erase(itr);
}

bool PerIsolateData::PerformForegroundTasks()
{
    bool did_work = false;

    std::queue<std::unique_ptr<WrappedTask>> tasks = foreground_tasks_queue_.PopAll();
    while (!tasks.empty())
    {
        std::unique_ptr<WrappedTask> wrapped = std::move(tasks.front());
        tasks.pop();

        did_work = true;
        if (wrapped->is_delayed)
        {
            uv_timer_init(main_loop_->handle(), &wrapped->delay_timer);
            wrapped->delay_timer.data = wrapped.get();

            uv_timer_start(&wrapped->delay_timer, [](uv_timer_t *timer) {
                CHECK(timer && timer->data);
                auto *wrapped = reinterpret_cast<WrappedTask*>(timer->data);
                QLOG(LOG_DEBUG, "TaskRunner: performing delayed foreground task on the main thread");
                wrapped->task->Run();
                wrapped->per_isolate->RemoveScheduledDelayedTask(wrapped);
            }, wrapped->delay_milliseconds, 0);

            uv_unref(reinterpret_cast<uv_handle_t*>(&wrapped->delay_timer));

            scheduled_delayed_tasks_.emplace_back(wrapped.release(),
                                                  [](WrappedTask *task) {
                uv_close(reinterpret_cast<uv_handle_t*>(&task->delay_timer),
                         [](uv_handle_t *handle) { delete handle; });
            });
        }
        else
        {
            QLOG(LOG_DEBUG, "TaskRunner: performing foreground task on the main thread");
            wrapped->task->Run();
        }
    }
    return did_work;
}

void PerIsolateData::PostTask(std::unique_ptr<v8::Task> task)
{
    // V8 may post tasks after the Isolate has been disposed,
    // and we can simply ignore it.
    if (disposed_)
        return;

    auto wrapped = std::make_unique<WrappedTask>();
    wrapped->task = std::move(task);
    wrapped->is_delayed = false;
    wrapped->per_isolate = this;
    foreground_tasks_queue_.Push(std::move(wrapped));
    uv_async_send(&tasks_notifier_);
}

void PerIsolateData::PostDelayedTask(std::unique_ptr<v8::Task> task,
                                     double delay_in_seconds)
{
    // V8 may post tasks after the Isolate has been disposed,
    // and we can simply ignore it.
    if (disposed_)
        return;

    auto wrapped = std::make_unique<WrappedTask>();
    wrapped->task = std::move(task);
    wrapped->is_delayed = true;
    wrapped->delay_milliseconds = std::llround(delay_in_seconds * 1000);
    wrapped->per_isolate = this;
}

void PerIsolateData::PostNonNestableTask(std::unique_ptr<v8::Task> task)
{
    return PostTask(std::move(task));
}

void PerIsolateData::PostNonNestableDelayedTask(std::unique_ptr<v8::Task> task,
                                                double delay_in_seconds)
{
    return PostDelayedTask(std::move(task), delay_in_seconds);
}

void PerIsolateData::PostIdleTask(std::unique_ptr<v8::IdleTask> task)
{
    MARK_UNREACHABLE();
}

// v8::Platform implementation

std::unique_ptr<Platform> Platform::Make(EventLoop *main_loop,
                                         int32_t workers,
                                         std::unique_ptr<TracingController> tracing_controller)
{
    CHECK(main_loop);
    return std::make_unique<Platform>(main_loop, workers, std::move(tracing_controller));
}

Platform::Platform(EventLoop *loop, int32_t workers,
                   std::unique_ptr<TracingController> tc)
    : main_loop_(loop)
    , tracing_controller_(std::move(tc))
    , worker_threads_pool_(std::make_unique<WorkerThreadsPool>(workers))
    , delayed_task_scheduler_(std::make_unique<DelayedTaskScheduler>(worker_threads_pool_.get()))
{
    CHECK(main_loop_);
}

Platform::~Platform()
{
    delayed_task_scheduler_->Dispose();
}

std::shared_ptr<PerIsolateData>& Platform::GetPerIsolateData(v8::Isolate *isolate)
{
    CHECK(per_isolate_datas_.count(isolate) > 0);
    return per_isolate_datas_[isolate];
}

void Platform::RegisterIsolate(v8::Isolate *isolate)
{
    if (per_isolate_datas_.count(isolate) > 0)
        return;
    per_isolate_datas_[isolate] = std::make_shared<PerIsolateData>(isolate, main_loop_);
}

void Platform::UnregisterIsolate(v8::Isolate *isolate)
{
    if (per_isolate_datas_.count(isolate) == 0)
        return;

    per_isolate_datas_[isolate]->Dispose();
    per_isolate_datas_.erase(isolate);
}

void Platform::DrainTasks(v8::Isolate *isolate)
{
    std::shared_ptr<PerIsolateData> per_isolate = GetPerIsolateData(isolate);
    if (!per_isolate)
        return;
    do
    {
        worker_threads_pool_->WaitDrainTasks();
    } while (per_isolate->PerformForegroundTasks());
}

int Platform::NumberOfWorkerThreads()
{
    return DEFAULT_THREAD_POOL_SIZE;
}

std::shared_ptr<v8::TaskRunner> Platform::GetForegroundTaskRunner(v8::Isolate *isolate)
{
    return GetPerIsolateData(isolate);
}

void Platform::PostTaskOnWorkerThreadImpl(v8::TaskPriority priority,
                                          std::unique_ptr<v8::Task> task,
                                          const v8::SourceLocation &location)
{
    CHECK(task);
    worker_threads_pool_->EnqueueTask(std::move(task));
}

void Platform::PostDelayedTaskOnWorkerThreadImpl(v8::TaskPriority priority,
                                                 std::unique_ptr<v8::Task> task,
                                                 double delay_in_seconds,
                                                 const v8::SourceLocation &location)
{
    CHECK(task);
    delayed_task_scheduler_->EnqueueDelayedTask(std::move(task),
                                                delay_in_seconds);
}

bool Platform::IdleTasksEnabled(v8::Isolate *isolate)
{
    return GetPerIsolateData(isolate)->IdleTasksEnabled();
}

double Platform::MonotonicallyIncreasingTime()
{
    return (static_cast<double>(uv_hrtime()) / 1e9);
}

double Platform::CurrentClockTimeMillis()
{
    return v8::Platform::SystemClockTimeMillis();
}

v8::TracingController *Platform::GetTracingController()
{
    CHECK(tracing_controller_ != nullptr);
    return tracing_controller_.get();
}

v8::Platform::StackTracePrinter Platform::GetStackTracePrinter()
{
    return []() {
        utils::PrintStackBacktrace("V8 requires stack backtrace");
    };
}

std::unique_ptr<v8::JobHandle> Platform::CreateJobImpl(v8::TaskPriority priority,
                                                       std::unique_ptr<v8::JobTask> job_task,
                                                       const v8::SourceLocation& location)
{
    return v8::platform::NewDefaultJobHandle(this, priority,
                                             std::move(job_task),
                                             NumberOfWorkerThreads());
}

v8::PageAllocator *Platform::GetPageAllocator()
{
    return nullptr;
}

GALLIUM_NS_END
