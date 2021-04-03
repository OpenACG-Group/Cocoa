#include "NaGui/Context.h"
#include "NaGui/WindowListener.h"
NAGUI_NS_BEGIN

WindowListener::WindowListener(Window::Ptr window)
        : fWindow(std::move(window)),
          fLastConfigureSize(0, 0)
{
}

void WindowListener::onConfigure(ciallo::DrEventPtr<ciallo::DrConfigureEvent> event)
{
    Vec2i newSize(event->width(), event->height());
    if (newSize == fLastConfigureSize)
        return;
    fWindow->fSurface->resize(newSize.s0, newSize.s1);
}

bool WindowListener::onClose()
{
    Context::Ref().collectClosedWindowLater(fWindow->drawable());
    return false;
}

void WindowListener::onRender(ciallo::DrEventPtr<ciallo::DrRepaintEvent> event)
{
    fWindow->paintEvent();
}

#define DW fWindow->fDraw
void WindowListener::onButtonPress(ciallo::DrEventPtr<ciallo::DrButtonPressEvent> event)
{
    DW->_privBeginInput();
    switch (event->button())
    {
    case ciallo::DrButton::kLeftButton:
        DW->_privMouseLeftButton(true, Vec2i(event->x(), event->y()));
        break;

    case ciallo::DrButton::kRightButton:
        DW->_privMouseRightButton(true, Vec2i(event->x(), event->y()));
        break;

    case ciallo::DrButton::kMiddleButton:
        DW->_privMouseMiddleButton(true, Vec2i(event->x(), event->y()));
        break;
    }
    DW->_privEndInput();
    fWindow->paint();
}

void WindowListener::onButtonRelease(ciallo::DrEventPtr<ciallo::DrButtonReleaseEvent> event)
{
    DW->_privBeginInput();
    switch (event->button())
    {
    case ciallo::DrButton::kLeftButton:
        DW->_privMouseLeftButton(false, Vec2i(event->x(), event->y()));
        break;

    case ciallo::DrButton::kRightButton:
        DW->_privMouseRightButton(false, Vec2i(event->x(), event->y()));
        break;

    case ciallo::DrButton::kMiddleButton:
        DW->_privMouseMiddleButton(false, Vec2i(event->x(), event->y()));
        break;
    }
    DW->_privEndInput();
    fWindow->paint();
}

void WindowListener::onMotion(ciallo::DrEventPtr<ciallo::DrMotionEvent> event)
{
    DW->_privBeginInput();
    DW->_privMouseMotion(Vec2i(event->x(), event->y()));
    DW->_privEndInput();
    fWindow->paint();
}

NAGUI_NS_END
