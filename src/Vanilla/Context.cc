#include <sigc++/sigc++.h>
#include "Vanilla/Context.h"
#include "Vanilla/VaDisplay.h"
VANILLA_NS_BEGIN

Context::Context(EventLoop *loop, Handle<VaDisplay> display)
    : fEventLoop(loop),
      fDisplay(std::move(display))
{
}

Handle<Context> Context::MakeX11(EventLoop *loop, const char *displayName)
{
    Handle<VaDisplay> display = VaDisplay::OpenX11(loop, displayName);
    Handle<Context> context = std::make_shared<Context>(loop, display);
    display->attachContext(context);

    return context;
}

VANILLA_NS_END
