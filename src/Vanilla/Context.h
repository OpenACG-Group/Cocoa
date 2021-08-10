#ifndef COCOA_CONTEXT_H
#define COCOA_CONTEXT_H

#include <map>

#include <sigc++/sigc++.h>
#include "Core/EventLoop.h"
#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

class Display;
class Context : public std::enable_shared_from_this<Context>
{
public:
    enum class Backend
    {
        kXcb
    };

    enum ResourceId
    {
        kDefault = 0
    };

    Context(EventLoop *loop, Backend backend);
    ~Context();

    va_nodiscard Backend backend() const
    { return fBackend; }

    static Handle<Context> Make(EventLoop *loop, Backend backend);

    va_nodiscard inline EventLoop *eventLoop()
    { return fEventLoop; }
    va_nodiscard Handle<Display> display(int32_t id);
    va_nodiscard inline bool hasDisplay(int32_t id)
    { return fDisplays.contains(id); }

    void connectTo(char const *displayName, int32_t id);
    void detachDisplay(int32_t id);
    bool allDisplaysAreUnique();

private:
    EventLoop                       *fEventLoop;
    Backend                          fBackend;
    std::map<int32_t, Handle<Display>>
                                     fDisplays;
};

VANILLA_NS_END
#endif //COCOA_CONTEXT_H
