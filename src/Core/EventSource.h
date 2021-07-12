#ifndef COCOA_EVENTSOURCE_H
#define COCOA_EVENTSOURCE_H

#include <functional>
#include <uv.h>

namespace cocoa
{

class EventLoop;

enum class KeepInLoop
{
    kYes,
    kNo
};

class EventSource
{
public:
    explicit EventSource(EventLoop *loop);
    virtual ~EventSource() = default;

    inline EventLoop *eventLoop()
    { return fLoop; }

private:
    EventLoop   *fLoop;
};

class TimerSource : public EventSource
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
    uv_timer_t  fUvTimer;
};

class AsyncSource : public EventSource
{
public:
    explicit AsyncSource(EventLoop *loop);
    ~AsyncSource() override;

protected:
    void disableAsync();
    void wakeupAsync();
    virtual void asyncDispatch() = 0;

private:
    static void Callback(uv_async_t *handle);
    uv_async_t  fUvAsync;
    bool fDisabled;
};

class CheckHandleSource : public EventSource
{
public:
    explicit CheckHandleSource(EventLoop *loop);
    ~CheckHandleSource() override;

protected:
    void startCheckHandle();
    void stopCheckHandle();
    virtual KeepInLoop checkHandleDispatch() = 0;

private:
    static void Callback(uv_check_t *handle);
    uv_check_t  fCheck;
};

class PollSource : public EventSource
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
    uv_poll_t   fUvPoll;
    bool        fStopped;
};

} // namespace cocoa

#endif //COCOA_EVENTSOURCE_H
