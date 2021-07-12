#ifndef COCOA_VADISPLAY_H
#define COCOA_VADISPLAY_H

#include <list>
#include <functional>

#include "Core/EventSource.h"
#include "Vanilla/Base.h"
#include "Vanilla/VaKeyboardProxy.h"
VANILLA_NS_BEGIN

class Context;
class VaWindow;

enum class DisplayBackend
{
    kDisplay_Xcb
};

class VaDisplay
{
public:
    explicit VaDisplay(DisplayBackend backend, const Handle<Context>& ctx);
    virtual ~VaDisplay() = default;

    static Handle<VaDisplay> OpenXcb(const Handle<Context>& ctx, const char *displayName);

    inline DisplayBackend backend() const
    { return fBackend; }

    virtual VaKeyboardProxy *keyboardProxy() = 0;

    virtual int32_t width() = 0;
    virtual int32_t height() = 0;
    Handle<VaWindow> createWindow(VaVec2f size, VaVec2f pos, Handle<VaWindow> parent = nullptr);
    void forEachWindow(const std::function<bool(Handle<VaWindow>)>& func);

    virtual void flush() = 0;
    virtual void dispose() = 0;

    /**
     * The window will not be owned by this Display anymore.
     * But that doesn't mean that the window is going be closed or destroyed.
     * It will be destroyed when the reference count goes to be zero.
     */
    void detachWindow(Handle<VaWindow> window);

    inline Handle<Context> getContext()
    { return fContext.lock(); }

protected:
    virtual Handle<VaWindow> onCreateWindow(VaVec2f size, VaVec2f pos, Handle<VaWindow> parent) = 0;

private:
    DisplayBackend                  fBackend;
    WeakHandle<Context>             fContext;
    std::list<Handle<VaWindow>>     fWindows;
};

VANILLA_NS_END
#endif //COCOA_VADISPLAY_H
