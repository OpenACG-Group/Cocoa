#ifndef COCOA_DRBUTTONRELEASEEVENT_H
#define COCOA_DRBUTTONRELEASEEVENT_H

#include "Ciallo/DrBaseEvent.h"
CIALLO_BEGIN_NS

class DrButtonReleaseEvent : public DrBaseEvent
{
public:
    DrButtonReleaseEvent(DrButton button, TimeStamp time, DrScalar x, DrScalar y)
            : DrBaseEvent(Kind::kButtonRelease, time),
              fButton(button), fX(x), fY(y) {}
    ~DrButtonReleaseEvent() override = default;

    inline DrButton button() const
    {
        return fButton;
    }

    inline DrScalar x() const
    {
        return fX;
    }

    inline DrScalar y() const
    {
        return fY;
    }

private:
    DrButton    fButton;
    DrScalar    fX;
    DrScalar    fY;
};

CIALLO_END_NS
#endif //COCOA_DRBUTTONRELEASEEVENT_H
