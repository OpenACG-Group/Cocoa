#include "NaGui/Context.h"
#include "NaGui/Window.h"
#include "NaGui/WindowListener.h"
NAGUI_NS_BEGIN

Window::Window()
        : fDrawable(nullptr),
          fSurface(nullptr),
          fDraw(nullptr)
{
}

void Window::paint()
{
    if (fDrawable)
        fDrawable->repaint();
}

void Window::_privSetSurface(ciallo::CrSurface::Ptr surface)
{
    fSurface = std::move(surface);
    fDraw = std::make_unique<Draw>(fSurface);
}

void Window::_privSetDrawable(ciallo::Drawable *pDrawable)
{
    fDrawable = pDrawable;
}

void Window::paintEvent()
{
    this->onRepaint(*fDraw);
    fDraw->render();
}

// -------------------------------------------------------------------------------------

void BindDrawableWindow(ciallo::Drawable *pDrawable,
                        Window::Ptr window)
{
    window->_privSetDrawable(pDrawable);
    window->_privSetSurface(ciallo::CrSurface::MakeFromDrawable(pDrawable));

    auto listener = std::make_shared<WindowListener>(std::move(window));
    pDrawable->setListener(listener);
    Context::Ref().addDrawable(pDrawable);
}

NAGUI_NS_END
