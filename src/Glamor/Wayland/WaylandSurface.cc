#include "Core/Journal.h"
#include "Core/EnumClassBitfield.h"
#include "Glamor/Glamor.h"
#include "Glamor/Wayland/WaylandSurface.h"
#include "Glamor/Wayland/WaylandRenderTarget.h"
#include "Glamor/Wayland/WaylandDisplay.h"
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
    w->Emit(CRSI_SURFACE_CONFIGURE, std::move(info));
}

void toplevel_close_callback(void *data, g_maybe_unused xdg_toplevel *toplevel)
{
    auto *w = reinterpret_cast<WaylandSurface *>(data);
    w->Emit(CRSI_SURFACE_CLOSE, RenderClientEmitterInfo());
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

    xdg_toplevel_set_app_id(w->xdg_toplevel_, "org.openacg.cocoa");
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
    , xdg_surface_(nullptr)
    , xdg_toplevel_(nullptr)
    , zxdg_toplevel_deco_(nullptr)
    , kde_kwin_server_deco_(nullptr)
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

GLAMOR_NAMESPACE_END
