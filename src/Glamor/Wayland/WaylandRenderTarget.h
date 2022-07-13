#ifndef COCOA_GLAMOR_WAYLAND_WAYLANDRENDERTARGET_H
#define COCOA_GLAMOR_WAYLAND_WAYLANDRENDERTARGET_H

#include <wayland-client-protocol.h>

#include "Glamor/Glamor.h"
#include "Glamor/RenderTarget.h"
GLAMOR_NAMESPACE_BEGIN

class WaylandRenderTarget : public RenderTarget
{
public:
    WaylandRenderTarget(const Shared<Display>& display, RenderDevice device,
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
    uint32_t OnRequestNextFrame() override;

protected:
    wl_surface          *wl_surface_;
    wl_event_queue      *wl_event_queue_;
    uint32_t             request_next_frame_sequence_counter_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_WAYLANDRENDERTARGET_H
