#ifndef COCOA_VADISPLAY_H
#define COCOA_VADISPLAY_H

#include <list>
#include <functional>

#include "Core/EventSource.h"
#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

class Context;
class VaWindow;

enum class DisplayBackend
{
    kDisplay_X11
};

class VaDisplay
{
public:
    explicit VaDisplay(DisplayBackend backend);
    virtual ~VaDisplay() = default;

    static Handle<VaDisplay> OpenX11(EventLoop *loop, const char *dispName);

    inline DisplayBackend backend() const
    { return fBackend; }

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

    inline void attachContext(const Handle<Context>& context)
    { fContext = context; }
    inline Handle<Context> getContext()
    { return fContext.lock(); }

protected:
    virtual Handle<VaWindow> onCreateWindow(VaVec2f size, VaVec2f pos, Handle<VaWindow> parent) = 0;

private:
    WeakHandle<Context>             fContext;
    DisplayBackend                  fBackend;
    std::list<Handle<VaWindow>>     fWindows;
};

VANILLA_NS_END
#endif //COCOA_VADISPLAY_H
