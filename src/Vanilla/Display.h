#ifndef COCOA_DISPLAY_H
#define COCOA_DISPLAY_H

#include <list>
#include <functional>

#include "Core/EventSource.h"
#include "Vanilla/Base.h"
#include "Vanilla/KeyboardProxy.h"
VANILLA_NS_BEGIN

class Context;
class Window;

enum class DisplayBackend
{
    kDisplay_Xcb,
    kDisplay_Wayland
};

class Display
{
public:
    explicit Display(DisplayBackend backend, const Handle<Context>& ctx);
    virtual ~Display() = default;

    static Handle<Display> OpenXcb(const Handle<Context>& ctx, const char *displayName);

    inline DisplayBackend backend() const
    { return fBackend; }

    virtual KeyboardProxy *keyboardProxy() = 0;

    virtual int32_t width() = 0;
    virtual int32_t height() = 0;
    Handle<Window> createWindow(vec::float2 size, vec::float2 pos, Handle<Window> parent = nullptr);
    void forEachWindow(const std::function<bool(Handle<Window>)>& func);

    virtual void flush() = 0;
    virtual void dispose() = 0;

    /**
     * The window will not be owned by this Display anymore.
     * But that doesn't mean that the window is going be closed or destroyed.
     * It will be destroyed when the reference count goes to be zero.
     */
    void detachWindow(Handle<Window> window);

    inline Handle<Context> getContext()
    { return fContext.lock(); }

protected:
    virtual Handle<Window> onCreateWindow(vec::float2 size, vec::float2 pos, Handle<Window> parent) = 0;

private:
    DisplayBackend                  fBackend;
    WeakHandle<Context>             fContext;
    std::list<Handle<Window>>     fWindows;
};

VANILLA_NS_END
#endif //COCOA_DISPLAY_H
