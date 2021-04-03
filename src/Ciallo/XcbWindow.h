#ifndef COCOA_XCBWINDOW_H
#define COCOA_XCBWINDOW_H

#define VK_USE_PLATFORM_XCB_KHR 1
#include <string>
#include <functional>
#include <utility>

#include "Ciallo/Ciallo.h"
#include "Ciallo/Drawable.h"
#include "Ciallo/XcbConnection.h"
#include "Ciallo/XcbScreen.h"

CIALLO_BEGIN_NS

class XcbWindow : public XcbWindowEventListener,
                  public Drawable
{
public:
    /**
     * @brief Construct an XcbWindow and map it to screen.
     *
     * @param screen    The screen that the window belongs to.
     * @param width     Width of window.
     * @param height    Height of window.
     * @param parent    Parent window, nullptr for the root window of current screen.
     */
    XcbWindow(XcbScreen *screen, const SkIRect& geometry, XcbWindow *parent = nullptr);
    ~XcbWindow();

    inline SkIRect geometry() const override
    { return fGeometry;}

    ImageFormat format() const override;

    inline uint8_t depth() const
    { return fDepth; }

    inline xcb_visualid_t visualId() const
    { return fVisualId; }

    XcbScreen *screen();

    /**
     * @brief Create a Cairo surface (XCB native rendering).
     *        This should be used in Cairo2d.
     */
    cairo_surface_t *createCairoSurface() override;
    void resizeCairoSurface(cairo_surface_t *surface, int32_t w, int32_t h) override;

    /**
     * @brief Create a vulkan surface for window that can be
     *        rendered directly by GPU. This should be used in
     *        Skia2d (GPU composition mode).
     *
     * @param instance Vulkan instance.
     * @return Vulkan surface.
     */
    VkSurfaceKHR createVkSurface(VkInstance instance) override;

    /**
     * @brief Write pixels onto the window directly.
     *        This should be used in Skia2d
     *        (CPU/OpenCL composition mode).
     *
     * @param pixmap Pixmap that contains the pixels.
     * @param rect The rectangle region that should be updated.
     */
    void writePixmap(const SkPixmap& pixmap, const SkIRect& rect) override;

    xcb_window_t nativeHandle() const;

    void setTitle(const std::string& title) override;
    void setResizable(bool value) override;

    void repaint() override;

    void close() override;
    bool isClosed() const override;

private:
    void closeWindow();
    void createWindow(XcbWindow *parent);
    void configureWindow();
    void selectXInputEvents();

    void handleExposeEvent(const xcb_expose_event_t *ev) override;
    void handleClientMessageEvent(const xcb_client_message_event_t *ev) override;
    void handleXInputButtonPress(const xcb_ge_event_t *ev) override;
    void handleXInputButtonRelease(const xcb_ge_event_t *ev) override;
    void handleXInputMotion(const xcb_ge_event_t *ev) override;
    void handleXInputEnter(const xcb_ge_event_t *ev) override;
    void handleXInputLeave(const xcb_ge_event_t *ev) override;
    void handleFocusInEvent(const xcb_focus_in_event_t *ev) override;
    void handleFocusOutEvent(const xcb_focus_out_event_t *ev) override;
    void handleConfigureNotifyEvent(const xcb_configure_notify_event_t *ev) override;

private:
    XcbScreen           *fScreen;
    xcb_connection_t    *fConnection;
    xcb_window_t         fWindow;
    uint8_t              fDepth;
    SkIRect              fGeometry;
    xcb_visualid_t       fVisualId;
    xcb_gcontext_t       fCopyPixmapGContext;
    ImageFormat          fFormat;
};

CIALLO_END_NS
#endif //COCOA_XCBWINDOW_H
