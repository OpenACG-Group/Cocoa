#include "Vanilla/Base.h"
#include "Vanilla/Window.h"
#include "Vanilla/Display.h"
VANILLA_NS_BEGIN

Window::Window(WeakHandle<Display> display, SkColorType format)
    : fDisplay(std::move(display)),
      fFormat(format)
{
}

Handle<Window> Window::Make(const Handle<Display>& display,
                                vec::float2 size, vec::float2 pos, Handle<Window> parent)
{
    return display->createWindow(size, pos, std::move(parent));
}

void Window::close()
{
    getDisplay()->detachWindow(shared_from_this());
    this->onClose();
}

void Window::update()
{
    this->update(SkRect::MakeXYWH(0, 0,
                                  static_cast<SkScalar>(this->width()),
                                  static_cast<SkScalar>(this->height())));
}

Handle<Context> Window::getContext()
{
    return fDisplay.lock()->getContext();
}

VANILLA_NS_END
