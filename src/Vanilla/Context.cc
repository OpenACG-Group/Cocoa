#include <sigc++/sigc++.h>

#include "Core/Journal.h"
#include "Vanilla/Context.h"
#include "Vanilla/VaDisplay.h"
VANILLA_NS_BEGIN

Context::Context(EventLoop *loop, Backend backend)
    : fEventLoop(loop),
      fBackend(backend)
{
}

void Context::connectTo(const char *displayName, int32_t id)
{
    if (fDisplays.contains(id))
    {
        Journal::Ref()(LOG_ERROR, "Display ID {} is unavailable", id);
        throw VanillaException(__func__, "Invalid display ID");
    }
    switch (fBackend)
    {
    case Backend::kXcb:
        fDisplays[id] = VaDisplay::OpenXcb(shared_from_this(), displayName);
    }
}

Handle<VaDisplay> Context::display(int32_t id)
{
    if (!fDisplays.contains(id))
    {
        Journal::Ref()(LOG_ERROR, "Display (ID:{}) haven't been opened yet", id);
        throw VanillaException(__func__, "Invalid Display ID");
    }
    return fDisplays[id];
}

Handle<Context> Context::Make(EventLoop *loop, Backend backend)
{
    Handle<Context> context = std::make_shared<Context>(loop, backend);
    return context;
}

VANILLA_NS_END
