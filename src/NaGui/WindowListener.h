#ifndef COCOA_WINDOWLISTENER_H
#define COCOA_WINDOWLISTENER_H

#include "Ciallo/DrawableListener.h"
#include "NaGui/Base.h"
#include "NaGui/Window.h"
NAGUI_NS_BEGIN

// Relationship between these classes:
// (A -> B means you can get B by A)
//
// Drawable -> NaWindowListener -> NaWindow
//    ^                              |
//    `------------------------------'
class WindowListener : public ciallo::DrawableListener
{
public:
    explicit WindowListener(Window::Ptr window);
    ~WindowListener() override = default;

    bool onClose() override;
    void onRender(ciallo::DrEventPtr<ciallo::DrRepaintEvent> event) override;
    void onButtonPress(ciallo::DrEventPtr<ciallo::DrButtonPressEvent> event) override;
    void onButtonRelease(ciallo::DrEventPtr<ciallo::DrButtonReleaseEvent> event) override;
    void onMotion(ciallo::DrEventPtr<ciallo::DrMotionEvent> event) override;
    void onConfigure(ciallo::DrEventPtr<ciallo::DrConfigureEvent> event) override;

private:
    Window::Ptr       fWindow;
    Vec2i             fLastConfigureSize;
};

NAGUI_NS_END
#endif //COCOA_WINDOWLISTENER_H
