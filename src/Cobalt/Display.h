#ifndef COCOA_COBALT_DISPLAY_H
#define COCOA_COBALT_DISPLAY_H

#include "Core/EventLoop.h"
#include "Cobalt/Cobalt.h"
COBALT_NAMESPACE_BEGIN

class Display : public std::enable_shared_from_this<Display>
{
public:
    static co_sp<Display> Connect(EventLoop *loop, const std::string& name);

    explicit Display(EventLoop *eventLoop);
    virtual ~Display();

    g_nodiscard g_inline EventLoop *GetEventLoop() const {
        return event_loop_;
    }

    void Close();

    g_signal_getter(Close);

protected:
    virtual void OnDispose() = 0;

private:
    g_signal_fields(
        g_signal_signature(void(const co_sp<Display>&), Close)
    )

    EventLoop           *event_loop_;
    bool                 has_disposed_;
};

COBALT_NAMESPACE_END
#endif //COCOA_COBALT_DISPLAY_H
