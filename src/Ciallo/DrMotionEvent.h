#ifndef COCOA_DRMOTIONEVENT_H
#define COCOA_DRMOTIONEVENT_H

#include "Ciallo/DrBaseEvent.h"
CIALLO_BEGIN_NS

class DrMotionEvent : public DrBaseEvent
{
public:
    DrMotionEvent(TimeStamp time, DrScalar x, DrScalar y)
        : DrBaseEvent(Kind::kMotion, time),
          fX(x), fY(y) {}
    ~DrMotionEvent() override = default;

    inline DrScalar x() const
    {
        return fX;
    }

    inline DrScalar y() const
    {
        return fY;
    }

private:
    DrScalar    fX;
    DrScalar    fY;
};

CIALLO_END_NS
#endif //COCOA_DRMOTIONEVENT_H
