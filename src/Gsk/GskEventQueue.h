#ifndef COCOA_GSKEVENTQUEUE_H
#define COCOA_GSKEVENTQUEUE_H

#include <list>

#include "Gsk/Gsk.h"
GSK_NAMESPACE_BEGIN

class GskEvent;
class GskDisplay;

class GskEventQueue
{
public:
    using QueueT = std::list<Handle<GskEvent>>;
    using QueueIterator = QueueT::iterator;

    explicit GskEventQueue(const Weak<GskDisplay>& display);
    ~GskEventQueue() = default;

    static void Emit(const Handle<GskEvent>& event);

    g_nodiscard g_inline Handle<GskDisplay> getDisplay() const {
        return fDisplay.lock();
    }

    g_nodiscard g_inline int getPauseCount() const {
        return fEventPauseCount;
    }

    g_nodiscard g_inline bool isReading() const {
        return fReading;
    }

    void g_inline setReading(bool reading) {
        fReading = reading;
    }

    void increasePauseCount();
    void decreasePauseCount();

    void push(const Handle<GskEvent>& event);
    Handle<GskEvent> popFirstEvent();
    QueueIterator getFirstNotFilledEventIterator();
    QueueIterator endIterator();

    void flush();

private:
    Weak<GskDisplay>    fDisplay;
    int                 fEventPauseCount;
    QueueT              fQueue;
    bool                fReading;
};

GSK_NAMESPACE_END
#endif //COCOA_GSKEVENTQUEUE_H
