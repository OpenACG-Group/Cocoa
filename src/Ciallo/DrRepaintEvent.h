#ifndef COCOA_DRREPAINTEVENT_H
#define COCOA_DRREPAINTEVENT_H

#include "Ciallo/DrBaseEvent.h"
CIALLO_BEGIN_NS

class DrRepaintEvent : public DrBaseEvent
{
public:
    DrRepaintEvent(TimeStamp time,
                   DrScalar x, DrScalar y,
                   DrScalar width, DrScalar height)
        : DrBaseEvent(Kind::kRepaint, time),
          fX(x), fY(y),
          fWidth(width), fHeight(height) {}

    ~DrRepaintEvent() override = default;

    inline DrScalar x() const
    {
        return fX;
    }

    inline DrScalar y() const
    {
        return fY;
    }

    inline DrScalar width() const
    {
        return fWidth;
    }

    inline DrScalar height() const
    {
        return fHeight;
    }

private:
    DrScalar    fX;
    DrScalar    fY;
    DrScalar    fWidth;
    DrScalar    fHeight;
};

CIALLO_END_NS
#endif //COCOA_DRREPAINTEVENT_H
