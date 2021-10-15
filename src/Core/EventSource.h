#ifndef COCOA_EVENTSOURCE_H
#define COCOA_EVENTSOURCE_H

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
    virtual ~EventSource() {
        if (fHandle)
        {
            close();
        }
    }

    void close()
    {
        uv_close(fHandle, [](uv_handle_t *handle) -> void {
            std::free(handle);
        });
        fHandle = nullptr;
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

class LoopPrologueSource : public EventSource<LoopPrologueSource, uv_prepare_t>
{
public:
    explicit LoopPrologueSource(EventLoop *loop);
    ~LoopPrologueSource() override;

protected:
    void startLoopPrologue();
    void stopLoopPrologue();
    virtual KeepInLoop loopPrologueDispatch() = 0;

private:
    static void Callback(uv_prepare_t *handle);
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

#endif //COCOA_EVENTSOURCE_H
