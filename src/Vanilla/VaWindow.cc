#include "Vanilla/Base.h"
#include "Vanilla/VaWindow.h"
#include "Vanilla/VaDisplay.h"
VANILLA_NS_BEGIN

VaWindow::VaWindow(WeakHandle<VaDisplay> display, SkColorType format)
    : fDisplay(std::move(display)),
      fFormat(format)
{
}

Handle<VaWindow> VaWindow::Make(const Handle<VaDisplay>& display,
                                VaVec2f size, VaVec2f pos, Handle<VaWindow> parent)
{
    return display->createWindow(size, pos, std::move(parent));
}

void VaWindow::close()
{
    getDisplay()->detachWindow(shared_from_this());
    this->onClose();
}

void VaWindow::update()
{
    this->update(SkRect::MakeXYWH(0, 0,
                                  static_cast<SkScalar>(this->width()),
                                  static_cast<SkScalar>(this->height())));
}

Handle<Context> VaWindow::getContext()
{
    return fDisplay.lock()->getContext();
}

VANILLA_NS_END
