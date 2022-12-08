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
#include "Core/EnumClassBitfield.h"
#include "Glamor/Glamor.h"
#include "Glamor/Wayland/WaylandSurface.h"
#include "Glamor/Wayland/WaylandRenderTarget.h"
#include "Glamor/Wayland/WaylandDisplay.h"
#include "Glamor/Wayland/WaylandMonitor.h"
#include "Glamor/Wayland/WaylandCursor.h"
#include "Glamor/Wayland/WaylandSeat.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Wayland.Surface)

namespace {

void surface_configure_callback(g_maybe_unused void *data, xdg_surface *surface, uint32_t serial)
{
    xdg_surface_ack_configure(surface, serial);
}

const xdg_surface_listener g_xdg_surface_listener = {
        surface_configure_callback
};

void toplevel_configure_callback(void *data, g_maybe_unused xdg_toplevel *toplevel,
                                 int32_t width, int32_t height, wl_array *states)
{
    auto *w = reinterpret_cast<WaylandSurface *>(data);

    Bitfield<ToplevelStates> st;
    for (auto *p = reinterpret_cast<xdg_toplevel_state *>(states->data);
         reinterpret_cast<uint8_t *>(p) < (reinterpret_cast<uint8_t *>(states->data) + states->size);
         p++)
    {
        switch (*p)
        {
        case XDG_TOPLEVEL_STATE_MAXIMIZED:
            st |= ToplevelStates::kMaximized;
            break;
        case XDG_TOPLEVEL_STATE_FULLSCREEN:
            st |= ToplevelStates::kFullscreen;
            break;
        case XDG_TOPLEVEL_STATE_RESIZING:
            st |= ToplevelStates::kResizing;
            break;
        case XDG_TOPLEVEL_STATE_ACTIVATED:
            st |= ToplevelStates::kActivated;
            break;
        case XDG_TOPLEVEL_STATE_TILED_LEFT:
            st |= ToplevelStates::kTiledLeft;
            break;
        case XDG_TOPLEVEL_STATE_TILED_RIGHT:
            st |= ToplevelStates::kTiledRight;
            break;
        case XDG_TOPLEVEL_STATE_TILED_TOP:
            st |= ToplevelStates::kTiledTop;
            break;
        case XDG_TOPLEVEL_STATE_TILED_BOTTOM:
            st |= ToplevelStates::kTiledBottom;
            break;
        }
    }

    RenderClientEmitterInfo info;
    info.PushBack(width)
        .PushBack(height)
        .PushBack(st);
    w->Emit(GLSI_SURFACE_CONFIGURE, std::move(info));
}

void toplevel_close_callback(void *data, g_maybe_unused xdg_toplevel *toplevel)
{
    auto *w = reinterpret_cast<WaylandSurface *>(data);
    w->Emit(GLSI_SURFACE_CLOSE, RenderClientEmitterInfo());
}

void toplevel_configure_bounds_callback(void *data, xdg_toplevel *toplevel,
                                        int32_t width, int32_t height)
{
    // TODO: implement this.
}

const xdg_toplevel_listener g_xdg_toplevel_listener = {
    .configure = toplevel_configure_callback,
    .close = toplevel_close_callback,
    .configure_bounds = toplevel_configure_bounds_callback
};

}

Shared<Surface> WaylandSurface::Make(const Shared<WaylandRenderTarget>& rt)
{
    auto& g = rt->GetDisplay()->Cast<WaylandDisplay>()->GetGlobalsRef();
    if (!g->xdg_wm_base_)
    {
        QLOG(LOG_ERROR, "Wayland compositor doesn't support xdg_wm_base interface");
        return nullptr;
    }

    auto w = std::make_shared<WaylandSurface>(rt);

    w->wl_display_ = rt->GetDisplay()->Cast<WaylandDisplay>()->GetWaylandDisplay();
    wl_surface *surface = rt->GetWaylandSurface();

    w->xdg_surface_ = xdg_wm_base_get_xdg_surface(g->xdg_wm_base_, surface);
    if (!w->xdg_surface_)
    {
        QLOG(LOG_ERROR, "Failed to get XDG surface for Wayland surface {}", fmt::ptr(surface));
        return nullptr;
    }

    wl_proxy_set_queue(reinterpret_cast<wl_proxy *>(w->xdg_surface_), rt->GetWaylandEventQueue());

    w->xdg_toplevel_ = xdg_surface_get_toplevel(w->xdg_surface_);
    if (!w->xdg_toplevel_)
    {
        QLOG(LOG_ERROR, "Failed to get XDG toplevel for Wayland surface {}", fmt::ptr(surface));
        return nullptr;
    }

    xdg_toplevel_set_app_id(w->xdg_toplevel_, COCOA_FREEDESKTOP_APPID);
    xdg_surface_add_listener(w->xdg_surface_, &g_xdg_surface_listener, w.get());
    xdg_toplevel_add_listener(w->xdg_toplevel_, &g_xdg_toplevel_listener, w.get());

    wl_surface_commit(surface);
    wl_display_roundtrip_queue(w->wl_display_, rt->GetWaylandEventQueue());

    if (g->zxdg_deco_manager)
    {
        w->zxdg_toplevel_deco_ = zxdg_decoration_manager_v1_get_toplevel_decoration(g->zxdg_deco_manager,
                                                                                    w->xdg_toplevel_);
        if (!w->zxdg_toplevel_deco_)
            QLOG(LOG_WARNING, "Window manager doesn't allow server side decoration");
        else
            zxdg_toplevel_decoration_v1_set_mode(w->zxdg_toplevel_deco_,
                                                 ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }
    else if (g->kde_deco_manager)
    {
        w->kde_kwin_server_deco_ = org_kde_kwin_server_decoration_manager_create(g->kde_deco_manager,
                                                                                 surface);
        if (!w->kde_kwin_server_deco_)
            QLOG(LOG_WARNING, "Window manager doesn't allow server side decoration");
        else
            org_kde_kwin_server_decoration_request_mode(w->kde_kwin_server_deco_,
                                                        ORG_KDE_KWIN_SERVER_DECORATION_MANAGER_MODE_SERVER);
    }

    rt->SetOpaque();
    wl_display_roundtrip_queue(w->wl_display_, rt->GetWaylandEventQueue());
    rt->OnClearFrameBuffers();

    return w;
}

WaylandSurface::WaylandSurface(const Shared<WaylandRenderTarget>& rt)
    : Surface(rt)
    , wl_display_(nullptr)
    , wl_surface_(rt->GetWaylandSurface())
    , xdg_surface_(nullptr)
    , xdg_toplevel_(nullptr)
    , zxdg_toplevel_deco_(nullptr)
    , kde_kwin_server_deco_(nullptr)
    , latest_pointer_enter_serial_(0)
    , entered_pointer_device_(nullptr)
    , entered_keyboard_device_(nullptr)
{
}

WaylandSurface::~WaylandSurface() = default;

void WaylandSurface::OnClose()
{
    if (zxdg_toplevel_deco_)
        zxdg_toplevel_decoration_v1_destroy(zxdg_toplevel_deco_);
    if (kde_kwin_server_deco_)
        org_kde_kwin_server_decoration_destroy(kde_kwin_server_deco_);

    if (xdg_toplevel_)
        xdg_toplevel_destroy(xdg_toplevel_);

    if (xdg_surface_)
        xdg_surface_destroy(xdg_surface_);
}

void WaylandSurface::OnSetTitle(const std::string_view& title)
{
    std::string str(title);
    xdg_toplevel_set_title(xdg_toplevel_, str.c_str());
}

void WaylandSurface::OnSetMinSize(int32_t width, int32_t height)
{
    xdg_toplevel_set_min_size(xdg_toplevel_, width, height);
    wl_surface_commit(wl_surface_);
}

void WaylandSurface::OnSetMaxSize(int32_t width, int32_t height)
{
    xdg_toplevel_set_max_size(xdg_toplevel_, width, height);
    wl_surface_commit(wl_surface_);
}

void WaylandSurface::OnSetMinimized(bool value)
{
    if (value)
        xdg_toplevel_set_minimized(xdg_toplevel_);
    // Wayland does not support unset the minimized state actively now,
    // so we ignore that case directly.
}

void WaylandSurface::OnSetMaximized(bool value)
{
    if (value)
        xdg_toplevel_set_maximized(xdg_toplevel_);
    else
        xdg_toplevel_unset_maximized(xdg_toplevel_);
}

void WaylandSurface::OnSetFullscreen(bool value, const Shared<Monitor>& monitor)
{
    if (value && !monitor)
        return;

    if (value)
    {
        wl_output *output = monitor->Cast<WaylandMonitor>()->GetWaylandOutput();
        xdg_toplevel_set_fullscreen(xdg_toplevel_, output);
    }
    else
        xdg_toplevel_unset_fullscreen(xdg_toplevel_);
}

void WaylandSurface::OnSetCursor(const Shared<Cursor>& cursor_base)
{
    // `entered_pointer_device_` is nullptr means that there is no pointer device
    // hovering on the surface. That is, we do not need to render any cursors.
    if (!entered_pointer_device_)
        return;

    Shared<WaylandCursor> cursor = cursor_base->As<WaylandCursor>();
    wl_pointer_set_cursor(entered_pointer_device_,
                          latest_pointer_enter_serial_,
                          cursor->GetCursorSurface(),
                          cursor->GetHotspotVector().x(),
                          cursor->GetHotspotVector().y());
}

void WaylandSurface::Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept
{
    Surface::Trace(tracer);

    tracer->TraceResource("xdg_toplevel",
                          TRACKABLE_TYPE_HANDLE,
                          TRACKABLE_DEVICE_CPU,
                          TRACKABLE_OWNERSHIP_STRICT_OWNED,
                          TraceIdFromPointer(xdg_toplevel_));

    tracer->TraceResource("xdg_surface",
                          TRACKABLE_TYPE_HANDLE,
                          TRACKABLE_DEVICE_CPU,
                          TRACKABLE_OWNERSHIP_STRICT_OWNED,
                          TraceIdFromPointer(xdg_surface_));

    tracer->TraceResource("zxdg_toplevel_decoration_v1",
                          TRACKABLE_TYPE_HANDLE,
                          TRACKABLE_DEVICE_CPU,
                          TRACKABLE_OWNERSHIP_STRICT_OWNED,
                          TraceIdFromPointer(zxdg_toplevel_deco_));

    tracer->TraceResource("org_kde_kwin_server_decoration",
                          TRACKABLE_TYPE_HANDLE,
                          TRACKABLE_DEVICE_CPU,
                          TRACKABLE_OWNERSHIP_STRICT_OWNED,
                          TraceIdFromPointer(kde_kwin_server_deco_));
}

GLAMOR_NAMESPACE_END
