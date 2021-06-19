#include "Vanilla/VaDisplay.h"
VANILLA_NS_BEGIN

VaDisplay::VaDisplay(DisplayBackend backend, const Handle<Context>& ctx)
    : fBackend(backend),
      fContext(ctx)
{
}

Handle<VaWindow> VaDisplay::createWindow(VaVec2f size, VaVec2f pos, Handle<VaWindow> parent)
{
    Handle<VaWindow> window = onCreateWindow(size, pos, std::move(parent));
    fWindows.push_back(window);
    return window;
}

void VaDisplay::forEachWindow(const std::function<bool(Handle<VaWindow>)>& func)
{
    for (auto& window : fWindows)
    {
        if (!func(window))
            break;
    }
}

void VaDisplay::detachWindow(Handle<VaWindow> window)
{
    fWindows.remove_if([&window](const Handle<VaWindow>& current) -> bool {
        return window == current;
    });
}

VANILLA_NS_END
