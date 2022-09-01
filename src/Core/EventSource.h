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

#ifndef COCOA_CORE_EVENTSOURCE_H
#define COCOA_CORE_EVENTSOURCE_H

#include <functional>
#include <atomic>

#include <uv.h>

namespace cocoa
{

class EventLoop;

enum class KeepInLoop
{
    kYes,
    kNo,
    kDeleted
};

template<typename T, typename R>
class EventSource
{
public:
    explicit EventSource(EventLoop *loop)
        : fLoop(loop), fHandle(nullptr)
    {
        fHandle = reinterpret_cast<uv_handle_t*>(std::malloc(sizeof(R)));
    }
    virtual ~EventSource()
    {
        uv_close(fHandle, [](uv_handle_t *ptr) {
            std::free(ptr);
        });
    }

    void unrefEventSource()
    {
        uv_unref(fHandle);
    }

    void refEventSource()
    {
        uv_ref(fHandle);
    }

    inline EventLoop *eventLoop() {
        return fLoop;
    }

protected:
    R *get() {
        return reinterpret_cast<R*>(fHandle);
    }

    void setThis(T *ptr) {
        uv_handle_set_data(fHandle, ptr);
    }

private:
    EventLoop       *fLoop;
    uv_handle_t     *fHandle;
};

class TimerSource : public EventSource<TimerSource, uv_timer_t>
{
public:
    explicit TimerSource(EventLoop *loop);
    ~TimerSource() override;

    void startTimer(uint64_t timeout, uint64_t repeat = 0);
    void stopTimer();

protected:
    virtual KeepInLoop timerDispatch() = 0;

private:
    static void Callback(uv_timer_t *handle);
};

class AsyncSource : public EventSource<AsyncSource, uv_async_t>
{
public:
    explicit AsyncSource(EventLoop *loop);
    ~AsyncSource() override = default;

protected:
    void disableAsync();
    void wakeupAsync();
    virtual void asyncDispatch() = 0;

private:
    static void Callback(uv_async_t *handle);
    std::atomic<bool> fDisabled;
};

class PrepareSource : public EventSource<PrepareSource, uv_prepare_t>
{
public:
    explicit PrepareSource(EventLoop *loop);
    ~PrepareSource() override;

protected:
    void startPrepare();
    void stopPrepare();
    virtual KeepInLoop prepareDispatch() = 0;

private:
    static void Callback(uv_prepare_t *handle);
};

class CheckSource : public EventSource<CheckSource, uv_check_t>
{
public:
    explicit CheckSource(EventLoop *loop);
    ~CheckSource() override;
    virtual KeepInLoop checkDispatch() = 0;

protected:
    void startCheck();
    void stopCheck();

private:
    static void Callback(uv_check_t *handle);
};

class PollSource : public EventSource<PollSource, uv_poll_t>
{
public:
    explicit PollSource(EventLoop *loop, int fd);
    ~PollSource() override;

protected:
    void startPoll(int events);
    void stopPoll();
    virtual KeepInLoop pollDispatch(int status, int events) = 0;

private:
    static void Callback(uv_poll_t *handle, int status, int events);
};

} // namespace cocoa

#endif //COCOA_CORE_EVENTSOURCE_H
