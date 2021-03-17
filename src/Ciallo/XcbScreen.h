#ifndef COCOA_XCBSCREEN_H
#define COCOA_XCBSCREEN_H

#include <map>
#include <memory>

#include <xcb/xcb.h>
#include "include/core/SkBitmap.h"

#include "Ciallo/Ciallo.h"
CIALLO_BEGIN_NS
class XcbConnection;

class XcbScreen
{
public:
    XcbScreen(XcbConnection *connection, xcb_screen_t *screen);

    /**
     * @brief Get the image of window.
     * @param window An XCB window handle, passing XCB_NONE for root window.
     * @return A pointer to SkBitmap object, maybe nullptr.
     */
    std::unique_ptr<SkBitmap> grabWindow(xcb_window_t window, const SkIRect& rect);

    inline XcbConnection *connection() const
    { return fConnection; }

    xcb_visualtype_t *visualType(xcb_visualid_t);
    
    inline xcb_window_t root() const
    { return fScreen->root; }

    inline xcb_visualid_t visualId() const
    { return fScreen->root_visual; }

    inline xcb_visualtype_t& rootVisual()
    { return fVisuals[fScreen->root_visual]; }

    inline uint8_t rootDepth()
    { return fDepths[fScreen->root_visual]; }

    inline xcb_screen_t *nativeHandle() const
    { return fScreen; }

    ImageFormat imageFormat(xcb_visualid_t visualId, uint8_t depth);
    ImageFormat imageFormat(const xcb_visualtype_t *visualType, uint8_t depth);

    inline int refreshRate() const
    { return fRefreshRate; }

    SkIPoint translate(xcb_window_t src, xcb_window_t dst, const SkIPoint& pointInChild);

private:
    XcbConnection       *fConnection;
    xcb_screen_t        *fScreen;
    int                  fRefreshRate;
    SkIRect              fGeometry;
    std::map<xcb_visualid_t, xcb_visualtype_t>  fVisuals;
    std::map<xcb_visualid_t, uint8_t>           fDepths;
};

CIALLO_END_NS
#endif // COCOA_XCBSCREEN_H
