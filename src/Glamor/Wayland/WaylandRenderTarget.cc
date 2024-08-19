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

#include "Core/Journal.h"
#include "Glamor/Wayland/WaylandRenderTarget.h"
#include "Glamor/Wayland/WaylandDisplay.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Wayland.RenderTarget)

namespace
{
constexpr wp_fractional_scale_v1_listener g_wp_fractional_scale_listener = {
    .preferred_scale = WaylandRenderTarget::on_preferred_scale
};
} // namespace anonymous

void WaylandRenderTarget::on_preferred_scale(void *data, wp_fractional_scale_v1*, uint32_t scale)
{
    WaylandRenderTarget *self = static_cast<WaylandRenderTarget*>(data);
    QLOG(LOG_DEBUG, "System perferred fractional scale changed: {}:120", scale);
    self->scale_factor_frac_ = scale;
    SkISize surface_size = SkISize::Make(self->GetWidth(), self->GetHeight());
    if (self->wp_viewport_)
        wp_viewport_set_destination(self->wp_viewport_, surface_size.width(), surface_size.height());
    self->OnContentBufferDimensionsUpdate(self->GetContentBufferDimensions(surface_size));
}

WaylandRenderTarget::WaylandRenderTarget(const std::shared_ptr<Display>& display,
                                         RenderDevice device,
                                         int32_t width,
                                         int32_t height,
                                         SkColorType format,
                                         wl_surface *surface,
                                         wl_event_queue *surface_queue)
    : RenderTarget(display, device, width, height ,format)
    , wl_surface_(surface)
    , wl_event_queue_(surface_queue)
    , current_frame_callback_(nullptr)
    , request_next_frame_sequence_counter_(0)
    , wp_fractional_scale_(nullptr)
    , wp_viewport_(nullptr)
    , scale_factor_frac_(120)
{
    wl_surface_set_user_data(wl_surface_, this);

    const std::unique_ptr<WaylandDisplay::Globals>& g = display->Cast<WaylandDisplay>()->GetGlobalsRef();
    if (g->viewporter)
    {
        wp_viewport_ = wp_viewporter_get_viewport(g->viewporter, surface);
        if (!wp_viewport_)
            QLOG(LOG_ERROR, "Compositor supports wp_viewporter but failed to create wp_viewport");
    }

    if (g->fractional_scale_manager)
    {
        wp_fractional_scale_ = wp_fractional_scale_manager_v1_get_fractional_scale(
            g->fractional_scale_manager, surface);
        if (!wp_fractional_scale_)
            QLOG(LOG_ERROR, "Compositor supports wp_factional_scale_manager but failed to create wp_fractional_scale");
        else
            wp_fractional_scale_v1_add_listener(wp_fractional_scale_, &g_wp_fractional_scale_listener, this);
    }
}

WaylandRenderTarget::~WaylandRenderTarget()
{
    if (wp_fractional_scale_)
        wp_fractional_scale_v1_destroy(wp_fractional_scale_);
    if (wp_viewport_)
        wp_viewport_destroy(wp_viewport_);
    if (wl_surface_)
        wl_surface_destroy(wl_surface_);
    if (wl_event_queue_)
        wl_event_queue_destroy(wl_event_queue_);
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

wl_callback_listener g_next_frame_listener = {
    WaylandRenderTarget::wayland_frame_done
};

} // namespace anonymous

void WaylandRenderTarget::wayland_frame_done(void *data, wl_callback *callback, uint32_t extra)
{
    CHECK(callback && data);
    auto *closure = reinterpret_cast<RequestFrameClosure*>(data);
    CHECK(closure->target->current_frame_callback_ == callback);

    if (closure->target->GetFrameNotificationRouter())
    {
        closure->target->OnNotifyImplFrame();
        closure->target->GetFrameNotificationRouter()->OnFrameNotification(closure->sequence);
    }

    closure->target->current_frame_callback_ = nullptr;
    delete closure;
    wl_callback_destroy(callback);
}

uint32_t WaylandRenderTarget::OnRequestNextFrame()
{
    OnTryCancelCurrentFrameRequest();
    wl_callback *callback = wl_surface_frame(wl_surface_);
    CHECK(callback);

    auto *closure = new RequestFrameClosure{
        .sequence = request_next_frame_sequence_counter_++,
        .target = this
    };

    wl_callback_add_listener(callback, &g_next_frame_listener, closure);
    wl_surface_commit(wl_surface_);

    current_frame_callback_ = callback;

    return closure->sequence;
}

void WaylandRenderTarget::OnTryCancelCurrentFrameRequest()
{
    // No pending frame, nothing should be done.
    if (!current_frame_callback_)
        return;

    delete static_cast<RequestFrameClosure*>(
        wl_callback_get_user_data(current_frame_callback_));

    wl_callback_destroy(current_frame_callback_);
    current_frame_callback_ = nullptr;
}

void WaylandRenderTarget::OnResize(int32_t width, int32_t height)
{
    if (wp_viewport_)
        wp_viewport_set_destination(wp_viewport_, width, height);
    OnContentBufferDimensionsUpdate(SkISize::Make(width, height));
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
