#include "Ciallo/Drawable.h"
CIALLO_BEGIN_NS

void DrawableListener::onButtonPress(DrEventPtr<DrButtonPressEvent>)        {}
void DrawableListener::onButtonRelease(DrEventPtr<DrButtonReleaseEvent>)    {}
void DrawableListener::onMotion(DrEventPtr<DrMotionEvent>)  {}
void DrawableListener::onFocusIn()  {}
void DrawableListener::onFocusOut() {}
void DrawableListener::onEnter()    {}
void DrawableListener::onLeave()    {}

void DrawableListener::onRender(DrEventPtr<DrRepaintEvent>) {}
bool DrawableListener::onClose()    { return true; }
void DrawableListener::onDestroy()  {}
void DrawableListener::onConfigure(DrEventPtr<DrConfigureEvent>)            {}

Drawable::Drawable(Backend backend)
    : fBackend(backend),
      fListener(std::make_shared<DrawableListener>())
{
}

void Drawable::setListener(const std::shared_ptr<DrawableListener>& listener)
{
    fListener = listener;
}

DrawableListener *Drawable::listener()
{
    return fListener.get();
}

CIALLO_END_NS
