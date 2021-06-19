#include <sys/poll.h>

#include <iostream>
#include <thread>
#include <queue>
#include <X11/Xlib.h>

#include "Core/Journal.h"
#include "Vanilla/Base.h"
#include "Vanilla/X11/VaX11EventQueue.h"
#include "Vanilla/X11/VaX11Display.h"
#include "Vanilla/X11/VaX11Window.h"
#include "Vanilla/Context.h"
VANILLA_NS_BEGIN

VaX11EventQueue::VaX11EventQueue(VaX11Display *display)
    : AsyncSource(display->getContext()->eventLoop()),
      fDisplay(display),
      fDisposed(false),
      fThread(&VaX11EventQueue::entrance, this)
{
}

VaX11EventQueue::~VaX11EventQueue()
{
    if (!fDisposed)
        disposeInMainThread();
}

void VaX11EventQueue::dispatch()
{
    fQueueMutex.lock();
    std::vector<UniqueHandle<XEvent>> events(fQueue.size());
    for (int32_t i = 0; !fQueue.empty(); i++)
    {
        events[i].swap(fQueue.front());
        fQueue.pop();
    }
    fQueueMutex.unlock();
    fDisplay->eventQueueDispatch(events);
}

void VaX11EventQueue::entrance()
{
#if defined(__linux__)
    pthread_setname_np(pthread_self(), "X11EventQueue");
#endif

    struct pollfd pfd{
        .fd = XConnectionNumber(fDisplay->display()),
        .events = POLLIN
    };
    while (true)
    {
        int ret = poll(&pfd, 1, -1);
        if (!XEventsQueued(fDisplay->display(), QueuedAlready))
        {
            Journal::Ref()(LOG_DEBUG, "Fake trigger");
            continue;
        }

        UniqueHandle<XEvent> event = std::make_unique<XEvent>();
        XNextEvent(fDisplay->display(), event.get());
        if (isDisposeEvent(event))
            break;

        fQueueMutex.lock();
        fQueue.emplace(std::move(event));
        fQueueMutex.unlock();

        /* Wakeup our main thread to handle events */
        this->wakeupAsync();
    }
}

void VaX11EventQueue::disposeInMainThread()
{
    if (!fThread.joinable())
        return;

    // Create a fake window to send some event
    Display *dis = fDisplay->display();
    Window win = XCreateSimpleWindow(dis,
                                     fDisplay->screen()->root,
                                     0, 0, 1, 1, 0, 0,
                                     XBlackPixelOfScreen(fDisplay->screen()));
    XEvent event;
    event.type = ClientMessage;
    event.xclient.window = win;
    event.xclient.display = dis;
    event.xclient.message_type = fDisplay->atoms().get(VaX11Atoms::VA_CLOSE_CONNECTION);
    event.xclient.send_event = True;
    event.xclient.format = 32;
    event.xclient.serial = 0;
    event.xclient.data.l[0] = 0;
    XSendEvent(dis, win, False, NoEventMask, &event);
    XFlush(dis);

    fThread.join();

    XDestroyWindow(dis, win);
    XFlush(dis);
    fDisposed = true;
}

bool VaX11EventQueue::isDisposeEvent(const UniqueHandle<XEvent>& event)
{
    if (event->type == ClientMessage)
    {
        Journal::Ref()(LOG_DEBUG, "Get ClientMessage, win = {}", event->xclient.window);
    }
    if (event->type == ClientMessage &&
        event->xclient.message_type == fDisplay->atoms().get(VaX11Atoms::VA_CLOSE_CONNECTION))
    {
        return true;
    }
    return false;
}

VANILLA_NS_END
