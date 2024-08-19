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
#include "Glamor/Wayland/protos/fractional-scale-v1.h"
#include "Glamor/Wayland/protos/viewporter.h"
GLAMOR_NAMESPACE_BEGIN

class WaylandRenderTarget : public RenderTarget
{
public:
    WaylandRenderTarget(const std::shared_ptr<Display>& display,
                        RenderDevice device,
                        int32_t width,
                        int32_t height,
                        SkColorType format,
                        wl_surface *surface,
                        wl_event_queue *surface_queue);

    ~WaylandRenderTarget() override;

    g_nodiscard wl_surface *GetWaylandSurface() {
        return wl_surface_;
    }

    g_nodiscard wl_event_queue *GetWaylandEventQueue() {
        return wl_event_queue_;
    }

    void SetOpaque();

    g_nodiscard uint32_t GetHiDPIScaleFracFactor() const {
        return scale_factor_frac_;
    }

    g_nodiscard float GetHiDPIScaleFactor() const {
        return static_cast<float>(scale_factor_frac_) / 120.0f;
    }

    virtual void OnClearFrameBuffers();
    virtual void OnNotifyImplFrame() {}

    uint32_t OnRequestNextFrame() override;
    void OnTryCancelCurrentFrameRequest() override;
    void OnResize(int32_t width, int32_t height) override;

    void Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept override;

    static void wayland_frame_done(void *data, wl_callback *callback, uint32_t extra);
    static void on_preferred_scale(void *data, wp_fractional_scale_v1 *iface, uint32_t scale);

protected:
    g_nodiscard SkISize GetContentBufferDimensions(const std::optional<SkISize>& not_scaled = {}) const {
        int32_t width, height;
        if (not_scaled)
        {
            width = not_scaled->width();
            height = not_scaled->height();
        }
        else
        {
            width = GetWidth();
            height = GetHeight();
        }
        // Wayland specification constrains the rounding method: rounded halfway away from zero.
        // See https://wayland.app/protocols/fractional-scale-v1
        float scalar = GetHiDPIScaleFactor();
        return SkISize::Make(static_cast<int>(std::round(scalar * static_cast<float>(width))),
                             static_cast<int>(std::round(scalar * static_cast<float>(height))));
    }

    // This is called when the dimensions of content buffer should be changed.
    // Several events can cause this, e.g. window resizing, change of scale factor, etc.
    virtual void OnContentBufferDimensionsUpdate(const SkISize& dimensions) = 0;

    wl_surface              *wl_surface_;
    wl_event_queue          *wl_event_queue_;
    wl_callback             *current_frame_callback_;
    uint32_t                 request_next_frame_sequence_counter_;
    wp_fractional_scale_v1  *wp_fractional_scale_;
    wp_viewport             *wp_viewport_;
    uint32_t                 scale_factor_frac_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_WAYLANDRENDERTARGET_H
