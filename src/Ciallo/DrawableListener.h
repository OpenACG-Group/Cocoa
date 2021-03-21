#ifndef COCOA_DRAWABLELISTENER_H
#define COCOA_DRAWABLELISTENER_H

#include <memory>

#include "Ciallo/Ciallo.h"
#include "Ciallo/DrRepaintEvent.h"
#include "Ciallo/DrButtonPressEvent.h"
#include "Ciallo/DrButtonReleaseEvent.h"
#include "Ciallo/DrMotionEvent.h"
CIALLO_BEGIN_NS

class DrawableListener
{
public:
    virtual ~DrawableListener() = default;

    template<typename T, typename... ArgsT>
    static std::shared_ptr<T> Make(ArgsT&&... args)
    {
        return std::make_shared<T>(std::forward<ArgsT>(args)...);
    }

    virtual void onButtonPress(DrEventPtr<DrButtonPressEvent> event);
    virtual void onButtonRelease(DrEventPtr<DrButtonReleaseEvent> event);
    virtual void onMotion(DrEventPtr<DrMotionEvent> event);
    virtual void onRender(DrEventPtr<DrRepaintEvent> event);
    virtual void onEnter();
    virtual void onLeave();
    virtual void onFocusIn();
    virtual void onFocusOut();

    /**
     * onClose() will be called when the window needs close.
     * @return True if the window should be closed.
     *         False if the window shouldn't be closed.
     */
    virtual bool onClose();

    /**
     * onDestroy() will be called when the window will be destroyed.
     */
    virtual void onDestroy();
};

CIALLO_END_NS
#endif //COCOA_DRAWABLELISTENER_H
