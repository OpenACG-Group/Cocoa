#include "Cobalt/Wayland/WaylandRenderTarget.h"
#include "Cobalt/Wayland/WaylandDisplay.h"
COBALT_NAMESPACE_BEGIN

WaylandRenderTarget::WaylandRenderTarget(const co_sp<Display>& display, RenderDevice device, int32_t width,
                                         int32_t height, SkColorType format)
    : RenderTarget(display, device, width, height ,format)
    , wl_surface_(nullptr)
    , wl_event_queue_(nullptr)
{
}

void WaylandRenderTarget::SetOpaque()
{
    auto& g = GetDisplay()->Cast<WaylandDisplay>()->GetGlobalsRef();
    wl_region *region = wl_compositor_create_region(g->wl_compositor_);
    wl_region_add(region, 0, 0, GetWidth(), GetHeight());
    wl_surface_set_opaque_region(wl_surface_, region);
    wl_surface_commit(wl_surface_);
    wl_region_destroy(region);
}

void WaylandRenderTarget::OnClearFrameBuffers()
{
}

COBALT_NAMESPACE_END
