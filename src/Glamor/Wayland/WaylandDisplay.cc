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
#include "Glamor/Wayland/WaylandSeat.h"
#include "Glamor/Wayland/WaylandCursorTheme.h"
#include "Glamor/Wayland/WaylandCursor.h"
#include "Glamor/Wayland/WaylandInputContext.h"
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

void WaylandDisplay::RegistryHandleGlobalRemove(void *data,
                                                g_maybe_unused wl_registry *registry,
                                                uint32_t name)
{
    WaylandDisplay *d = WaylandDisplay::BareCast(data);
    auto& globals = d->GetGlobalsIdMap();

    // Registry objects exists either in the `globals` list maintained by `WaylandDisplay`
    // or in `WaylandMonitor` and `WaylandSeat`.
    if (globals.count(name) > 0)
    {
        globals.erase(name);
    }
    else
    {
        if (d->TryRemoveSeat(name))
            return;

        // If `name` is not a seat, it may be a monitor
        auto itr = std::find_if(d->monitors_list_.begin(), d->monitors_list_.end(),
                                [name](const Shared<Monitor>& mon) {
            auto wl_monitor = mon->Cast<WaylandMonitor>();
            return (wl_monitor->GetOutputRegistryId() == name);
        });
        if (itr != d->monitors_list_.end())
        {
            auto wl_monitor = (*itr)->Cast<WaylandMonitor>();
            d->monitors_list_.erase(itr);

            // Notify monitor's listeners that monitor was detached from display
            wl_monitor->Emit(GLSI_MONITOR_DETACHED, PresentSignal());

            // Notify display's listeners that a monitor was removed from list
            PresentSignal emitterInfo;
            emitterInfo.EmplaceBack<Shared<Monitor>>(wl_monitor);
            d->Emit(GLSI_DISPLAY_MONITOR_REMOVED, std::move(emitterInfo));
        }
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
    QLOG(LOG_DEBUG, "shm supports format {} (0x{:08x})", get_shm_format_name(format), format);
}

#define WL_SHM_VERSION      1U

void WaylandDisplay::RegistryHandleGlobal(void *data, wl_registry *registry, uint32_t id,
                                          const char *interface, uint32_t version)
{
    WaylandDisplay *d = WaylandDisplay::BareCast(data);
    CHECK(d);

    d->globals_id_map_[id] = std::string(interface);
    QLOG(LOG_DEBUG, "Interface: %italic<>%fg<bl,hl>{}%reset [ID {}, Version {}]", interface, id, version);

    auto shared_disp = d->Self()->As<WaylandDisplay>();

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
        d->AppendMonitor(WaylandMonitor::Make(shared_disp, output, id));
    }
    else if (strcmp(interface, "wl_seat") == 0)
    {
        auto *seat = (struct wl_seat *)
                wl_registry_bind(registry, id, &wl_seat_interface, version);
        d->AppendSeat(WaylandSeat::Make(shared_disp, seat, id));
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

    // The input context is required in next roundtrip (compositor may send `keymap`
    // event for keyboard device), so it must be created before next roundtrip.
    display->input_context_ = WaylandInputContext::Make(display.get());
    if (!display->input_context_)
    {
        QLOG(LOG_ERROR, "Failed to create input context for new display");
        return nullptr;
    }

    if (wl_display_roundtrip(display->wl_display_) < 0 ||
        wl_display_roundtrip(display->wl_display_) < 0)
    {
        QLOG(LOG_ERROR, "Failed to initialize Wayland globals: {}", strerror(errno));
        return nullptr;
    }

    if (!display->globals_->wl_shm_)
    {
        QLOG(LOG_ERROR, "Registry wl_shm is required but not provided by compositor");
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

    // Load default cursor theme, which can be replaced later by
    // user through `Surface` API.
    auto default_theme = WaylandCursorTheme::MakeDefault(display);
    if (!default_theme)
    {
        QLOG(LOG_ERROR, "Failed to load default cursor theme");
        return nullptr;
    }
    display->AppendDefaultCursorTheme(default_theme);

    scope.cancel();
    return display;
}

void WaylandDisplay::PrepareCallback()
{
    while (wl_display_prepare_read(wl_display_) != 0)
        wl_display_dispatch_pending(wl_display_);
    if (wl_display_flush(wl_display_) < 0 && errno != EAGAIN)
    {
        QLOG(LOG_ERROR, "Lost connection to compositor");
        wl_display_cancel_read(wl_display_);
        Close();
    }
    display_is_reading_ = true;
}

void WaylandDisplay::PollCallback(int status, int events)
{
    if (status < 0)
    {
        QLOG(LOG_ERROR, "Error: {}", uv_strerror(status));
        wl_display_cancel_read(wl_display_);
    }
    else if (events == UV_READABLE)
    {
        wl_display_read_events(wl_display_);
        wl_display_dispatch_pending(wl_display_);
        for (const Shared<Surface>& surface : GetSurfacesList())
        {
            auto rt = std::static_pointer_cast<WaylandRenderTarget>(surface->GetRenderTarget());
            wl_display_dispatch_queue_pending(wl_display_, rt->GetWaylandEventQueue());
        }
    }
    else
    {
        QLOG(LOG_ERROR, "Lost connection to compositor");
        wl_display_cancel_read(wl_display_);
        Close();
    }
    display_is_reading_ = false;
}

void WaylandDisplay::CheckCallback()
{
    if (display_is_reading_)
    {
        wl_display_cancel_read(wl_display_);
        display_is_reading_ = false;
    }
}

WaylandRoundtripScope::WaylandRoundtripScope(Shared<WaylandDisplay> display)
    : display_(std::move(display))
    , changed_(false)
{
    if (display_->display_is_reading_)
    {
        changed_ = true;
        display_->CheckCallback();
    }
}

WaylandRoundtripScope::~WaylandRoundtripScope()
{
    if (changed_)
        display_->PrepareCallback();
}

WaylandDisplay::WaylandDisplay(uv_loop_t *loop, int fd)
    : Display(loop)
    , wl_display_(nullptr)
    , wl_registry_(nullptr)
    , globals_(std::make_unique<Globals>())
    , display_is_reading_(false)
{
    uv_prepare_.emplace(loop);
    uv_check_.emplace(loop);
    uv_poll_.emplace(loop, fd);

    uv_prepare_->Start([this] { PrepareCallback(); });
    uv_check_->Start([this] { CheckCallback(); });
    uv_poll_->Start(UV_READABLE | UV_DISCONNECT, [this](int status, int events) {
        PollCallback(status, events);
    });
}

WaylandDisplay::~WaylandDisplay() = default;

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
    uv_prepare_.reset();
    uv_check_.reset();
    uv_poll_.reset();

    // Check each seat to make sure they are not referenced by other scopes.
    for (const auto& seat : seats_list_)
    {
        CHECK(seat.unique() && "WaylandSeat was referenced by other scopes");
    }

    // All the `WaylandSeat` objects should be destructed here.
    seats_list_.clear();

    input_context_.reset();

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

bool WaylandDisplay::TryRemoveSeat(uint32_t id)
{
    auto itr = std::find_if(seats_list_.begin(), seats_list_.end(),
                            [id](const Shared<WaylandSeat>& ptr) {
        return (ptr->GetRegistryId() == id);
    });

    if (itr == seats_list_.end())
        return false;

    // `WaylandSeat` instances always keep resources of wayland server,
    // unlike `WaylandMonitor`, so making sure it will be released immediately
    // after removing from seats list is necessary.
    CHECK(itr->unique() && "Multiple reference error");

    seats_list_.erase(itr);
    return true;
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

    auto surface = WaylandSurface::Make(rt);
    CHECK(surface);

    auto default_cursor = GetDefaultCursorTheme()->LoadCursorFromName("left_ptr");
    if (!default_cursor)
    {
        QLOG(LOG_WARNING, "Failed to load cursor \"left_ptr\" from default theme");
        QLOG(LOG_WARNING, "The new created surface will have no cursor associated with it");
    }
    else
    {
        surface->SetAttachedCursor(default_cursor);
        QLOG(LOG_DEBUG, "New created surface has an associated cursor from default theme");
    }

    if (surface)
        AppendSurface(surface);
    return surface;
}

bool WaylandDisplay::HasPointerDeviceInSeats()
{
    if (seats_list_.empty())
        return false;

    auto itr = std::find_if(seats_list_.begin(), seats_list_.end(),
                            [](const Shared<WaylandSeat>& seat) -> bool {
        return seat->GetPointerDevice();
    });
    return (itr != seats_list_.end());
}

bool WaylandDisplay::HasKeyboardDeviceInSeats()
{
    if (seats_list_.empty())
        return false;

    auto itr = std::find_if(seats_list_.begin(), seats_list_.end(),
                            [](const Shared<WaylandSeat>& seat) -> bool {
        return seat->GetKeyboardDevice();
    });

    return (itr != seats_list_.end());
}

Shared<WaylandSurface> WaylandDisplay::GetPointerEnteredSurface(wl_pointer *pointer)
{
    CHECK(pointer);

    if (!HasPointerDeviceInSeats())
        return nullptr;

    auto itr = std::find_if(surfaces_list_.begin(), surfaces_list_.end(),
                            [pointer](const Shared<Surface>& surface) -> bool {
        wl_pointer *hover_pointer = surface->As<WaylandSurface>()->GetEnteredPointerDevice();
        return (hover_pointer == pointer);
    });

    if (itr == surfaces_list_.end())
        return nullptr;
    return (*itr)->As<WaylandSurface>();
}

Shared<WaylandSurface> WaylandDisplay::GetKeyboardEnteredSurface(wl_keyboard *keyboard)
{
    CHECK(keyboard);

    if (!HasKeyboardDeviceInSeats())
        return nullptr;

    auto itr = std::find_if(surfaces_list_.begin(), surfaces_list_.end(),
                            [keyboard](const Shared<Surface>& surface) -> bool {
        wl_keyboard *focus = surface->As<WaylandSurface>()->GetEnteredKeyboardDevice();
        return (focus == keyboard);
    });

    if (itr == surfaces_list_.end())
        return nullptr;

    return (*itr)->As<WaylandSurface>();
}

Shared<CursorTheme> WaylandDisplay::OnLoadCursorTheme(const std::string& name, int size)
{
    return WaylandCursorTheme::MakeFromName(Self()->As<WaylandDisplay>(), name, size);
}

Shared<Cursor> WaylandDisplay::OnCreateCursor(const Shared<SkBitmap>& bitmap,
                                              int32_t hotspot_x, int32_t hotspot_y)
{
    CHECK(bitmap);
    CHECK(hotspot_x >= 0 && hotspot_y >= 0);
    return WaylandCursor::MakeFromBitmap(Self()->As<WaylandDisplay>(),
                                         bitmap,
                                         SkIVector::Make(hotspot_x, hotspot_y));
}

void WaylandDisplay::Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept
{
    Display::Trace(tracer);

    tracer->TraceResource("Wayland Display",
                          TRACKABLE_TYPE_HANDLE,
                          TRACKABLE_DEVICE_CPU,
                          TRACKABLE_OWNERSHIP_STRICT_OWNED,
                          TraceIdFromPointer(wl_display_));

    tracer->TraceResource("Wayland Registry",
                          TRACKABLE_TYPE_HANDLE,
                          TRACKABLE_DEVICE_CPU,
                          TRACKABLE_OWNERSHIP_STRICT_OWNED,
                          TraceIdFromPointer(wl_registry_));

    tracer->TraceResource("Wayland Registry: wl_compositor",
                          TRACKABLE_TYPE_HANDLE,
                          TRACKABLE_DEVICE_CPU,
                          TRACKABLE_OWNERSHIP_STRICT_OWNED,
                          TraceIdFromPointer(globals_->wl_compositor_));

    tracer->TraceResource("Wayland Registry: xdg_wm_base",
                          TRACKABLE_TYPE_HANDLE,
                          TRACKABLE_DEVICE_CPU,
                          TRACKABLE_OWNERSHIP_STRICT_OWNED,
                          TraceIdFromPointer(globals_->xdg_wm_base_));

    tracer->TraceResource("Wayland Registry: wl_shm",
                          TRACKABLE_TYPE_HANDLE,
                          TRACKABLE_DEVICE_CPU,
                          TRACKABLE_OWNERSHIP_STRICT_OWNED,
                          TraceIdFromPointer(globals_->wl_shm_));

    if (globals_->zxdg_deco_manager)
    {
        tracer->TraceResource("Wayland Registry: zxdg_decoration_manager_v1",
                              TRACKABLE_TYPE_HANDLE,
                              TRACKABLE_DEVICE_CPU,
                              TRACKABLE_OWNERSHIP_STRICT_OWNED,
                              TraceIdFromPointer(globals_->zxdg_deco_manager));
    }

    if (globals_->kde_deco_manager)
    {
        tracer->TraceResource("Wayland Registry: org_kde_kwin_server_decoration_manager",
                              TRACKABLE_TYPE_HANDLE,
                              TRACKABLE_DEVICE_CPU,
                              TRACKABLE_OWNERSHIP_STRICT_OWNED,
                              TraceIdFromPointer(globals_->kde_deco_manager));
    }

    // TODO(sora): trace seats
}

GLAMOR_NAMESPACE_END
