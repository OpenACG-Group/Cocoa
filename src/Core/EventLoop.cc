#include <cassert>

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
    // FIXME: uv_loop_close returns EBUSY
    uv_loop_close(fLoop);
    delete fLoop;
}

int EventLoop::run()
{
    return uv_run(fLoop, UV_RUN_DEFAULT);
}

EventSource::EventSource(EventLoop *loop)
    : fLoop(loop)
{
}

PollSource::PollSource(EventLoop *loop, int fd)
    : EventSource(loop),
      fUvPoll{},
      fStopped(true)
{
    uv_poll_init(EventSource::eventLoop()->handle(), &fUvPoll, fd);
    uv_handle_set_data((uv_handle_t*)&fUvPoll, this);
}

PollSource::~PollSource()
{
    stopPoll();
}

void PollSource::startPoll(int events)
{
    uv_poll_start(&fUvPoll, events, PollSource::Callback);
    fStopped = false;
}

void PollSource::stopPoll()
{
    if (fStopped)
        return;
    uv_poll_stop(&fUvPoll);
    fStopped = true;
}

void PollSource::Callback(uv_poll_t *handle, int status, int events)
{
    auto *pThis = reinterpret_cast<PollSource*>(uv_handle_get_data((uv_handle_t*)handle));
    KeepInLoop keep = pThis->pollDispatch(status, events);
    if (keep == KeepInLoop::kNo)
        pThis->stopPoll();
}

TimerSource::TimerSource(EventLoop *loop)
    : EventSource(loop),
      fUvTimer{}
{
    uv_timer_init(loop->handle(), &fUvTimer);
    uv_handle_set_data((uv_handle_t*)&fUvTimer, this);
}

TimerSource::~TimerSource()
{
    stopTimer();
}

void TimerSource::startTimer(uint64_t timeout, uint64_t repeat)
{
    uv_timer_start(&fUvTimer, TimerSource::Callback, timeout, repeat);
}

void TimerSource::stopTimer()
{
    uv_timer_stop(&fUvTimer);
}

void TimerSource::Callback(uv_timer_t *handle)
{
    auto *pThis = reinterpret_cast<TimerSource*>(uv_handle_get_data((uv_handle_t*)handle));
    if (pThis->timerDispatch() == KeepInLoop::kNo)
        pThis->stopTimer();
}

AsyncSource::AsyncSource(EventLoop *loop)
    : EventSource(loop),
      fUvAsync{},
      fDisabled(false)
{
    uv_async_init(loop->handle(), &fUvAsync, AsyncSource::Callback);
    uv_handle_set_data((uv_handle_t*)&fUvAsync, this);
}

AsyncSource::~AsyncSource()
{
    disableAsync();
}

void AsyncSource::disableAsync()
{
    if (!fDisabled)
    {
        uv_unref((uv_handle_t *) &fUvAsync);
        fDisabled = true;
    }
}

void AsyncSource::wakeupAsync()
{
    uv_async_send(&fUvAsync);
}

void AsyncSource::Callback(uv_async_t *handle)
{
    auto *pThis = reinterpret_cast<AsyncSource*>(uv_handle_get_data((uv_handle_t*)handle));
    pThis->asyncDispatch();
}

CheckHandleSource::CheckHandleSource(EventLoop *loop)
    : EventSource(loop),
      fCheck{}
{
    uv_check_init(loop->handle(), &fCheck);
    uv_handle_set_data((uv_handle_t*)&fCheck, this);
}

CheckHandleSource::~CheckHandleSource()
{
    uv_check_stop(&fCheck);
    uv_unref((uv_handle_t*)&fCheck);
}

void CheckHandleSource::startCheckHandle()
{
    uv_check_start(&fCheck, CheckHandleSource::Callback);
}

void CheckHandleSource::stopCheckHandle()
{
    uv_check_stop(&fCheck);
}

void CheckHandleSource::Callback(uv_check_t *handle)
{
    auto *pThis = reinterpret_cast<CheckHandleSource*>(uv_handle_get_data((uv_handle_t*)handle));
    if (pThis->checkHandleDispatch() == KeepInLoop::kNo)
        pThis->stopCheckHandle();
}

} // namespace cocoa
