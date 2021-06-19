#ifndef COCOA_VAX11DISPLAY_H
#define COCOA_VAX11DISPLAY_H

#include <X11/Xlib.h>
#include "Vanilla/Base.h"
#include "Vanilla/VaDisplay.h"
#include "Vanilla/X11/VaX11Atoms.h"
#include "Vanilla/X11/VaX11EventQueue.h"
VANILLA_NS_BEGIN

class VaX11Window;
class VaX11Display : public VaDisplay,
                     public std::enable_shared_from_this<VaX11Display>
{
    friend class VaX11EventQueue;

public:
    explicit VaX11Display(const Handle<Context>& ctx,
                          Display *display,
                          Screen *screen,
                          int32_t screenNum,
                          Visual *visual,
                          SkColorType format);
    ~VaX11Display() noexcept override;

    int32_t width() override;
    int32_t height() override;

    inline Display *display()
    { return fDisplay; }
    inline Screen *screen()
    { return fScreen; }
    inline Visual *visual()
    { return fVisual; }
    VaX11Atoms& atoms()
    { return fAtoms; }

    void flush() override;
    void dispose() override;

private:
    void eventQueueDispatch(const std::vector<UniqueHandle<XEvent>>& events);
    void dispatchEachEvent(const XEvent& event);
    void realDispose();
    Handle<VaWindow> onCreateWindow(VaVec2f size, VaVec2f pos, Handle<VaWindow> parent) override;

    Handle<VaX11Window> matchWindow(Window window);

    Display            *fDisplay;
    Screen             *fScreen;
    int32_t             fScreenNumber;
    Visual             *fVisual;
    SkColorType         fColorFormat;
    VaX11Atoms          fAtoms;
    bool                fDisposed;
    VaX11EventQueue     fEventQueue;
};

VANILLA_NS_END
#endif //COCOA_VAX11DISPLAY_H
