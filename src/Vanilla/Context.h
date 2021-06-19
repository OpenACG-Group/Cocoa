#ifndef COCOA_CONTEXT_H
#define COCOA_CONTEXT_H

#include <map>

#include <sigc++/sigc++.h>
#include "Core/EventLoop.h"
#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

class VaDisplay;
class Context : public std::enable_shared_from_this<Context>
{
public:
    enum class Backend
    {
        kBackend_X11
    };

    enum DisplayId
    {
        kDisplay_Default = 0
    };

    Context(EventLoop *loop, Backend backend);
    ~Context() = default;

    static Handle<Context> Make(EventLoop *loop, Backend backend);

    va_nodiscard inline EventLoop *eventLoop()
    { return fEventLoop; }
    va_nodiscard Handle<VaDisplay> display(int32_t id);

    void open(char const *displayName, int32_t id);

private:
    EventLoop                       *fEventLoop;
    Backend                          fBackend;
    std::map<int32_t, Handle<VaDisplay>>
                                     fDisplays;
};

VANILLA_NS_END
#endif //COCOA_CONTEXT_H
