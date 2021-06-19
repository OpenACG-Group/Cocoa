#ifndef COCOA_VAXCBRASTERDRAWCONTEXT_H
#define COCOA_VAXCBRASTERDRAWCONTEXT_H

#include <xcb/xcb.h>

#include "Vanilla/Base.h"
#include "Vanilla/VaDrawContext.h"
VANILLA_NS_BEGIN

class VaXcbWindow;
class VaXcbRasterDrawContext : public VaDrawContext
{
public:
    explicit VaXcbRasterDrawContext(Handle<VaWindow> window);
    ~VaXcbRasterDrawContext() override;

private:
    void createSurface(int32_t width, int32_t height);

    sk_sp<SkSurface> onBeginFrame(const SkRect& region) override;
    void onEndFrame(const SkRect& region) override;
    void onResize(int32_t width, int32_t height) override;

    xcb_connection_t       *fConnection;
    xcb_window_t            fWindow;
    xcb_gcontext_t          fXcbGc;
    uint8_t                *fPixelMemory;
    sk_sp<SkSurface>        fSurface;
    uint8_t                 fDepth;
};

VANILLA_NS_END
#endif //COCOA_VAXCBRASTERDRAWCONTEXT_H
