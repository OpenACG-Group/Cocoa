#ifndef COCOA_CONTEXT_H
#define COCOA_CONTEXT_H

#include <vector>

#include <sigc++/sigc++.h>
#include "Core/EventLoop.h"
#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

class VaDisplay;
class Context
{
public:
    Context(EventLoop *loop,
            Handle<VaDisplay> display);
    ~Context() = default;

    static Handle<Context> MakeX11(EventLoop *loop, const char *displayName);

    inline EventLoop *eventLoop()
    { return fEventLoop; }
    inline Handle<VaDisplay> display()
    { return fDisplay; }

private:
    EventLoop                       *fEventLoop;
    Handle<VaDisplay>                fDisplay;
};

VANILLA_NS_END
#endif //COCOA_CONTEXT_H
