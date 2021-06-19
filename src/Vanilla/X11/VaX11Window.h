#ifndef COCOA_VAX11WINDOW_H
#define COCOA_VAX11WINDOW_H

#include <optional>
#include <X11/Xlib.h>

#include "Vanilla/Base.h"
#include "Vanilla/VaWindow.h"
VANILLA_NS_BEGIN

class VaX11Display;
class VaX11Window : public VaWindow
{
public:
    VaX11Window(WeakHandle<VaX11Display> display, Window window,
                int32_t width, int32_t height, SkColorType format);
    ~VaX11Window() override;

    void setTitle(const std::string& title) override;
    void setResizable(bool resizable) override;
    void setIconFile(const std::string& file) override;
    void show() override;
    int32_t width() override;
    int32_t height() override;
    void update(const SkRect& rect) override;

    va_nodiscard inline Window nativeWindowHandle() const
    { return fWindow; }

    void dispatchCloseEvent();
    void dispatchMapEvent();
    void dispatchUnmapEvent();
    void dispatchConfigureEvent(const XConfigureEvent& ev);
    void dispatchExposure(const XExposeEvent& expose);

private:
    void onClose() override;

    Display                *fXDisplay;
    Window                  fWindow;
    int32_t                 fWidth;
    int32_t                 fHeight;
    bool                    fMapped;
};

VANILLA_NS_END
#endif //COCOA_VAX11WINDOW_H
