#include <iostream>
#include <cassert>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/XInput2.h>

#include "Core/Journal.h"
#include "Vanilla/X11/VaX11Display.h"
#include "Vanilla/X11/VaX11Window.h"
VANILLA_NS_BEGIN

Handle<VaDisplay> VaDisplay::OpenX11(EventLoop *loop, const char *dispName)
{
    Display *display = XOpenDisplay(dispName);
    if (display == nullptr)
    {
        log_write(LOG_ERROR) << "Failed to open X11 display" << log_endl;
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
            log_write(LOG_ERROR) << "XRender extension is unavailable" << log_endl;
            return nullptr;
        }
    }

    VaColorFormat format;
    {
        XRenderPictFormat *format1 = XRenderFindStandardFormat(display, PictStandardRGB24);
        XRenderPictFormat *format2 = XRenderFindStandardFormat(display, PictStandardARGB32);
        XRenderPictFormat *visualFormat = XRenderFindVisualFormat(display, visual);
        if (format1->id == visualFormat->id)
            format = VaColorFormat::kRGB_8888;
        else if (format2->id == visualFormat->id)
            format = VaColorFormat::kARGB_8888;
        else
        {
            log_write(LOG_ERROR) << "Unsupported picture format (ARGB32 or RGB24 is required)" << log_endl;
            return nullptr;
        }
    }

    leave.cancel();
    return std::make_shared<VaX11Display>(loop, display, screen,
                                          screenNumber, visual, format);
}

VaX11Display::VaX11Display(EventLoop *loop,
                           Display *display,
                           Screen *screen,
                           int32_t screenNum,
                           Visual *visual,
                           VaColorFormat format)
    : VaDisplay(DisplayBackend::kDisplay_X11),
      PollSource(loop, XConnectionNumber(display)),
      fDisplay(display),
      fScreen(screen),
      fScreenNumber(screenNum),
      fVisual(visual),
      fColorFormat(format),
      fAtoms(display),
      fDisposed(false)
{
    PollSource::startPoll(UV_READABLE | UV_DISCONNECT);
}

VaX11Display::~VaX11Display() noexcept
{
    PollSource::stopPoll();
    if (fDisplay)
        XCloseDisplay(fDisplay);
}

void VaX11Display::dispose()
{
    fDisposed = true;
}

Handle<VaX11Window> VaX11Display::matchWindow(Window window)
{
    Handle<VaWindow> result = nullptr;
    VaDisplay::forEachWindow([window, &result](Handle<VaWindow> pWindow) -> bool {
        if (std::dynamic_pointer_cast<VaX11Window>(pWindow)->window() == window)
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
        pureParent = std::dynamic_pointer_cast<VaX11Window>(parent)->window();
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

KeepInLoop VaX11Display::dispatch(int status, int events)
{
    if (status < 0)
    {
        log_write(LOG_ERROR) << "Failed to dispatch X11 events: "
                             << uv_strerror(status) << log_endl;
        return KeepInLoop::kNo;
    }
    if (events == UV_DISCONNECT)
    {
        log_write(LOG_WARNING) << "Lost X11 connection" << log_endl;
        return KeepInLoop::kNo;
    }

    XEvent event;
    XNextEvent(fDisplay, &event);
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
        log_write(LOG_WARNING) << "Unknown X11 event " << &event
                               << " type " << event.type << log_endl;
        break;
    }

    if (fDisposed)
        return KeepInLoop::kNo;
    return KeepInLoop::kYes;
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
