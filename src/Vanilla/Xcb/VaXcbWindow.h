#ifndef COCOA_VAXCBWINDOW_H
#define COCOA_VAXCBWINDOW_H

#include <xcb/xcb.h>

#include "Vanilla/Base.h"
#include "Vanilla/VaWindow.h"
#include "Vanilla/Xcb/EventHandlerMacros.h"
VANILLA_NS_BEGIN

class VaXcbDisplay;
class VaXcbWindow : public VaWindow
{
public:
    VaXcbWindow(WeakHandle<VaXcbDisplay> display,
                xcb_window_t window,
                int32_t width, int32_t height,
                SkColorType format);
    ~VaXcbWindow() override;

    va_nodiscard inline xcb_window_t window() const
    { return fWindow; }

    void setTitle(const std::string& title) override;
    void setResizable(bool resizable) override;
    void setIconFile(const std::string& file) override;
    void show() override;
    int32_t width() override;
    int32_t height() override;
    void update(const SkRect& rect) override;

    VA_WIN_HANDLER_DECL(expose)
    VA_WIN_HANDLER_DECL(configure_notify)
    VA_WIN_HANDLER_DECL(client_message)
    VA_WIN_HANDLER_DECL(map_notify)
    VA_WIN_HANDLER_DECL(unmap_notify)

private:
    void onClose() override;

    xcb_connection_t    *fConnection;
    xcb_window_t         fWindow;
    int32_t              fWidth;
    int32_t              fHeight;
};

VANILLA_NS_END
#endif //COCOA_VAXCBWINDOW_H
