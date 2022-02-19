#include <cstring>

#include "Core/EventSource.h"
#include "Core/EventLoop.h"
#include "Core/Journal.h"
#include "Core/Errors.h"
#include "Core/Exception.h"
#include "Cobalt/Wayland/WaylandDisplay.h"
COBALT_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Cobalt.Wayland.Display)

namespace {

const wl_registry_listener registry_listener_ = {
    .global = WaylandDisplay::RegistryHandleGlobal,
    .global_remove = WaylandDisplay::RegistryHandleGlobalRemove
};

// NOLINTNEXTLINE
const xdg_wm_base_listener wm_base_listener_ = {
    .ping = [](g_maybe_unused void *data, xdg_wm_base *wm, uint32_t serial) {
        xdg_wm_base_pong(wm, serial);
    }
};

} // namespace anonymous

void WaylandDisplay::RegistryHandleGlobalRemove(void *data, g_maybe_unused wl_registry *registry, uint32_t name)
{
    WaylandDisplay *d = WaylandDisplay::BareCast(data);
    auto& globals = d->GetGlobalsIdMap();
    if (globals.count(name) > 0)
    {
        // TODO: remove corresponding seat
        globals.erase(name);
    }
}

void WaylandDisplay::RegistryHandleGlobal(void *data, wl_registry *registry, uint32_t id,
                                          const char *interface, uint32_t version)
{
    WaylandDisplay *d = WaylandDisplay::BareCast(data);

    d->globals_id_map_[id] = std::string(interface);
    QLOG(LOG_DEBUG, "Interface: %italic<>%fg<bl,hl>{}%reset [ID {}, Version {}]", interface, id, version);

    if (strcmp(interface, "wl_compositor") == 0)
    {
        d->wl_compositor_ = (struct wl_compositor *)
                            wl_registry_bind(registry, id, &wl_compositor_interface, version);
    }
    else if (strcmp(interface, "xdg_wm_base") == 0)
    {
        d->xdg_wm_base_ = (struct xdg_wm_base *)
                          wl_registry_bind(registry, id, &xdg_wm_base_interface, version);
        xdg_wm_base_add_listener(d->xdg_wm_base_, &wm_base_listener_, d);
    }
}

co_sp<WaylandDisplay> WaylandDisplay::Connect(uv_loop_t *loop, const std::string& name)
{
    CHECK(loop);

    wl_display *d = wl_display_connect(name.empty() ? nullptr : name.c_str());

    if (!d)
    {
        QLOG(LOG_ERROR, "Failed to connect to Wayland compositor: {}", strerror(errno));
        return nullptr;
    }

    auto display = std::make_shared<WaylandDisplay>(loop, wl_display_get_fd(d));
    ScopeEpilogue scope([&display] {
        display->Close();
    });

    display->wl_display_ = d;
    wl_display_set_user_data(display->wl_display_, display.get());

    display->wl_registry_ = wl_display_get_registry(display->wl_display_);
    wl_registry_add_listener(display->wl_registry_, &registry_listener_, display.get());

    if (wl_display_roundtrip(display->wl_display_) < 0 ||
        wl_display_roundtrip(display->wl_display_) < 0)
    {
        QLOG(LOG_ERROR, "Failed to initialize Wayland globals: {}", strerror(errno));
        return nullptr;
    }

    if (display->xdg_wm_base_)
    {
        QLOG(LOG_INFO, "Using %fg<gr,hl>XDG shell client protocol%reset as Wayland shell");
    }
    else
    {
        QLOG(LOG_ERROR, "No available Wayland shell protocols [xdg_wm_base required]");
        return nullptr;
    }

    scope.abolish();
    return display;
}

WaylandDisplay::WaylandDisplay(uv_loop_t *loop, int fd)
    : Display(loop)
    , wl_display_(nullptr)
    , wl_registry_(nullptr)
    , wl_compositor_(nullptr)
    , xdg_wm_base_(nullptr)
{
}

WaylandDisplay::~WaylandDisplay() = default;

void WaylandDisplay::OnDispose()
{
    if (xdg_wm_base_)
    {
        xdg_wm_base_destroy(xdg_wm_base_);
        xdg_wm_base_ = nullptr;
    }

    if (wl_compositor_)
    {
        wl_compositor_destroy(wl_compositor_);
        wl_compositor_ = nullptr;
    }

    if (wl_registry_)
    {
        wl_registry_destroy(wl_registry_);
        wl_registry_ = nullptr;
    }

    if (wl_display_)
    {
        wl_display_disconnect(wl_display_);
        wl_display_ = nullptr;
    }
}

COBALT_NAMESPACE_END
