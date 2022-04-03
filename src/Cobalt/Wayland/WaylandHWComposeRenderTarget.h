#ifndef COCOA_COBALT_WAYLANDHWCOMPOSERENDERTARGET_H
#define COCOA_COBALT_WAYLANDHWCOMPOSERENDERTARGET_H

#include <wayland-client.h>

#include "Cobalt/Cobalt.h"
#include "Cobalt/Wayland/WaylandRenderTarget.h"
COBALT_NAMESPACE_BEGIN

class WaylandDisplay;
class HWComposeSwapchain;

class WaylandHWComposeRenderTarget : public WaylandRenderTarget
{
public:
    static co_sp<WaylandHWComposeRenderTarget> Make(const co_sp<WaylandDisplay>& display,
                                                    int32_t width, int32_t height);

    WaylandHWComposeRenderTarget(co_sp<HWComposeContext> hwContext,
                                 const co_sp<WaylandDisplay>& display,
                                 int32_t width, int32_t height, SkColorType format);
    ~WaylandHWComposeRenderTarget() override;

    void OnClearFrameBuffers() override;
    SkSurface *OnBeginFrame() override;
    void OnSubmitFrame(SkSurface *surface, const SkRegion& damage) override;
    void OnResize(int32_t width, int32_t height) override;

private:
    co_sp<HWComposeContext>         hw_compose_context_;
    co_sp<HWComposeSwapchain>       swapchain_;
};

COBALT_NAMESPACE_END
#endif //COCOA_COBALT_WAYLANDHWCOMPOSERENDERTARGET_H
