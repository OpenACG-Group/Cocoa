#ifndef COCOA_CONTEXT_H
#define COCOA_CONTEXT_H

#include <map>

#include <sigc++/sigc++.h>
#include "Core/EventLoop.h"
#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

class VaDisplay;

/**
 * A Vanilla context is the highest-level container in Vanilla
 * which holds all of the necessary object instances of your platform.
 *
 * Display is an abstraction of a connection to display server
 * (for an example, X11 server on Unix systems). It allows you create
 * windows and acquire some information about display server. Display
 * also takes responsibility for receiving and handling events that
 * is sent by display server.
 *
 * Context will connect to the DBus daemon as soon as it's constructed
 * if DBus daemon is active. Then it requests a DBus service name
 * "org.OpenACG.Cocoa".
 */
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
    ~Context() = default;

    va_nodiscard Backend backend() const
    { return fBackend; }

    static Handle<Context> Make(EventLoop *loop, Backend backend);

    va_nodiscard inline EventLoop *eventLoop()
    { return fEventLoop; }
    va_nodiscard Handle<VaDisplay> display(int32_t id);

    void connectTo(char const *displayName, int32_t id);

private:
    EventLoop                       *fEventLoop;
    Backend                          fBackend;
    std::map<int32_t, Handle<VaDisplay>>
                                     fDisplays;
};

VANILLA_NS_END
#endif //COCOA_CONTEXT_H
