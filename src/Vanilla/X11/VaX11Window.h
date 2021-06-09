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
                int32_t width, int32_t height, VaColorFormat format);
    ~VaX11Window() override;

    inline Window window()
    { return fWindow; }

    void setTitle(const std::string& title) override;
    void setResizable(bool resizable) override;
    void setIconFile(const std::string& file) override;
    void show() override;
    int32_t width() override;
    int32_t height() override;
    void update(const SkRect& rect) override;

    void dispatchCloseEvent();
    void dispatchMapEvent();
    void dispatchUnmapEvent();
    void dispatchConfigureEvent(XConfigureEvent& ev);
    void dispatchExposure(XExposeEvent& expose);

private:
    void onClose() override;

    struct WindowOperationCache
    {
        std::optional<std::string> title;
        std::optional<std::string> iconFile;
        std::optional<bool>        resizable;
    };

    Display                *fXDisplay;
    Window                  fWindow;
    int32_t                 fWidth;
    int32_t                 fHeight;
    bool                    fMapped;
    WindowOperationCache    fOperationCache;
};

VANILLA_NS_END
#endif //COCOA_VAX11WINDOW_H
