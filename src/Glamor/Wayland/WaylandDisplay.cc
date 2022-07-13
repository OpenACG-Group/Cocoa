#include <cstring>

#include "uv.h"
#include "Core/Journal.h"
#include "Core/Errors.h"
#include "Core/Exception.h"
#include "Glamor/Wayland/WaylandDisplay.h"
#include "Glamor/Wayland/WaylandSHMRenderTarget.h"
#include "Glamor/Wayland/WaylandHWComposeRenderTarget.h"
#include "Glamor/Wayland/WaylandSurface.h"
#include "Glamor/Wayland/WaylandMonitor.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Wayland.Display)

namespace {

const wl_registry_listener g_registry_listener = {
    .global = WaylandDisplay::RegistryHandleGlobal,
    .global_remove = WaylandDisplay::RegistryHandleGlobalRemove
};

// NOLINTNEXTLINE
const xdg_wm_base_listener g_wm_base_listener = {
    .ping = [](g_maybe_unused void *data, xdg_wm_base *wm, uint32_t serial) {
        xdg_wm_base_pong(wm, serial);
    }
};

const wl_shm_listener g_shm_listener = {
    .format = WaylandDisplay::ShmFormatHandler
};

thread_local char g_wayland_log_buffer[1024];
void wayland_log_handler(const char *fmt, va_list va)
{
    va_list stored_va_list;

    // `vsnprintf` consumes `va_list` structure after the invocation,
    // so we copy it as we will use it twice.
    va_copy(stored_va_list, va);

    // Compute the length of formatted string
    size_t size = vsnprintf(nullptr, 0, fmt, stored_va_list);

    char *content_ptr;
    bool free_ptr = false;
    if (size < sizeof(g_wayland_log_buffer))
    {
        content_ptr = g_wayland_log_buffer;
    }
    else
    {
        content_ptr = new char[size + 1];
        free_ptr = true;
    }

    vsnprintf(content_ptr, size + 1, fmt, va);
    QLOG(LOG_WARNING, "(wayland-client) {}", content_ptr);

    if (free_ptr)
        delete[] content_ptr;
}

} // namespace anonymous

void WaylandDisplay::RegistryHandleGlobalRemove(void *data, g_maybe_unused wl_registry *registry, uint32_t name)
{
    WaylandDisplay *d = WaylandDisplay::BareCast(data);
    auto& globals = d->GetGlobalsIdMap();
    if (globals.count(name) > 0)
    {
        // TODO: remove corresponding seat and monitor
        globals.erase(name);
    }
}

std::string get_shm_format_name(uint32_t format)
{
    if (format == WL_SHM_FORMAT_ARGB8888)
        return "ARGB8888";
    else if (format == WL_SHM_FORMAT_XRGB8888)
        return "XRGB8888";
    else
    {
        char buf[10];
        snprintf(buf, 10, "4cc[%c%c%c%c]",
                 (char) (format & 0xff),
                 (char) (format >> 8) & 0xff,
                 (char) (format >> 16) & 0xff,
                 (char) (format >> 24) & 0xff);
        return buf;
    }
}

void WaylandDisplay::ShmFormatHandler(void *data, g_maybe_unused wl_shm *shm, uint32_t format)
{
    WaylandDisplay *d = WaylandDisplay::BareCast(data);
    CHECK(d);

    d->wl_shm_formats_.push_back(static_cast<wl_shm_format>(format));
    QLOG(LOG_DEBUG, "shm supported format {} (0x{:08x})", get_shm_format_name(format), format);
}

#define WL_SHM_VERSION      1U

void WaylandDisplay::RegistryHandleGlobal(void *data, wl_registry *registry, uint32_t id,
                                          const char *interface, uint32_t version)
{
    WaylandDisplay *d = WaylandDisplay::BareCast(data);
    CHECK(d);

    d->globals_id_map_[id] = std::string(interface);
    QLOG(LOG_DEBUG, "Interface: %italic<>%fg<bl,hl>{}%reset [ID {}, Version {}]", interface, id, version);

    if (strcmp(interface, "wl_compositor") == 0)
    {
        d->globals_->wl_compositor_ = (struct wl_compositor *)
                wl_registry_bind(registry, id, &wl_compositor_interface, version);
    }
    else if (strcmp(interface, "xdg_wm_base") == 0)
    {
        d->globals_->xdg_wm_base_ = (struct xdg_wm_base *)
                wl_registry_bind(registry, id, &xdg_wm_base_interface, version);
        xdg_wm_base_add_listener(d->globals_->xdg_wm_base_, &g_wm_base_listener, d);
    }
    else if (strcmp(interface, "wl_shm") == 0)
    {
        d->globals_->wl_shm_ = (struct wl_shm *)
                wl_registry_bind(registry, id, &wl_shm_interface, std::min(WL_SHM_VERSION, version));
        wl_shm_add_listener(d->globals_->wl_shm_, &g_shm_listener, d);
    }
    else if (strcmp(interface, "zxdg_decoration_manager_v1") == 0)
    {
        d->globals_->zxdg_deco_manager = (struct zxdg_decoration_manager_v1 *)
                wl_registry_bind(registry, id, &zxdg_decoration_manager_v1_interface, 1);
    }
    else if (strcmp(interface, "org_kde_kwin_server_decoration_manager") == 0)
    {
        d->globals_->kde_deco_manager = (struct org_kde_kwin_server_decoration_manager *)
                wl_registry_bind(registry, id, &org_kde_kwin_server_decoration_manager_interface, 1);
    }
    else if (strcmp(interface, "wl_output") == 0)
    {
        auto *output = (struct wl_output *)
                wl_registry_bind(registry, id, &wl_output_interface, version);
        d->AppendMonitor(WaylandMonitor::Make(d->Self()->As<WaylandDisplay>(), output));
    }
}

Shared<WaylandDisplay> WaylandDisplay::Connect(uv_loop_t *loop, const std::string& name)
{
    CHECK(loop);
    wl_log_set_handler_client(wayland_log_handler);

    wl_display *d = wl_display_connect(name.empty() ? nullptr : name.c_str());

    if (!d)
    {
        QLOG(LOG_ERROR, "Failed to connect to Wayland compositor: {}", strerror(errno));
        return nullptr;
    }

    auto display = std::make_shared<WaylandDisplay>(loop, wl_display_get_fd(d));
    ScopeExitAutoInvoker scope([&display] {
        display->Close();
    });

    display->wl_display_ = d;
    wl_display_set_user_data(display->wl_display_, display.get());

    display->wl_registry_ = wl_display_get_registry(display->wl_display_);
    wl_registry_add_listener(display->wl_registry_, &g_registry_listener, display.get());

    if (wl_display_roundtrip(display->wl_display_) < 0 ||
        wl_display_roundtrip(display->wl_display_) < 0)
    {
        QLOG(LOG_ERROR, "Failed to initialize Wayland globals: {}", strerror(errno));
        return nullptr;
    }

    if (display->globals_->xdg_wm_base_)
    {
        QLOG(LOG_INFO, "Using %fg<gr,hl>XDG shell client protocol%reset as Wayland shell");
    }
    else
    {
        QLOG(LOG_ERROR, "No available Wayland shell protocols [xdg_wm_base required]");
        return nullptr;
    }

    scope.cancel();
    return display;
}

void WaylandDisplay::PrepareCallback(uv_prepare_t *prepare)
{
    WaylandDisplay *d = WaylandDisplay::BareCast(prepare->data);
    while (wl_display_prepare_read(d->wl_display_) != 0)
        wl_display_dispatch_pending(d->wl_display_);

    if (wl_display_flush(d->wl_display_) < 0 && errno != EAGAIN)
    {
        QLOG(LOG_ERROR, "Lost connection to compositor");
        wl_display_cancel_read(d->wl_display_);
        d->Close();
    }

    d->display_is_reading_ = true;
}

void WaylandDisplay::PollCallback(uv_poll_t *poll, int status, int events)
{
    WaylandDisplay *d = WaylandDisplay::BareCast(poll->data);

    if (status < 0)
    {
        QLOG(LOG_ERROR, "Error: {}", uv_strerror(status));
        wl_display_cancel_read(d->wl_display_);
    }
    else if (events == UV_READABLE)
    {
        wl_display_read_events(d->wl_display_);
        wl_display_dispatch_pending(d->wl_display_);
        for (const Shared<Surface>& surface : d->GetSurfacesList())
        {
            auto rt = std::dynamic_pointer_cast<WaylandRenderTarget>(surface->GetRenderTarget());
            wl_display_dispatch_queue_pending(d->wl_display_, rt->GetWaylandEventQueue());
        }
    }
    else
    {
        QLOG(LOG_ERROR, "Lost connection to compositor");
        wl_display_cancel_read(d->wl_display_);
        d->Close();
    }

    d->display_is_reading_ = false;
}

void WaylandDisplay::CheckCallback(uv_check_t *check)
{
    WaylandDisplay *d = WaylandDisplay::BareCast(check->data);
    if (d->display_is_reading_)
    {
        wl_display_cancel_read(d->wl_display_);
        d->display_is_reading_ = false;
    }
}

WaylandRoundtripScope::WaylandRoundtripScope(Shared<WaylandDisplay> display)
    : display_(std::move(display))
    , changed_(false)
{
    if (display_->display_is_reading_)
    {
        changed_ = true;
        display_->CheckCallback(display_->uv_check_handle_);
    }
}

WaylandRoundtripScope::~WaylandRoundtripScope()
{
    if (changed_)
        display_->PrepareCallback(display_->uv_prepare_handle_);
}

WaylandDisplay::WaylandDisplay(uv_loop_t *loop, int fd)
    : Display(loop)
    , wl_display_(nullptr)
    , wl_registry_(nullptr)
    , globals_(std::make_unique<Globals>())
    , uv_prepare_handle_(nullptr)
    , uv_check_handle_(nullptr)
    , uv_poll_handle_(nullptr)
    , display_is_reading_(false)
{
    uv_prepare_handle_ = reinterpret_cast<uv_prepare_t *>(::malloc(sizeof(uv_prepare_t)));
    uv_prepare_init(loop, uv_prepare_handle_);
    uv_prepare_start(uv_prepare_handle_, PrepareCallback);
    uv_handle_set_data(reinterpret_cast<uv_handle_t *>(uv_prepare_handle_), this);

    uv_check_handle_ = reinterpret_cast<uv_check_t *>(::malloc(sizeof(uv_check_t)));
    uv_check_init(loop, uv_check_handle_);
    uv_check_start(uv_check_handle_, CheckCallback);
    uv_handle_set_data(reinterpret_cast<uv_handle_t *>(uv_check_handle_), this);

    uv_poll_handle_ = reinterpret_cast<uv_poll_t *>(::malloc(sizeof(uv_poll_t)));
    uv_poll_init(loop, uv_poll_handle_, fd);
    uv_poll_start(uv_poll_handle_, UV_READABLE | UV_DISCONNECT, PollCallback);
    uv_handle_set_data(reinterpret_cast<uv_handle_t *>(uv_poll_handle_), this);
}

WaylandDisplay::~WaylandDisplay()
{
    uv_close(reinterpret_cast<uv_handle_t *>(uv_prepare_handle_), [](uv_handle_t *hnd) {
        ::free(hnd);
    });

    uv_close(reinterpret_cast<uv_handle_t *>(uv_check_handle_), [](uv_handle_t *hnd) {
        ::free(hnd);
    });

    uv_close(reinterpret_cast<uv_handle_t *>(uv_poll_handle_), [](uv_handle_t *hnd) {
        ::free(hnd);
    });
}

WaylandDisplay::Globals::~Globals()
{
    if (wl_compositor_)
        wl_compositor_destroy(wl_compositor_);
    if (wl_shm_)
        wl_shm_destroy(wl_shm_);
    if (xdg_wm_base_)
        xdg_wm_base_destroy(xdg_wm_base_);
    if (zxdg_deco_manager)
        zxdg_decoration_manager_v1_destroy(zxdg_deco_manager);
    if (kde_deco_manager)
        org_kde_kwin_server_decoration_manager_destroy(kde_deco_manager);
}

void WaylandDisplay::OnDispose()
{
    uv_prepare_stop(uv_prepare_handle_);
    uv_check_stop(uv_check_handle_);
    uv_poll_stop(uv_poll_handle_);

    globals_.reset();

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

namespace {

// NOLINTNEXTLINE
std::map<wl_shm_format, SkColorType> g_wl_shm_format_mapping = {
    { WL_SHM_FORMAT_ARGB8888, SkColorType::kBGRA_8888_SkColorType },
    { WL_SHM_FORMAT_XRGB8888, SkColorType::kBGRA_8888_SkColorType },
    { WL_SHM_FORMAT_ABGR8888, SkColorType::kRGBA_8888_SkColorType },
    { WL_SHM_FORMAT_XBGR8888, SkColorType::kRGBA_8888_SkColorType }
};

}

SkColorType WlShmFormatToSkColorType(wl_shm_format type)
{
    if (g_wl_shm_format_mapping.count(type) > 0)
        return g_wl_shm_format_mapping[type];
    return SkColorType::kUnknown_SkColorType;
}

wl_shm_format SkColorTypeToWlShmFormat(SkColorType type)
{
    for (const auto& p : g_wl_shm_format_mapping)
    {
        if (type == p.second)
            return p.first;
    }
    throw RuntimeException(__func__, "Unsupported color type");
}

std::vector<SkColorType> WaylandDisplay::GetRasterColorFormats()
{
    std::vector<SkColorType> result;
    for (wl_shm_format f : wl_shm_formats_)
    {
        if (g_wl_shm_format_mapping.count(f) > 0)
            result.push_back(g_wl_shm_format_mapping[f]);
    }
    return result;
}

Shared<Surface> WaylandDisplay::OnCreateSurface(int32_t width, int32_t height, SkColorType format,
                                                RenderTarget::RenderDevice device)
{
    WaylandRoundtripScope scope(Self()->Cast<WaylandDisplay>());

    Shared<WaylandRenderTarget> rt;
    switch (device)
    {
    case RenderTarget::RenderDevice::kRaster:
        rt = WaylandSHMRenderTarget::Make(this->Cast<WaylandDisplay>(), width, height, format);
        break;

    case RenderTarget::RenderDevice::kHWComposer:
        rt = WaylandHWComposeRenderTarget::Make(this->Cast<WaylandDisplay>(), width, height);
        break;
    }

    if (!rt)
    {
        QLOG(LOG_ERROR, "Failed to create RenderTarget on display");
        return nullptr;
    }

    return WaylandSurface::Make(rt);
}

GLAMOR_NAMESPACE_END
