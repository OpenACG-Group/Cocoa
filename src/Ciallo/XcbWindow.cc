#include <string>

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_util.h>
#include <cairo-xcb.h>

#include "Core/Journal.h"
#include "Core/Exception.h"
#include "Ciallo/XcbWindow.h"

CIALLO_BEGIN_NS

XcbWindow::XcbWindow(XcbScreen *screen, const SkIRect& geometry, XcbWindow *parent)
    : Drawable(Backend::kXcb),
      fScreen(screen),
      fConnection(screen->connection()->nativeHandle()),
      fWindow(XCB_NONE),
      fDepth(0),
      fGeometry(geometry),
      fVisualId(XCB_NONE),
      fCopyPixmapGContext(XCB_NONE)
{
    BeforeLeaveScope beforeLeaveScope([this]() -> void {
        if (fWindow != XCB_NONE)
            xcb_destroy_window(fConnection, fWindow);
    });

    createWindow(parent);
    configureWindow();
    xcb_map_window(fConnection, fWindow);
    xcb_flush(fConnection);

    fScreen->connection()->addWindowEventListener(fWindow, this);
    fFormat = fScreen->imageFormat(fVisualId, fDepth);
    beforeLeaveScope.cancel();
}

XcbWindow::~XcbWindow()
{
    closeWindow();
    xcb_flush(fConnection);
}

XcbScreen *XcbWindow::screen()
{
    return fScreen;
}

cairo_surface_t *XcbWindow::createCairoSurface()
{
    cairo_surface_t *pSurface = cairo_xcb_surface_create(fConnection,
                                                         fWindow,
                                                         fScreen->visualType(fVisualId),
                                                         fGeometry.width(),
                                                         fGeometry.height());

    if (pSurface == nullptr)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Failed to create Cairo2d surface")
                .make<RuntimeException>();
    }
    return pSurface;
}

VkSurfaceKHR XcbWindow::createVkSurface(VkInstance instance)
{
    VkSurfaceKHR surface;
    VkXcbSurfaceCreateInfoKHR createInfo{
        .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .connection = fConnection,
        .window = fWindow
    };

    VkResult result = vkCreateXcbSurfaceKHR(instance, &createInfo, nullptr, &surface);
    if (result != VK_SUCCESS)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Failed to create Vulkan surface by XCB")
                .make<RuntimeException>();
    }

    return surface;
}

void XcbWindow::writePixmap(const SkPixmap& pixmap, const SkIRect& rect)
{
    if (fCopyPixmapGContext == XCB_NONE)
    {
        fCopyPixmapGContext = xcb_generate_id(fConnection);
        xcb_create_gc(fConnection, fCopyPixmapGContext, fWindow, 0, nullptr);
    }

    xcb_put_image(fConnection,
                  XCB_IMAGE_FORMAT_Z_PIXMAP,
                  fWindow,
                  fCopyPixmapGContext,
                  rect.width(),
                  rect.height(),
                  rect.x(),
                  rect.y(),
                  0,
                  24,
                  pixmap.computeByteSize(),
                  reinterpret_cast<const uint8_t*>(pixmap.addr(rect.x(), rect.y())));
    xcb_flush(fConnection);
}

xcb_window_t XcbWindow::nativeHandle() const
{
    return fWindow;
}

void XcbWindow::createWindow(XcbWindow *parent)
{
    xcb_window_t parentWin;
    if (parent == nullptr)
    {
        parentWin = fScreen->root();
        fDepth = fScreen->rootDepth();
        fVisualId = fScreen->visualId();
    }
    else
    {
        parentWin = parent->nativeHandle();
        fDepth = parent->depth();
        fVisualId = parent->visualId();
    }

    constexpr uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    const uint32_t maskValues[2] = {
            fScreen->nativeHandle()->white_pixel,
            XCB_EVENT_MASK_EXPOSURE |
            XCB_EVENT_MASK_POINTER_MOTION |
            XCB_EVENT_MASK_BUTTON_PRESS |
            XCB_EVENT_MASK_BUTTON_RELEASE |
            XCB_EVENT_MASK_BUTTON_MOTION |
            XCB_EVENT_MASK_KEY_PRESS |
            XCB_EVENT_MASK_KEY_RELEASE |
            XCB_EVENT_MASK_ENTER_WINDOW |
            XCB_EVENT_MASK_LEAVE_WINDOW
    };

    fWindow = xcb_generate_id(fConnection);
    xcb_create_window(fConnection,
                      fDepth,
                      fWindow,
                      parentWin,
                      fGeometry.x(),
                      fGeometry.y(),
                      fGeometry.width(),
                      fGeometry.height(),
                      0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      fVisualId,
                      mask,
                      maskValues);
}

void XcbWindow::configureWindow()
{
    auto wmProtocolAtom = fScreen->connection()->atom(XcbAtom::Atom::WM_PROTOCOLS);
    auto deleteWindowAtom = fScreen->connection()->atom(XcbAtom::Atom::WM_DELETE_WINDOW);

    xcb_change_property(fConnection,
                        XCB_PROP_MODE_REPLACE,
                        fWindow,
                        wmProtocolAtom,
                        XCB_ATOM_ATOM,
                        32, 1,
                        &deleteWindowAtom);
}

void XcbWindow::setTitle(const std::string& title)
{
    xcb_change_property(fConnection,
                        XCB_PROP_MODE_REPLACE,
                        fWindow,
                        XCB_ATOM_WM_NAME,
                        XCB_ATOM_STRING,
                        8,
                        title.length(),
                        title.c_str());
    xcb_flush(fConnection);
}

void XcbWindow::setResizable(bool value)
{
    xcb_size_hints_t hints{
        .flags = XCB_ICCCM_SIZE_HINT_P_MIN_SIZE | XCB_ICCCM_SIZE_HINT_P_MAX_SIZE,
        .min_width = fGeometry.width(),
        .min_height = fGeometry.height(),
        .max_width = fGeometry.width(),
        .max_height = fGeometry.height()
    };

    if (value)
    {
        hints.min_width = 0;
        hints.min_height = 0;
        hints.max_width = 0xffff;
        hints.max_height = 0xffff;
    }

    xcb_change_property(fConnection,
                        XCB_PROP_MODE_REPLACE,
                        fWindow,
                        XCB_ATOM_WM_NORMAL_HINTS,
                        XCB_ATOM_WM_SIZE_HINTS,
                        32,
                        sizeof(hints) >> 2,
                        &hints);
    xcb_flush(fConnection);
}

ImageFormat XcbWindow::format() const
{
    return fFormat;
}

void XcbWindow::handleExposeEvent(const xcb_expose_event_t *ev)
{
    Drawable::listener()->onRender();
}

void XcbWindow::handleClientMessageEvent(const xcb_client_message_event_t *ev)
{
    if (ev->data.data32[0] == fScreen->connection()->atom(XcbAtom::Atom::WM_DELETE_WINDOW))
    {
        bool shouldClose = Drawable::listener()->onClose();
        if (shouldClose)
            closeWindow();
    }
}

void XcbWindow::close()
{
    closeWindow();
}

void XcbWindow::closeWindow()
{
    if (fWindow == XCB_NONE)
        return;

    Drawable::listener()->onDestroy();
    fScreen->connection()->removeWindowEventListener(this);
    xcb_destroy_window(fConnection, fWindow);
    fWindow = XCB_NONE;
}

bool XcbWindow::isClosed() const
{
    return fWindow == XCB_NONE;
}

CIALLO_END_NS
