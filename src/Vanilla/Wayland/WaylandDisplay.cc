#include <wayland-client.h>

#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Core/UniquePersistent.h"

#include "Vanilla/Context.h"
#include "Vanilla/Display.h"
#include "Vanilla/Wayland/WaylandSeat.h"
#include "Vanilla/Wayland/WaylandDisplay.h"
VANILLA_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Vanilla.Wayland)

namespace {

struct FutureIFaceRequirement : UniquePersistent<FutureIFaceRequirement>
{
    struct Req
    {
        std::vector<std::string> require_ifaces;
        uint32_t id;
        uint32_t version;
        std::function<void(WaylandDisplay*, const Req&)> callback;
    };
    std::list<Req> requests;

    static bool hasRequiredIfaces(const Req& req, WaylandDisplay *wd)
    {
        const auto& known = wd->getInterfacesBatch()->known_ifaces;
        for (auto& iface : req.require_ifaces) // NOLINT
        {
            if (std::find(known.begin(), known.end(), iface) == known.end())
                return false;
        }
        return true;
    }

    void processRequests(WaylandDisplay *display)
    {
        for (auto itr = requests.begin(); itr != requests.end(); itr++)
        {
            if (hasRequiredIfaces(*itr, display))
            {
                itr->callback(display, *itr);
                itr = requests.erase(itr);
            }
            if (itr == requests.end())
                break;
        }
    }
};

void wl_client_logger(const char *format, va_list va)
{
    size_t size = ::vsnprintf(nullptr, 0, format, va);
    char *buffer = new char[size + 1];
    CHECK(buffer);
    ::vsnprintf(buffer, size + 1, format, va);
    QLOG(LOG_INFO, "{}", buffer);
}

void registry_listener_global_add(void *data, ::wl_registry *registry, uint32_t name,
                                  const char *interface, uint32_t version)
{
    QLOG(LOG_DEBUG, "Wayland interface: {} [{}, version {}]", interface, name, version);
    WaylandDisplay *display = WaylandDisplay::GetBareFromUserData(data);
    CHECK(display);

    auto& batch = display->getInterfacesBatch();

#define iface_match(s) std::strcmp(interface, s) == 0
    if (iface_match("wl_compositor"))
    {
        batch->compositor = {
            (::wl_compositor*)
            ::wl_registry_bind(registry, name,
                               &::wl_compositor_interface,
                               std::min(version, 4U)),
            std::min(version, 4U)
        };
    }
    else if (iface_match("wl_shm"))
    {
        batch->shm = {
            (::wl_shm*)
            ::wl_registry_bind(registry, name, &::wl_shm_interface, 1),
            1
        };
    }
    else if (iface_match("wl_data_device_manager"))
    {
        batch->data_device_manager = {
            (::wl_data_device_manager*)
            ::wl_registry_bind(registry, name,
                               &::wl_data_device_manager_interface,
                               std::min(version, 3U)),
            std::min(version, 3U)
        };
    }
    else if (iface_match("wl_subcompositor"))
    {
        batch->subcompositor = {
            (::wl_subcompositor*)
            ::wl_registry_bind(registry, name, &::wl_subcompositor_interface, 1),
            1
        };
    }
    else if (iface_match("wl_seat"))
    {
        FutureIFaceRequirement::Req req{};
        req.id = name;
        req.version = version;
        req.require_ifaces = {"wl_compositor", "wl_data_device_manager"};
        req.callback = [registry](WaylandDisplay *display, const FutureIFaceRequirement::Req& req) {
            auto& batch = display->getInterfacesBatch();
            batch->seat = {
                (::wl_seat *) ::wl_registry_bind(registry, req.id,
                                                 &::wl_seat_interface, std::min(req.version, 7U)),
                req.id,
                std::min(req.version, 7U)
            };
            display->createSeatFromIFacesBatch();
            display->asyncRoundtrip();
        };
        FutureIFaceRequirement::Ref().requests.push_back(req);
    }
    else if (iface_match("xdg_wm_base"))
    {
        batch->xdg_wm_base = {nullptr, name, version};
    }
    else if (iface_match("zxdg_shell_v6"))
    {
        batch->zxdg_shell_v6 = {nullptr, name, version};
    }
    else if (iface_match("zwp_primary_selection_device_manager_v1"))
    {
        batch->primary_selection_manager = {
            (::zwp_primary_selection_device_manager_v1*)
            ::wl_registry_bind(registry, name,
                               &::zwp_primary_selection_device_manager_v1_interface,
                               version),
            version
        };
    }
    else if (iface_match("zwp_pointer_gestures_v1"))
    {
        batch->pointer_gestures = {
            (::zwp_pointer_gestures_v1*)
            ::wl_registry_bind(registry, name, &::zwp_pointer_gestures_v1_interface,
                               std::min(version, 1U)),
            std::min(version, 1U)
        };
    }
#undef iface_match
    batch->known_ifaces.emplace_back(interface);
}

void registry_listener_global_remove(void *data, ::wl_registry *registry, uint32_t name)
{
    QLOG(LOG_DEBUG, "Remove interface: {}", name);
}

const ::wl_registry_listener registry_listener_ = {
        .global = registry_listener_global_add,
        .global_remove = registry_listener_global_remove
};

const ::wl_callback_listener async_roundtrip_listener_ = {
        WaylandDisplay::AsyncRoundtripCallback
};

// NOLINTNEXTLINE
const ::xdg_wm_base_listener wm_base_listener_ = {
        [](void *data, ::xdg_wm_base *wm, uint32_t serial) -> void {
            ::xdg_wm_base_pong(wm, serial);
        }
};

// NOLINTNEXTLINE
const ::zxdg_shell_v6_listener zxdg_shell_listener_ = {
        [](void *data, ::zxdg_shell_v6 *shell, uint32_t serial) -> void {
            ::zxdg_shell_v6_pong(shell, serial);
        }
};

} // namespace anonymous

Handle<Display> Display::OpenWayland(const Handle<Context>& ctx, const std::string& name)
{
    CHECK(ctx);

    ::wl_log_set_handler_client(wl_client_logger);

    /* The initialization of Wayland will surely fail if this
     * environment variable is unset, because Wayland client
     * library will find the UNIX socket file of the Wayland
     * compositor in $XDG_RUNTIME_DIR. */
    if (std::getenv("XDG_RUNTIME_DIR") == nullptr)
    {
        QLOG(LOG_ERROR, "Variable $XDG_RUNTIME_DIR is unset. Failed to connect to Wayland compositor");
        return nullptr;
    }

    ::wl_display *display = ::wl_display_connect(name.empty() ? nullptr : name.c_str());
    if (!display)
    {
        QLOG(LOG_ERROR, "Unable to connect to Wayland server");
        return nullptr;
    }

    uint32_t version = ::wl_display_get_version(display);
    QLOG(LOG_INFO, "Connected to wayland server, version {}", version);

    auto wlDisplay = std::make_shared<WaylandDisplay>(ctx, display, ::wl_display_get_fd(display));
    CHECK(wlDisplay);

    ::wl_display_set_user_data(display, wlDisplay.get());

    ::wl_registry *registry = ::wl_display_get_registry(display);
    FutureIFaceRequirement::New();
    ScopeEpilogue scope([] { FutureIFaceRequirement::Delete(); });

    ::wl_registry_add_listener(registry, &registry_listener_, wlDisplay.get());
    if (::wl_display_roundtrip(display) < 0)
        return nullptr;

    FutureIFaceRequirement::Ref().processRequests(wlDisplay.get());

    /* Wait for initializing to complete. */
    if (!wlDisplay->waitForAsynchronousRoundtrips())
        return nullptr;

    auto& batch = wlDisplay->getInterfacesBatch();
    if (std::get<1>(batch->xdg_wm_base))
    {
        wlDisplay->setShellVariant(WaylandDisplay::WaylandShellVariant::kXDGShell);
        void *base = ::wl_registry_bind(registry,
                                        std::get<1>(batch->xdg_wm_base),
                                        &::xdg_wm_base_interface,
                                        std::min(std::get<2>(batch->xdg_wm_base), 3U));
        std::get<0>(batch->xdg_wm_base) = reinterpret_cast<::xdg_wm_base*>(base);
        std::get<2>(batch->xdg_wm_base) = std::min(std::get<2>(batch->xdg_wm_base), 3U);
        ::xdg_wm_base_add_listener(std::get<0>(batch->xdg_wm_base), &wm_base_listener_, nullptr);
    }
    else if (std::get<1>(batch->zxdg_shell_v6))
    {
        wlDisplay->setShellVariant(WaylandDisplay::WaylandShellVariant::kZXDGShellV6);
        void *base = ::wl_registry_bind(registry,
                                        std::get<1>(batch->zxdg_shell_v6),
                                        &::zxdg_shell_v6_interface, 1);
        std::get<0>(batch->zxdg_shell_v6) = reinterpret_cast<::zxdg_shell_v6*>(base);
        std::get<2>(batch->zxdg_shell_v6) = 1;
        ::zxdg_shell_v6_add_listener(std::get<0>(batch->zxdg_shell_v6), &zxdg_shell_listener_, nullptr);
    }
    else
    {
        QLOG(LOG_ERROR, "The Wayland compositor does not provide any supported shell interface");
        return nullptr;
    }

    QLOG(LOG_INFO, "Wayland compositor provides {} as shell interface",
         wlDisplay->getShellVariant() == WaylandDisplay::WaylandShellVariant::kXDGShell ?
         "XDG_WM_BASE" : "ZXDG_SHELL_V6");

    return wlDisplay;
}

WaylandDisplay::WaylandDisplay(const Handle<Context>& ctx,
                               ::wl_display *display,
                               int32_t poll_fd)
    : Display(DisplayBackend::kWayland, ctx)
    , PollSource(ctx->eventLoop(), poll_fd)
    , fDisposed(false)
    , fWlDisplay(display)
    , fIFacesBatch(std::make_unique<WlInterfacesBatch>())
    , fShellVariant(WaylandShellVariant::kPending)
{
    CHECK(fWlDisplay != nullptr);
    PollSource::startPoll(UV_READABLE | UV_DISCONNECT);
}

WaylandDisplay::~WaylandDisplay()
{
    if (!fDisposed)
        this->disposeNonVirtualOverride();
}

::wl_callback *WaylandDisplay::sync()
{
    return ::wl_display_sync(fWlDisplay);
}

void WaylandDisplay::asyncRoundtrip()
{
    ::wl_callback *cb = this->sync();
    ::wl_callback_add_listener(cb, &async_roundtrip_listener_, this);
    fAsyncRoundtripList.push_back(cb);
    QLOG(LOG_DEBUG, "Async roundtrip (callback {}) registered", fmt::ptr(cb));
}

void WaylandDisplay::AsyncRoundtripCallback(void *data, wl_callback *cb, uint32_t time)
{
    WaylandDisplay *wd = WaylandDisplay::GetBareFromUserData(data);
    CHECK(wd);

    wd->fAsyncRoundtripList.remove(cb);
    ::wl_callback_destroy(cb);
    QLOG(LOG_DEBUG, "Async roundtrip (callback {}) fired", fmt::ptr(cb));
}

bool WaylandDisplay::waitForAsynchronousRoundtrips()
{
    while (!fAsyncRoundtripList.empty())
    {
        if (::wl_display_dispatch(fWlDisplay) < 0)
            return false;
    }
    return true;
}

void WaylandDisplay::createSeatFromIFacesBatch()
{
    if (fSeat)
        QLOG(LOG_WARNING, "Multiple seats are not allowed and only the last one works.");
    fSeat = std::make_shared<WaylandSeat>(shared_from_this());
}

void WaylandDisplay::flush()
{
    ::wl_display_flush(fWlDisplay);
}

void WaylandDisplay::dispose()
{
    disposeNonVirtualOverride();
}

void WaylandDisplay::disposeNonVirtualOverride()
{
    if (fDisposed)
        return;
    PollSource::stopPoll();

    fSeat.reset();
    for (::wl_callback *cb : fAsyncRoundtripList)
    {
        if (cb)
            ::wl_callback_destroy(cb);
    }

    fIFacesBatch.reset();
    ::wl_display_disconnect(fWlDisplay);
    QLOG(LOG_INFO, "Disconnected from wayland server");

    fDisposed = true;
}

KeepInLoop WaylandDisplay::pollDispatch(int status, int events)
{
}

VANILLA_NS_END
