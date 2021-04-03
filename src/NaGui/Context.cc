#include "Core/BaseEventProcessor.h"
#include "Core/EventDispatcher.h"
#include "Core/Journal.h"
#include "Core/Exception.h"
#include "NaGui/Context.h"
NAGUI_NS_BEGIN

class WindowCollector : public BaseEventProcessor
{
public:
    ~WindowCollector() override = default;
    void processEvent() override;
};

void WindowCollector::processEvent()
{
    auto& queue = Context::Ref().fClosedwindows;
    if (queue.empty())
    {
        log_write(LOG_ERROR) << "NaGui: We're notified to collect dead windows"
                             << " but there's noting in the queue." << log_endl;
        return;
    }

    auto *ptr = queue.back();
    queue.pop();
    Context::Ref().removeDrawable(ptr);
}

Context::Context()
        : fCollector(new WindowCollector())
{
}

Context::~Context()
{
    for (ciallo::Drawable *ptr : fDrawables)
        delete ptr;
    delete fCollector;
}

void Context::addDrawable(ciallo::Drawable *pDrawable)
{
    if (!pDrawable)
        return;
    fDrawables.push_back(pDrawable);
}

void Context::removeDrawable(ciallo::Drawable *pDrawable)
{
    if (!pDrawable)
        return;

    bool found = false;
    fDrawables.remove_if([pDrawable, &found](ciallo::Drawable *ptr) -> bool {
        if (ptr == pDrawable)
        {
            found = true;
            return true;
        }
        return false;
    });

    if (found)
    {
        /**
         * Our listener owns some resources of Drawable,
         * resetting the listener to a fake one will destruct
         * the real listener (NaWindowListener), then we can destruct
         * Drawable safely.
         */
        pDrawable->setListener(ciallo::DrawableListener::Make<ciallo::DrawableListener>());
        delete pDrawable;
    }
}

void Context::collectClosedWindowLater(ciallo::Drawable *pDrawable)
{
    fClosedwindows.push(pDrawable);
    EventDispatcher::Ref().wakeup(fCollector);
}

NAGUI_NS_END
