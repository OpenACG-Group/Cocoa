#ifndef COCOA_XCBWINDOW_H
#define COCOA_XCBWINDOW_H

#include <xcb/xcb.h>
#include <xcb/xinput.h>

#include "Vanilla/Base.h"
#include "Vanilla/Window.h"
#include "Vanilla/Xcb/EventHandlerMacros.h"
VANILLA_NS_BEGIN

class XcbDisplay;
class XcbWindow : public Window
{
public:
    XcbWindow(WeakHandle<XcbDisplay> display,
                xcb_window_t window,
                int32_t width, int32_t height,
                SkColorType format);
    ~XcbWindow() override;

    va_nodiscard inline xcb_window_t window() const
    { return fWindow; }

    void setTitle(const std::string& title) override;
    void setResizable(bool resizable) override;
    void setIcon(std::istream &stream, std::streamsize size) override;
    void show() override;
    int32_t width() override;
    int32_t height() override;
    void update(const SkRect& rect) override;

    VA_WIN_HANDLER_DECL(expose)
    VA_WIN_HANDLER_DECL(configure_notify)
    VA_WIN_HANDLER_DECL(client_message)
    VA_WIN_HANDLER_DECL(map_notify)
    VA_WIN_HANDLER_DECL(unmap_notify)
    VA_WIN_HANDLER_DECL(input_button_press)
    VA_WIN_HANDLER_DECL(input_button_release)
    VA_WIN_HANDLER_DECL(input_motion)
    VA_WIN_HANDLER_DECL(input_touch_begin)
    VA_WIN_HANDLER_DECL(input_touch_end)
    VA_WIN_HANDLER_DECL(input_touch_update)
    VA_WIN_HANDLER_DECL(key_press)
    VA_WIN_HANDLER_DECL(key_release)

private:
    void onClose() override;

    xcb_connection_t    *fConnection;
    xcb_window_t         fWindow;
    int32_t              fWidth;
    int32_t              fHeight;
};

VANILLA_NS_END
#endif //COCOA_XCBWINDOW_H
