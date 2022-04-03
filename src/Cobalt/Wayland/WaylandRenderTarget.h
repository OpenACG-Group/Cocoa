#ifndef COCOA_COBALT_WAYLAND_WAYLANDRENDERTARGET_H
#define COCOA_COBALT_WAYLAND_WAYLANDRENDERTARGET_H

#include <wayland-client-protocol.h>

#include "Cobalt/Cobalt.h"
#include "Cobalt/RenderTarget.h"
COBALT_NAMESPACE_BEGIN

class WaylandRenderTarget : public RenderTarget
{
public:
    WaylandRenderTarget(const co_sp<Display>& display, RenderDevice device,
                        int32_t width, int32_t height, SkColorType format);
    ~WaylandRenderTarget() override = default;

    g_nodiscard g_inline wl_surface *GetWaylandSurface() {
        return wl_surface_;
    }

    g_nodiscard g_inline wl_event_queue *GetWaylandEventQueue() {
        return wl_event_queue_;
    }

    void SetOpaque();
    virtual void OnClearFrameBuffers();

protected:
    wl_surface          *wl_surface_;
    wl_event_queue      *wl_event_queue_;
};

COBALT_NAMESPACE_END
#endif //COCOA_COBALT_WAYLAND_WAYLANDRENDERTARGET_H
