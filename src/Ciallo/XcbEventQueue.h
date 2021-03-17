#ifndef COCOA_XCBEVENTQUEUE_H
#define COCOA_XCBEVENTQUEUE_H

#include <thread>
#include <queue>
#include <mutex>

#include "Ciallo/Ciallo.h"
#include "Ciallo/XcbConnection.h"
CIALLO_BEGIN_NS

class XcbEventQueue
{
    friend class XcbConnection;

public:
    explicit XcbEventQueue(XcbConnection *connection);
    ~XcbEventQueue();

    xcb_generic_event_t *take();

private:
    void run();

    void enqueue(xcb_generic_event_t *event);
    void sendCloseConnection();

private:
    XcbConnection          *fConnection;
    std::thread             fThread;
    bool                    fClosedConnection;
    std::queue<xcb_generic_event_t*>
                            fQueue;
    std::mutex              fQueueMutex;
};

CIALLO_END_NS
#endif //COCOA_XCBEVENTQUEUE_H
