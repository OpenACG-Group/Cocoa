#include "Core/Errors.h"

#include "Gsk/GskEventQueue.h"
#include "Gsk/GskEvent.h"
GSK_NAMESPACE_BEGIN

void GskEventQueue::Emit(const Handle<GskEvent>& event)
{
    // TODO(Important): Emitting events here.
}

GskEventQueue::GskEventQueue(const Weak<GskDisplay>& display)
    : fDisplay(display)
    , fEventPauseCount(0)
    , fReading(false)
{
    CHECK(!display.expired());
}

void GskEventQueue::increasePauseCount()
{
    ++fEventPauseCount;
}

void GskEventQueue::decreasePauseCount()
{
    --fEventPauseCount;
}

GskEventQueue::QueueIterator GskEventQueue::endIterator()
{
    return fQueue.end();
}

GskEventQueue::QueueIterator GskEventQueue::getFirstNotFilledEventIterator()
{
    bool paused = fEventPauseCount > 0;
    auto pending = fQueue.end();

    for (auto itr = fQueue.begin(); itr != fQueue.end(); itr++)
    {
        Handle<GskEvent> event = *itr;
        if (!(event->getFlagsRef() & GSK_EVENT_FLAG_PENDING) &&
            (!paused || (event->getFlagsRef() & GSK_EVENT_FLAG_FLUSHED)))
        {
            if (pending != fQueue.end())
                return pending;
            using T = GskEventType;
            using D = GskScrollDirection;

            T type = event->getEventType();
            if ((type == T::kMotionNotify ||
                 (type == T::kScroll && GskEvent::As<GskScrollEvent>(event)->getDirection() == D::kSmooth)) &&
                !(event->getFlagsRef() & GSK_EVENT_FLAG_FLUSHED))
            {
                pending = itr;
            }
            else
            {
                return itr;
            }
        }
    }
    return fQueue.end();
}

Handle<GskEvent> GskEventQueue::popFirstEvent()
{
    auto itr = getFirstNotFilledEventIterator();
    Handle<GskEvent> event;

    if (itr != endIterator())
    {
        event = *itr;
        fQueue.erase(itr);
    }

    return event;
}

void GskEventQueue::push(const Handle<GskEvent>& event)
{
    fQueue.emplace_back(event);
}

void GskEventQueue::flush()
{
    while (!fQueue.empty())
    {
        Handle<GskEvent> event = fQueue.front();
        fQueue.pop_front();

        event->getFlagsRef() |= GSK_EVENT_FLAG_FLUSHED;
        GskEventQueue::Emit(event);
    }
}

GSK_NAMESPACE_END
