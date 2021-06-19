#ifndef COCOA_VAX11RASTERDRAWCONTEXT_H
#define COCOA_VAX11RASTERDRAWCONTEXT_H

#include <X11/Xlib.h>

#include "include/core/SkBitmap.h"
#include "Vanilla/Base.h"
#include "Vanilla/VaDrawContext.h"
VANILLA_NS_BEGIN

class VaWindow;
class VaX11RasterDrawContext : public VaDrawContext
{
public:
    explicit VaX11RasterDrawContext(Handle<VaWindow> window);
    ~VaX11RasterDrawContext() override;

private:
    sk_sp<SkSurface> onBeginFrame(const SkRect& region) override;
    void onEndFrame(const SkRect& region) override;
    void onResize(int32_t width, int32_t height) override;

    void createImages(int32_t width, int32_t height);

    Display                     *fXDisplay;
    Window                       fXWindow;
    uint8_t                     *fPixels;
    sk_sp<SkSurface>             fSurface;
    uint8_t                      fWindowDepth;
    GC                           fGc;
    XImage                      *fImage;
};

VANILLA_NS_END
#endif //COCOA_VAX11RASTERDRAWCONTEXT_H
