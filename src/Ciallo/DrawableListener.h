#ifndef COCOA_DRAWABLELISTENER_H
#define COCOA_DRAWABLELISTENER_H

#include "Ciallo/Ciallo.h"
CIALLO_BEGIN_NS

class DrawableListener
{
public:
    virtual ~DrawableListener() = default;

    virtual void onRender();

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
