#ifndef COCOA_XCBRASTERDRAWCONTEXT_H
#define COCOA_XCBRASTERDRAWCONTEXT_H

#include <xcb/xcb.h>

#include "Vanilla/Base.h"
#include "Vanilla/DrawContext.h"
VANILLA_NS_BEGIN

class XcbWindow;
class XcbRasterDrawContext : public DrawContext
{
public:
    explicit XcbRasterDrawContext(Handle<Window> window);
    ~XcbRasterDrawContext() override;

private:
    void createSurface(int32_t width, int32_t height);

    sk_sp<SkSurface> createBackendSurface(const SkImageInfo &info, SkBudgeted budgeted) override;
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
#endif //COCOA_XCBRASTERDRAWCONTEXT_H
