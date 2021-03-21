#ifndef COCOA_DRBASEEVENT_H
#define COCOA_DRBASEEVENT_H

#include <memory>

#include "Ciallo/Ciallo.h"
CIALLO_BEGIN_NS

using DrScalar = double;
enum class DrButton
{
    kLeftButton,
    kMiddleButton,
    kRightButton
};

template<typename T>
using DrEventPtr = std::shared_ptr<T>;

template<typename T, typename...ArgsT>
DrEventPtr<T> DrMakeEvent(ArgsT&&... args)
{
    return std::make_shared<T>(std::forward<ArgsT>(args)...);
}

class DrBaseEvent : public std::enable_shared_from_this<DrBaseEvent>
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

    using TimeStamp = uint64_t;

    DrBaseEvent(Kind kind, TimeStamp stamp);
    virtual ~DrBaseEvent() = default;

    Kind kind() const;
    TimeStamp timeStamp() const;

    template<typename T>
    std::shared_ptr<T> cast() {
        return shared_from_this();
    }

private:
    Kind        fKind;
    TimeStamp   fTimeStamp;
};

CIALLO_END_NS
#endif //COCOA_DRBASEEVENT_H
