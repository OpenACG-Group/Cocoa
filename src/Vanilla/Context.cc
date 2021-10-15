#include <sigc++/sigc++.h>
#include <cassert>

#include "Core/Journal.h"
#include "Vanilla/Context.h"
#include "Vanilla/Display.h"
VANILLA_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Vanilla)

Context::Context(EventLoop *loop, Backend backend)
    : fEventLoop(loop),
      fBackend(backend)
{
    LOGF(LOG_INFO, "Cocoa/Vanilla 2D rendering engine, version {}.{}",
                   VANILLA_MAJOR_VERSION, VANILLA_MINOR_VERSION)
}

Context::~Context()
{
    for (auto& pair : fDisplays)
    {
        if (pair.second)
        {
            assert(pair.second.use_count() == 1);
            pair.second->dispose();
        }
    }
    LOGW(LOG_INFO, "Destroying Cocoa/Vanilla 2D rendering engine")
}

void Context::detachDisplay(int32_t id)
{
    fDisplays.erase(id);
}

bool Context::allDisplaysAreUnique()
{
    for (auto& pair : fDisplays)
    {
        if (pair.second && !pair.second.unique())
            return false;
    }
    return true;
}

void Context::connectTo(const char *displayName, int32_t id)
{
    if (fDisplays.contains(id))
    {
        LOGF(LOG_ERROR, "Display ID {} is unavailable", id)
        throw VanillaException(__func__, "Invalid display ID");
    }
    switch (fBackend)
    {
    case Backend::kXcb:
        fDisplays[id] = Display::OpenXcb(shared_from_this(), displayName);
        if (fDisplays[id] == nullptr)
            throw VanillaException(__func__, "Couldn't connect to display server");
    }
}

Handle<Display> Context::display(int32_t id)
{
    if (!fDisplays.contains(id))
    {
        LOGF(LOG_ERROR, "Display (ID:{}) haven't been opened yet", id)
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
