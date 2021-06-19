#include <iostream>
#include <cassert>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/XInput2.h>

#include "Core/Journal.h"
#include "Vanilla/X11/VaX11Display.h"
#include "Vanilla/X11/VaX11Window.h"
VANILLA_NS_BEGIN

Handle<VaDisplay> VaDisplay::OpenX11(const Handle<Context>& ctx, const char *dispName)
{
    Display *display = XOpenDisplay(dispName);
    if (display == nullptr)
    {
        Journal::Ref()(LOG_ERROR, "Failed to open X11 display");
        return nullptr;
    }
    BeforeLeaveScope leave([display]() -> void {
        XCloseDisplay(display);
    });

    int screenNumber = XDefaultScreen(display);
    Screen *screen = XScreenOfDisplay(display, screenNumber);
    if (screen == nullptr)
        return nullptr;
    Visual *visual = XDefaultVisualOfScreen(screen);
    if (visual == nullptr)
        return nullptr;

    {
        int event, error;
        if (!XRenderQueryExtension(display, &event, &error))
        {
            Journal::Ref()(LOG_ERROR, "XRender extension is unavailable");
            return nullptr;
        }
    }

    SkColorType format;
    {
        XRenderPictFormat *format1 = XRenderFindStandardFormat(display, PictStandardRGB24);
        XRenderPictFormat *format2 = XRenderFindStandardFormat(display, PictStandardARGB32);
        XRenderPictFormat *visualFormat = XRenderFindVisualFormat(display, visual);
        if (format1->id == visualFormat->id ||
            format2->id == visualFormat->id)
            format = SkColorType::kBGRA_8888_SkColorType;
        else
        {
            Journal::Ref()(LOG_ERROR, "Unsupported picture format (ARGB32 or RGB24 is required)");
            return nullptr;
        }
    }

    leave.cancel();
    return std::make_shared<VaX11Display>(ctx, display, screen,
                                          screenNumber, visual, format);
}

VaX11Display::VaX11Display(const Handle<Context>& ctx,
                           Display *display,
                           Screen *screen,
                           int32_t screenNum,
                           Visual *visual,
                           SkColorType format)
    : VaDisplay(DisplayBackend::kDisplay_X11, ctx),
      fDisplay(display),
      fScreen(screen),
      fScreenNumber(screenNum),
      fVisual(visual),
      fColorFormat(format),
      fAtoms(display),
      fDisposed(false),
      fEventQueue(this)
{
}

VaX11Display::~VaX11Display() noexcept
{
    if (!fDisposed)
        this->realDispose();
}

void VaX11Display::dispose()
{
    this->realDispose();
}

void VaX11Display::realDispose()
{
    fDisposed = true;
    fEventQueue.disposeInMainThread();
    if (fDisplay)
        XCloseDisplay(fDisplay);
}

Handle<VaX11Window> VaX11Display::matchWindow(Window window)
{
    Handle<VaWindow> result = nullptr;
    VaDisplay::forEachWindow([window, &result](Handle<VaWindow> pWindow) -> bool {
        if (std::dynamic_pointer_cast<VaX11Window>(pWindow)->nativeWindowHandle() == window)
        {
            result = pWindow;
            return false;
        }
        return true;
    });
    return std::dynamic_pointer_cast<VaX11Window>(result);
}

void VaX11Display::flush()
{
    XFlush(fDisplay);
}

Handle<VaWindow> VaX11Display::onCreateWindow(VaVec2f size, VaVec2f pos, Handle<VaWindow> parent)
{
    Window pureParent = XRootWindowOfScreen(fScreen);
    if (parent != nullptr)
    {
        assert(parent->getDisplay()->backend() == DisplayBackend::kDisplay_X11);
        pureParent = std::dynamic_pointer_cast<VaX11Window>(parent)->nativeWindowHandle();
    }

    auto x = static_cast<int32_t>(pos.x()),
         y = static_cast<int32_t>(pos.y()),
         w = static_cast<int32_t>(size.x()),
         h = static_cast<int32_t>(size.y());
    XSetWindowAttributes attributes{
        .background_pixel = XBlackPixelOfScreen(fScreen)
    };

    Window window = XCreateWindow(fDisplay,
                                  pureParent,
                                  x, y, w, h,
                                  0,
                                  XDefaultDepthOfScreen(fScreen),
                                  InputOutput,
                                  fVisual,
                                  CWBackPixel,
                                  &attributes);
    assert(window != 0);
    XSetWMProtocols(fDisplay, window, &fAtoms.get(VaX11Atoms::WM_DELETE_WINDOW), 1);
    XSelectInput(fDisplay, window, ExposureMask | StructureNotifyMask);
    return std::make_shared<VaX11Window>(shared_from_this(), window, w, h, fColorFormat);
}

void VaX11Display::eventQueueDispatch(const std::vector<UniqueHandle<XEvent>>& events)
{
    for (const auto& ev : events)
    {
        dispatchEachEvent(*ev);
        if (fDisposed)
            break;
    }
}

void VaX11Display::dispatchEachEvent(const XEvent& event)
{
    switch (event.type)
    {
    case ClientMessage:
        if (event.xclient.data.l[0] == fAtoms.get(VaX11Atoms::WM_DELETE_WINDOW))
        {
            Handle<VaX11Window> window = matchWindow(event.xclient.window);
            if (!window)
                break;
            window->dispatchCloseEvent();
        }
        break;

    case ConfigureNotify:
        {
            Handle<VaX11Window> window = matchWindow(event.xconfigure.window);
            if (!window)
                break;
            window->dispatchConfigureEvent(event.xconfigure);
        }
        break;

    case MapNotify:
        {
            Handle<VaX11Window> window = matchWindow(event.xmap.window);
            if (!window)
                break;
            window->dispatchMapEvent();
        }
        break;

    case UnmapNotify:
        {
            Handle<VaX11Window> window = matchWindow(event.xunmap.window);
            if (!window)
                break;
            window->dispatchUnmapEvent();
        }
        break;

    case Expose:
        {
            Handle<VaX11Window> window = matchWindow(event.xexpose.window);
            if (!window)
                break;
            window->dispatchExposure(event.xexpose);
        }
        break;

    default:
        Journal::Ref()(LOG_WARNING, "Unknown event {} type {}", fmt::ptr(&event), event.type);
        break;
    }
}

int32_t VaX11Display::width()
{
    return XWidthOfScreen(fScreen);
}

int32_t VaX11Display::height()
{
    return XHeightOfScreen(fScreen);
}

VANILLA_NS_END
