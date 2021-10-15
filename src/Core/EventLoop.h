#ifndef COCOA_EVENTLOOP_H
#define COCOA_EVENTLOOP_H

#include <functional>

#include <uv.h>

#include "Core/UniquePersistent.h"

namespace cocoa
{

class EventLoop : public UniquePersistent<EventLoop>
{
public:
    EventLoop();
    ~EventLoop();
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    int run();
    void walk(std::function<void(uv_handle_t*)> function);
    inline uv_loop_t *handle()
    { return fLoop; }

    void dispose();

private:
    uv_loop_t   *fLoop;
};

} // namespace cocoa
#endif //COCOA_EVENTLOOP_H
