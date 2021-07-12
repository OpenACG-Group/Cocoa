#ifndef COCOA_VAXCBEVENTQUEUE_H
#define COCOA_VAXCBEVENTQUEUE_H

#include <thread>
#include <mutex>
#include <queue>

#include <xcb/xcb.h>
#include "Core/EventSource.h"
#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

class VaXcbDisplay;
class VaXcbEventQueue : public AsyncSource
{
public:
    explicit VaXcbEventQueue(VaXcbDisplay *display);
    ~VaXcbEventQueue() override;

    void disposeFromMainThread();

private:
    void asyncDispatch() override;

    bool tryEnqueue(xcb_generic_event_t *event);
    bool isDisposeEvent(xcb_generic_event_t *pEvent);
    void entrance();

    VaXcbDisplay                    *fDisplay;
    std::thread::id                  fMainThreadId;
    bool                             fDisposed;
    std::thread                      fThread;
    std::mutex                       fQueueMutex;
    std::queue<xcb_generic_event_t*> fQueue;
};

VANILLA_NS_END
#endif //COCOA_VAXCBEVENTQUEUE_H
