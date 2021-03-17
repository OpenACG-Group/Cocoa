#ifndef COCOA_EVENTDISPATCHER_H
#define COCOA_EVENTDISPATCHER_H

#include <mutex>
#include <queue>

#include "Core/UniquePersistent.h"
#include "Core/BaseEventProcessor.h"

namespace cocoa {

class EventDispatcher : public UniquePersistent<EventDispatcher>
{
public:
    EventDispatcher();
    ~EventDispatcher();

    /**
     * @brief Notify the event dispatcher we have a new event.
     *        Usually wakeup() is called by other threads.
     */
    void wakeup(BaseEventProcessor *processor);
    int32_t wait() const;

    void handleEvents(int32_t num);
    void dispose();

private:
    bool                    fDispose;
    int                     fEventFd;
    std::mutex              fQueueMutex;
    std::queue<BaseEventProcessor*> fProcessorQueue;
};

}

#endif //COCOA_EVENTDISPATCHER_H
