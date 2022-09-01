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

#include "Glamor/Wayland/WaylandRenderTarget.h"
#include "Glamor/Wayland/WaylandDisplay.h"
GLAMOR_NAMESPACE_BEGIN

WaylandRenderTarget::WaylandRenderTarget(const Shared<Display>& display, RenderDevice device, int32_t width,
                                         int32_t height, SkColorType format)
    : RenderTarget(display, device, width, height ,format)
    , wl_surface_(nullptr)
    , wl_event_queue_(nullptr)
    , request_next_frame_sequence_counter_(0)
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

namespace {

struct RequestFrameClosure
{
    uint32_t sequence;
    WaylandRenderTarget *target;
};

void wayland_frame_done(void *data, wl_callback *callback, uint32_t extra)
{
    CHECK(callback && data);

    auto *closure = reinterpret_cast<RequestFrameClosure*>(data);
    if (closure->target->GetFrameNotificationRouter())
    {
        closure->target->OnNotifyImplFrame();
        closure->target->GetFrameNotificationRouter()->OnFrameNotification(closure->sequence);
    }

    wl_callback_destroy(callback);
}
wl_callback_listener g_next_frame_listener = { wayland_frame_done };

} // namespace anonymous

uint32_t WaylandRenderTarget::OnRequestNextFrame()
{
    wl_callback *callback = wl_surface_frame(wl_surface_);
    CHECK(callback);

    auto *closure = new RequestFrameClosure{
        .sequence = request_next_frame_sequence_counter_++,
        .target = this
    };

    wl_callback_add_listener(callback, &g_next_frame_listener, closure);
    wl_surface_commit(wl_surface_);

    return closure->sequence;
}

void WaylandRenderTarget::Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept
{
    RenderTarget::Trace(tracer);
    tracer->TraceResource("Wayland Surface",
                          TRACKABLE_TYPE_HANDLE,
                          TRACKABLE_DEVICE_CPU,
                          TRACKABLE_OWNERSHIP_STRICT_OWNED,
                          TraceIdFromPointer(wl_surface_));

    tracer->TraceResource("Wayland Event Queue",
                          TRACKABLE_TYPE_HANDLE,
                          TRACKABLE_DEVICE_CPU,
                          TRACKABLE_OWNERSHIP_STRICT_OWNED,
                          TraceIdFromPointer(wl_event_queue_));
}

GLAMOR_NAMESPACE_END
