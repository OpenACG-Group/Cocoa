#ifndef COCOA_GLAMOR_WAYLANDHWCOMPOSERENDERTARGET_H
#define COCOA_GLAMOR_WAYLANDHWCOMPOSERENDERTARGET_H

#include <wayland-client.h>

#include "Glamor/Glamor.h"
#include "Glamor/Wayland/WaylandRenderTarget.h"
GLAMOR_NAMESPACE_BEGIN

class WaylandDisplay;
class HWComposeSwapchain;

class WaylandHWComposeRenderTarget : public WaylandRenderTarget
{
public:
    static Shared<WaylandHWComposeRenderTarget> Make(const Shared<WaylandDisplay>& display,
                                                     int32_t width, int32_t height);

    WaylandHWComposeRenderTarget(Shared<HWComposeContext> hwContext,
                                 const Shared<WaylandDisplay>& display,
                                 int32_t width, int32_t height, SkColorType format);
    ~WaylandHWComposeRenderTarget() override;

    void OnClearFrameBuffers() override;
    SkSurface *OnBeginFrame() override;
    void OnSubmitFrame(SkSurface *surface, const SkRegion& damage) override;
    void OnResize(int32_t width, int32_t height) override;
    const Shared<HWComposeSwapchain>& OnGetHWComposeSwapchain() override;
    sk_sp<SkSurface> OnCreateOffscreenBackendSurface(const SkImageInfo& info) override;

private:
    Shared<HWComposeContext>         hw_compose_context_;
    Shared<HWComposeSwapchain>       swapchain_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLANDHWCOMPOSERENDERTARGET_H
