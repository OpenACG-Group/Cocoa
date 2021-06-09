#ifndef COCOA_EVENTSOURCE_H
#define COCOA_EVENTSOURCE_H

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

class PollSource : public EventSource
{
public:
    explicit PollSource(EventLoop *loop, int fd);
    ~PollSource() override;

protected:
    void startPoll(int events);
    void stopPoll();
    virtual KeepInLoop dispatch(int status, int events) = 0;

private:
    static void Callback(uv_poll_t *handle, int status, int events);
    uv_poll_t   fUvPoll;
    bool        fStopped;
};

} // namespace cocoa

#endif //COCOA_EVENTSOURCE_H
