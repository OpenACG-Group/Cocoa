/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef COCOA_GLAMOR_WAYLAND_WAYLANDRENDERTARGET_H
#define COCOA_GLAMOR_WAYLAND_WAYLANDRENDERTARGET_H

#include <wayland-client-protocol.h>

#include "Glamor/Glamor.h"
#include "Glamor/RenderTarget.h"
GLAMOR_NAMESPACE_BEGIN

class WaylandRenderTarget : public RenderTarget
{
public:
    WaylandRenderTarget(const std::shared_ptr<Display>& display, RenderDevice device,
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
    virtual void OnNotifyImplFrame() {}

    uint32_t OnRequestNextFrame() override;

    void Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept override;

protected:
    wl_surface          *wl_surface_;
    wl_event_queue      *wl_event_queue_;
    uint32_t             request_next_frame_sequence_counter_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_WAYLANDRENDERTARGET_H
