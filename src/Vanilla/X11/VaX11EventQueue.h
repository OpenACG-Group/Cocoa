#ifndef COCOA_VAX11EVENTQUEUE_H
#define COCOA_VAX11EVENTQUEUE_H

#include <thread>
#include <queue>
#include <mutex>
#include <X11/Xlib.h>

#include "Core/EventSource.h"
#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

class VaX11Display;
class VaX11EventQueue : public AsyncSource
{
public:
    explicit VaX11EventQueue(VaX11Display *display);
    ~VaX11EventQueue() override;

    void disposeInMainThread();

private:
    void dispatch() override;
    void entrance();
    bool isDisposeEvent(const UniqueHandle<XEvent>& event);

    VaX11Display               *fDisplay;
    bool                        fDisposed;
    std::thread                 fThread;
    std::mutex                  fQueueMutex;
    std::queue<UniqueHandle<XEvent>>
                                fQueue;
};

VANILLA_NS_END
#endif //COCOA_VAX11EVENTQUEUE_H
