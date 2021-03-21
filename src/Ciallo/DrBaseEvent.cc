#include "Ciallo/DrBaseEvent.h"
CIALLO_BEGIN_NS

DrBaseEvent::DrBaseEvent(Kind kind, TimeStamp timeStamp)
    : fKind(kind),
      fTimeStamp(timeStamp)
{
}

DrBaseEvent::Kind DrBaseEvent::kind() const
{
    return fKind;
}

DrBaseEvent::TimeStamp DrBaseEvent::timeStamp() const
{
    return fTimeStamp;
}

CIALLO_END_NS
