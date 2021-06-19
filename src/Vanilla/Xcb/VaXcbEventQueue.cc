#include <thread>
#include <mutex>
#include <queue>
#include <cassert>

#include <xcb/xcb.h>

#include "Core/Journal.h"
#include "Vanilla/Base.h"
#include "Vanilla/Xcb/VaXcbDisplay.h"
#include "Vanilla/Xcb/VaXcbEventQueue.h"
#include "Vanilla/Context.h"
VANILLA_NS_BEGIN

VaXcbEventQueue::VaXcbEventQueue(VaXcbDisplay *display)
    : AsyncSource(display->getContext()->eventLoop()),
      fDisplay(display),
      fMainThreadId(std::this_thread::get_id()),
      fDisposed(false),
      fThread(&VaXcbEventQueue::entrance, this)
{
}

VaXcbEventQueue::~VaXcbEventQueue()
{
    disposeFromMainThread();
}

void VaXcbEventQueue::dispatch()
{
    fQueueMutex.lock();
    std::vector<xcb_generic_event_t*> events(fQueue.size());
    for (int32_t i = 0; !fQueue.empty(); i++)
    {
        events[i] = fQueue.front();
        fQueue.pop();
    }
    fQueueMutex.unlock();

    for (xcb_generic_event_t *ev : events)
    {
        fDisplay->handleEvent(ev);
        std::free(ev);
    }
}

void VaXcbEventQueue::disposeFromMainThread()
{
    if (fDisposed)
        return;
    assert(std::this_thread::get_id() == fMainThreadId);

    xcb_client_message_event_t event{};
    std::memset(&event, 0xff, sizeof(event));

    xcb_connection_t *connection = fDisplay->connection();
    xcb_window_t fakeWindow = xcb_generate_id(connection);
    xcb_create_window(connection,
                      XCB_COPY_FROM_PARENT,
                      fakeWindow,
                      fDisplay->screen()->root,
                      0, 0, 1, 1, 0,
                      XCB_WINDOW_CLASS_INPUT_ONLY,
                      fDisplay->screen()->root_visual,
                      0, nullptr);

    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 32;
    event.sequence = 0;
    event.window = fakeWindow;
    event.type = fDisplay->atom(VaXcbAtoms::_VA_CLOSE_CONNECTION);
    event.data.data32[0] = 0;

    xcb_send_event(connection, false, fakeWindow,
                   XCB_EVENT_MASK_NO_EVENT,
                   reinterpret_cast<const char*>(&event));
    xcb_destroy_window(connection, fakeWindow);
    xcb_flush(connection);

    if (fThread.joinable())
        fThread.join();

    AsyncSource::disableAsync();
}

bool VaXcbEventQueue::isDisposeEvent(xcb_generic_event_t *pEvent)
{
    if (pEvent && (pEvent->response_type & ~0x80) == XCB_CLIENT_MESSAGE)
    {
        auto msg = reinterpret_cast<xcb_client_message_event_t*>(pEvent);
        if (msg->type == fDisplay->atom(VaXcbAtoms::_VA_CLOSE_CONNECTION))
            return true;
    }
    return false;
}

bool VaXcbEventQueue::tryEnqueue(xcb_generic_event_t *event)
{
    if (isDisposeEvent(event))
        return true;
    fQueueMutex.lock();
    fQueue.push(event);
    fQueueMutex.unlock();
    return false;
}

void VaXcbEventQueue::entrance()
{
#if defined(__linux__)
    pthread_setname_np(pthread_self(), "XcbEventQueue");
#endif
    Journal::Ref()(LOG_INFO, "X11 event queue thread has been started");

    xcb_connection_t *connection = fDisplay->connection();
    xcb_generic_event_t *event;
    bool shouldExit = false;
    while (!shouldExit && (event = xcb_wait_for_event(connection)) != nullptr)
    {
        shouldExit = tryEnqueue(event);
        while (!shouldExit && (event = xcb_poll_for_queued_event(connection)) != nullptr)
            shouldExit = tryEnqueue(event);
        AsyncSource::wakeupAsync();
    }

    if (shouldExit)
        Journal::Ref()(LOG_INFO, "X11 event queue thread exited normally");
    else
        Journal::Ref()(LOG_WARNING, "X11 event queue thread exited because of an error");
}

VANILLA_NS_END
