#ifndef COCOA_COBALT_WAYLAND_WAYLANDSURFACE_H
#define COCOA_COBALT_WAYLAND_WAYLANDSURFACE_H

#include "Cobalt/Cobalt.h"
#include "Cobalt/Surface.h"
#include "Cobalt/Wayland/WaylandDisplay.h"
#include "Cobalt/Wayland/WaylandRenderTarget.h"
COBALT_NAMESPACE_BEGIN

class WaylandSurface : public Surface
{
public:
    explicit WaylandSurface(const co_sp<WaylandRenderTarget>& rt);
    ~WaylandSurface() override;

    static co_sp<Surface> Make(const co_sp<WaylandRenderTarget>& rt);

    void OnClose() override;
    void OnSetTitle(const std::string_view &title) override;

private:
    wl_display          *wl_display_;
    xdg_surface         *xdg_surface_;
    xdg_toplevel        *xdg_toplevel_;
    zxdg_toplevel_decoration_v1         *zxdg_toplevel_deco_;
    org_kde_kwin_server_decoration      *kde_kwin_server_deco_;
};

COBALT_NAMESPACE_END
#endif //COCOA_COBALT_WAYLAND_WAYLANDSURFACE_H
