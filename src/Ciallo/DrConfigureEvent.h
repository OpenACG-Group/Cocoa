#ifndef COCOA_DRCONFIGUREEVENT_H
#define COCOA_DRCONFIGUREEVENT_H

#include "Ciallo/DrBaseEvent.h"
CIALLO_BEGIN_NS

class DrConfigureEvent : public DrBaseEvent
{
public:
    DrConfigureEvent(DrScalar x, DrScalar y, DrScalar w, DrScalar h, TimeStamp time)
        : DrBaseEvent(Kind::kConfigure, time),
          fX(x), fY(y), fWidth(w), fHeight(h) {}
    ~DrConfigureEvent() override = default;

    inline DrScalar x() const { return fX; }
    inline DrScalar y() const { return fY; }
    inline DrScalar width() const { return fWidth; }
    inline DrScalar height() const { return fHeight; }

private:
    DrScalar    fX;
    DrScalar    fY;
    DrScalar    fWidth;
    DrScalar    fHeight;
};

CIALLO_END_NS
#endif //COCOA_DRCONFIGUREEVENT_H
