#ifndef COCOA_DRBUTTONPRESSEVENT_H
#define COCOA_DRBUTTONPRESSEVENT_H

#include "Ciallo/DrBaseEvent.h"
CIALLO_BEGIN_NS

class DrButtonPressEvent : public DrBaseEvent
{
public:
    DrButtonPressEvent(DrButton button, TimeStamp time, DrScalar x, DrScalar y)
        : DrBaseEvent(Kind::kButtonPress, time),
          fButton(button), fX(x), fY(y) {}
    ~DrButtonPressEvent() override = default;

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
#endif //COCOA_DRBUTTONPRESSEVENT_H
