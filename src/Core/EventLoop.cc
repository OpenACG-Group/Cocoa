#include "Core/Errors.h"
#include <iostream>

#include "Core/EventLoop.h"
#include "Core/EventSource.h"
namespace cocoa
{

EventLoop::EventLoop()
    : fLoop(nullptr)
{
    fLoop = new uv_loop_t;
    uv_loop_init(fLoop);
}

EventLoop::~EventLoop()
{
    this->dispose();
}

int EventLoop::run()
{
    if (!fLoop)
    {
        return -1;
    }
    return uv_run(fLoop, UV_RUN_DEFAULT);
}

void EventLoop::dispose()
{
    // FIXME: uv_loop_close returns EBUSY
    if (fLoop)
    {
        uv_loop_close(fLoop);
        delete fLoop;
        fLoop = nullptr;
    }
}

void EventLoop::walk(std::function<void(uv_handle_t *)> function)
{
    uv_walk(fLoop, [](uv_handle_t *handle, void *closure) -> void {
        auto pFunc = reinterpret_cast<std::function<void(uv_handle_t*)>*>(closure);
        (*pFunc)(handle);
    }, &function);
}

namespace {

struct TaskClosure
{
    uv_work_t   task_work;
    EventLoop::TaskRoutineVoid task_routine;
    EventLoop::PostTaskRoutineVoid post_task_routine;
};

// This function will be executed in the thread pool asynchronously
void thread_pool_task_routine(uv_work_t *work)
{
    auto *closure = reinterpret_cast<TaskClosure*>(work->data);
    CHECK(closure);
    closure->task_routine();
}

// This function will be executed in main thread locally
// to notify us that a task have been finished.
void thread_pool_post_task_routine(uv_work_t *work, int status)
{
    auto *closure = reinterpret_cast<TaskClosure*>(work->data);
    CHECK(closure);
    closure->post_task_routine();
    delete closure;
}

} // namespace anonymous

void EventLoop::enqueueThreadPoolTrivialTask(const TaskRoutineVoid& task, const PostTaskRoutineVoid& post_task)
{
    auto *closure = new TaskClosure{
        .task_routine = task,
        .post_task_routine = post_task
    };
    closure->task_work.data = closure;

    uv_queue_work(fLoop, &closure->task_work, thread_pool_task_routine,
                  thread_pool_post_task_routine);
}

PollSource::PollSource(EventLoop *loop, int fd)
    : EventSource(loop)
{
    uv_poll_init(EventSource::eventLoop()->handle(), get(), fd);
    setThis(this);
}

PollSource::~PollSource()
{
    stopPoll();
}

void PollSource::startPoll(int events)
{
    uv_poll_start(get(), events, PollSource::Callback);
}

void PollSource::stopPoll()
{
    uv_poll_stop(get());
}

void PollSource::Callback(uv_poll_t *handle, int status, int events)
{
    auto *pThis = reinterpret_cast<PollSource*>(uv_handle_get_data((uv_handle_t*)handle));
    KeepInLoop keep = pThis->pollDispatch(status, events);
    if (keep == KeepInLoop::kNo)
        pThis->stopPoll();
}

TimerSource::TimerSource(EventLoop *loop)
    : EventSource(loop)
{
    uv_timer_init(loop->handle(), get());
    setThis(this);
}

TimerSource::~TimerSource()
{
    stopTimer();
}

void TimerSource::startTimer(uint64_t timeout, uint64_t repeat)
{
    uv_timer_start(get(), TimerSource::Callback, timeout, repeat);
}

void TimerSource::stopTimer()
{
    uv_timer_stop(get());
}

void TimerSource::Callback(uv_timer_t *handle)
{
    auto *pThis = reinterpret_cast<TimerSource*>(uv_handle_get_data((uv_handle_t*)handle));
    if (pThis->timerDispatch() == KeepInLoop::kNo)
        pThis->stopTimer();
}

AsyncSource::AsyncSource(EventLoop *loop)
    : EventSource(loop), fDisabled(false)
{
    uv_async_init(loop->handle(), get(), AsyncSource::Callback);
    setThis(this);
}

void AsyncSource::disableAsync()
{
    fDisabled = true;
    uv_unref(reinterpret_cast<uv_handle_t*>(get()));
}

void AsyncSource::wakeupAsync()
{
    if (!fDisabled)
        uv_async_send(get());
}

void AsyncSource::Callback(uv_async_t *handle)
{
    auto *pThis = reinterpret_cast<AsyncSource*>(uv_handle_get_data((uv_handle_t*)handle));
    pThis->asyncDispatch();
}

PrepareSource::PrepareSource(EventLoop *loop)
    : EventSource(loop)
{
    uv_prepare_init(loop->handle(), get());
    setThis(this);
}

PrepareSource::~PrepareSource()
{
    uv_prepare_stop(get());
}

void PrepareSource::startPrepare()
{
    uv_prepare_start(get(), PrepareSource::Callback);
}

void PrepareSource::stopPrepare()
{
    uv_prepare_stop(get());
}

void PrepareSource::Callback(uv_prepare_t *handle)
{
    auto *pThis = reinterpret_cast<PrepareSource*>(uv_handle_get_data((uv_handle_t*)handle));
    if (pThis->prepareDispatch() == KeepInLoop::kNo)
        pThis->stopPrepare();
}

CheckSource::CheckSource(EventLoop *loop)
    : EventSource(loop)
{
    uv_check_init(loop->handle(), get());
    setThis(this);
}

CheckSource::~CheckSource()
{
    uv_check_stop(get());
}

void CheckSource::startCheck()
{
    uv_check_start(get(), CheckSource::Callback);
}

void CheckSource::stopCheck()
{
    uv_check_stop(get());
}

void CheckSource::Callback(uv_check_t *handle)
{
    auto *pThis = reinterpret_cast<CheckSource*>(uv_handle_get_data((uv_handle_t*)handle));
    if (pThis->checkDispatch() == KeepInLoop::kNo)
        pThis->stopCheck();
}

} // namespace cocoa
