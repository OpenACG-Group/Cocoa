#ifndef COCOA_GLAMOR_WAYLAND_WAYLANDSURFACE_H
#define COCOA_GLAMOR_WAYLAND_WAYLANDSURFACE_H

#include "Glamor/Glamor.h"
#include "Glamor/Surface.h"
#include "Glamor/Wayland/WaylandDisplay.h"
#include "Glamor/Wayland/WaylandRenderTarget.h"
GLAMOR_NAMESPACE_BEGIN

class WaylandSurface : public Surface
{
public:
    explicit WaylandSurface(const Shared<WaylandRenderTarget>& rt);
    ~WaylandSurface() override;

    static Shared<Surface> Make(const Shared<WaylandRenderTarget>& rt);

    void OnClose() override;
    void OnSetTitle(const std::string_view &title) override;

private:
    wl_display          *wl_display_;
    xdg_surface         *xdg_surface_;
    xdg_toplevel        *xdg_toplevel_;
    zxdg_toplevel_decoration_v1         *zxdg_toplevel_deco_;
    org_kde_kwin_server_decoration      *kde_kwin_server_deco_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_WAYLANDSURFACE_H
