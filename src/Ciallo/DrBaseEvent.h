#ifndef COCOA_DBASEEVENT_H
#define COCOA_DBASEEVENT_H

#include <memory>

#include "Ciallo/Ciallo.h"
CIALLO_BEGIN_NS

class DBaseEvent
{
public:
    enum class Kind
    {
        kButtonPress,
        kButtonRelease,
        kMotion,
        kFocusIn,
        kFocusOut,
        kRepaint,
        kClose,
        kDestroy
    };

    using Ptr = std::shared_ptr<DBaseEvent>;

    explicit DBaseEvent(Kind kind);
    virtual ~DBaseEvent() = default;

    Kind kind() const;

private:
    Kind        fKind;
};

CIALLO_END_NS
#endif //COCOA_DBASEEVENT_H
