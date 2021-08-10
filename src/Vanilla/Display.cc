#include "Vanilla/Display.h"
VANILLA_NS_BEGIN

Display::Display(DisplayBackend backend, const Handle<Context>& ctx)
    : fBackend(backend),
      fContext(ctx)
{
}

Handle<Window> Display::createWindow(vec::float2 size, vec::float2 pos, Handle<Window> parent)
{
    Handle<Window> window = onCreateWindow(size, pos, std::move(parent));
    fWindows.push_back(window);
    return window;
}

void Display::forEachWindow(const std::function<bool(Handle<Window>)>& func)
{
    for (auto& window : fWindows)
    {
        if (!func(window))
            break;
    }
}

void Display::detachWindow(Handle<Window> window)
{
    fWindows.remove_if([&window](const Handle<Window>& current) -> bool {
        return window == current;
    });
}

VANILLA_NS_END
