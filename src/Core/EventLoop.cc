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

LoopPrologueSource::LoopPrologueSource(EventLoop *loop)
    : EventSource(loop)
{
    uv_prepare_init(loop->handle(), get());
    setThis(this);
}

LoopPrologueSource::~LoopPrologueSource()
{
    uv_prepare_stop(get());
}

void LoopPrologueSource::startLoopPrologue()
{
    uv_prepare_start(get(), LoopPrologueSource::Callback);
}

void LoopPrologueSource::stopLoopPrologue()
{
    uv_prepare_stop(get());
}

void LoopPrologueSource::Callback(uv_prepare_t *handle)
{
    auto *pThis = reinterpret_cast<LoopPrologueSource*>(uv_handle_get_data((uv_handle_t*)handle));
    if (pThis->loopPrologueDispatch() == KeepInLoop::kNo)
        pThis->stopLoopPrologue();
}

} // namespace cocoa
