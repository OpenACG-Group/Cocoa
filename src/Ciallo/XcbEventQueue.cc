#include <thread>
#include <iostream>

#include "Core/EventDispatcher.h"
#include "Ciallo/Ciallo.h"
#include "Ciallo/XcbEventQueue.h"
#include "Ciallo/XcbConnection.h"
CIALLO_BEGIN_NS

XcbEventQueue::XcbEventQueue(XcbConnection *connection)
    : fConnection(connection),
      fThread(&XcbEventQueue::run, this),
      fClosedConnection(false)
{
}

XcbEventQueue::~XcbEventQueue()
{
    if (fThread.joinable())
    {
        sendCloseConnection();
        fThread.join();
    }

    while (!fQueue.empty())
    {
        std::free(fQueue.front());
        fQueue.pop();
    }
}

void XcbEventQueue::run()
{
#ifdef __linux__
    pthread_setname_np(pthread_self(), "XcbEventQueue");
#endif /* __linux__ */

    xcb_generic_event_t *event = nullptr;
    while (!fClosedConnection &&
           (event = xcb_wait_for_event(fConnection->nativeHandle())))
    {
        {
            std::scoped_lock<std::mutex> lock(fQueueMutex);
            this->enqueue(event);
            /*
            while (!fClosedConnection &&
                   (event = xcb_poll_for_queued_event(fConnection->nativeHandle())))
                this->enqueue(event);
            */
        }

        EventDispatcher::Instance()->wakeup(fConnection);
    }

    /* Notify event dispatcher we're dead, event loop should exit */
    EventDispatcher::Instance()->dispose();
}

void XcbEventQueue::enqueue(xcb_generic_event_t *event)
{
    if (!event)
        return;

    auto type = event->response_type & ~0x80;
    if (type == XCB_CLIENT_MESSAGE)
    {
        auto *pClient = reinterpret_cast<const xcb_client_message_event_t*>(event);
        if (pClient->type == fConnection->atom(XcbAtom::Atom::CA_CLOSE_CONNECTION))
            fClosedConnection = true;
    }
    fQueue.push(event);
}

void XcbEventQueue::sendCloseConnection()
{
    xcb_client_message_event_t event{};

    xcb_connection_t *conn = fConnection->nativeHandle();
    const xcb_window_t window = xcb_generate_id(conn);
    xcb_screen_iterator_t it = xcb_setup_roots_iterator(fConnection->setup());
    xcb_screen_t *screen = it.data;

    xcb_create_window(conn,
                      XCB_COPY_FROM_PARENT,
                      window,
                      screen->root,
                      0, 0, 1, 1, 0,
                      XCB_WINDOW_CLASS_INPUT_ONLY,
                      screen->root_visual,
                      0, nullptr);
    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 32;
    event.sequence = 0;
    event.window = window;
    event.type = fConnection->atom(XcbAtom::Atom::CA_CLOSE_CONNECTION);
    event.data.data32[0] = 0;

    xcb_send_event(conn, false, window, XCB_EVENT_MASK_NO_EVENT,
                   reinterpret_cast<const char*>(&event));
    xcb_destroy_window(conn, window);
    xcb_flush(conn);
}

xcb_generic_event_t *XcbEventQueue::take()
{
    xcb_generic_event_t *ptr;
    {
        std::scoped_lock<std::mutex> lock(fQueueMutex);
        if (fQueue.empty())
            return nullptr;
        ptr = fQueue.front();
        fQueue.pop();
    }
    return ptr;
}

CIALLO_END_NS
