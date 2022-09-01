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

#include "Core/Errors.h"
#include <iostream>

#include "Core/EventLoop.h"
#include "Core/EventSource.h"
namespace cocoa
{

EventLoop::EventLoop()
    : loop_{}
{
    uv_loop_init(&loop_);
}

EventLoop::~EventLoop()
{
    this->dispose();
}

int EventLoop::run()
{
    return uv_run(&loop_, UV_RUN_DEFAULT);
}

void EventLoop::spin(const std::function<void()>& func)
{
    do
    {
        uv_run(&loop_, UV_RUN_DEFAULT);
        func();
    } while (uv_loop_alive(&loop_));
}

void EventLoop::dispose()
{
    // FIXME: uv_loop_close returns EBUSY
    uv_loop_close(&loop_);
}

void EventLoop::walk(std::function<void(uv_handle_t *)> function)
{
    uv_walk(&loop_, [](uv_handle_t *handle, void *closure) -> void {
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

    uv_queue_work(&loop_, &closure->task_work, thread_pool_task_routine,
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
