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

#ifndef COCOA_GLAMOR_WAYLAND_WAYLANDSURFACE_H
#define COCOA_GLAMOR_WAYLAND_WAYLANDSURFACE_H

#include <set>

#include "Glamor/Glamor.h"
#include "Glamor/Surface.h"
#include "Glamor/Wayland/WaylandDisplay.h"
#include "Glamor/Wayland/WaylandRenderTarget.h"
GLAMOR_NAMESPACE_BEGIN

class WaylandSurface : public Surface
{
public:
    explicit WaylandSurface(const std::shared_ptr<WaylandRenderTarget>& rt);
    ~WaylandSurface() override;

    static std::shared_ptr<Surface> Make(const std::shared_ptr<WaylandRenderTarget>& rt);

    g_nodiscard g_inline wl_surface *GetWaylandSurface() const {
        return wl_surface_;
    }

    void OnClose() override;
    void OnSetTitle(const std::string_view &title) override;
    void OnSetMinSize(int32_t width, int32_t height) override;
    void OnSetMaxSize(int32_t width, int32_t height) override;
    void OnSetMinimized(bool value) override;
    void OnSetMaximized(bool value) override;
    void OnSetFullscreen(bool value, const std::shared_ptr<Monitor>& monitor) override;

    void OnSetCursor(const std::shared_ptr<Cursor> &cursor) override;

    g_private_api g_inline void SetPointerEntered(uint32_t serial, wl_pointer *device) {
        latest_pointer_enter_serial_ = serial;
        entered_pointer_device_ = device;
    }

    g_private_api g_nodiscard g_inline uint32_t GetLatestPointerEnterEventSerial() const {
        return latest_pointer_enter_serial_;
    }

    g_private_api g_nodiscard g_inline wl_pointer *GetEnteredPointerDevice() const {
        return entered_pointer_device_;
    }

    g_private_api g_inline void SetKeyboardEntered(wl_keyboard *device) {
        entered_keyboard_device_ = device;
    }

    g_private_api g_nodiscard g_inline wl_keyboard *GetEnteredKeyboardDevice() const {
        return entered_keyboard_device_;
    }

    void Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept override;

private:
    wl_display                          *wl_display_;

    // Although we keep a reference of `wl_surface` here, but we will NOT
    // take the ownership of this surface object. It is owned by `RenderTarget`
    // and `RenderTarget` is owned by `Surface`.
    wl_surface                          *wl_surface_;

    xdg_surface                         *xdg_surface_;
    xdg_toplevel                        *xdg_toplevel_;
    zxdg_toplevel_decoration_v1         *zxdg_toplevel_deco_;
    org_kde_kwin_server_decoration      *kde_kwin_server_deco_;

    uint32_t                             latest_pointer_enter_serial_;
    wl_pointer                          *entered_pointer_device_;

    wl_keyboard                         *entered_keyboard_device_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_WAYLANDSURFACE_H
