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

#include <gio/gio.h>
#include <gio/gunixfdlist.h>

#include "fmt/format.h"

#include "Errors.h"
#include "DesktopPortal.h"
#include "Pipewire.h"
SCREENCAST_NAMESPACE_BEGIN

#define PORTAL_SERVICE      "org.freedesktop.portal.Desktop"
#define PORTAL_OBJECT_PATH  "/org/freedesktop/portal/desktop"
#define PORTAL_INTERFACE    "org.freedesktop.portal.ScreenCast"

enum CaptureType
{
    kCaptureType_Monitor    = 1 << 0,
    kCaptureType_Window     = 1 << 1,
    kCaptureType_Virtual    = 1 << 2
};

namespace {
void select_sources(DesktopPortal *portal, CaptureType type);
void start_capture(DesktopPortal *portal);
void open_pipewire_remote(DesktopPortal *portal);
} // namespace anonymous

class BusCallbackClosure
{
public:
    static BusCallbackClosure *New(DesktopPortal *portal,
                                   const std::string& path)
    {
        auto *closure = new BusCallbackClosure;
        closure->desktop_portal_ = portal;
        closure->request_path = path;
        closure->cancelled_id = g_signal_connect(portal->cancellable_,
                                                 "cancelled",
                                                 G_CALLBACK(on_cancelled),
                                                 closure);

        // This should be set later by caller.
        closure->signal_id = 0;

        return closure;
    }

    static void on_cancelled(g_maybe_unused GCancellable *cancellable, void *data)
    {
        CHECK(data);
        auto *closure = reinterpret_cast<BusCallbackClosure*>(data);

        fmt::print("[portal] Screencast session cancelled\n");

        g_dbus_connection_call(closure->desktop_portal_->GetConnection(),
                               "org.freedesktop.portal.Desktop",
                               closure->request_path.c_str(),
                               "org.freedesktop.portal.Request",
                               "Close",
                               nullptr,
                               nullptr,
                               G_DBUS_CALL_FLAGS_NONE,
                               -1,
                               nullptr,
                               nullptr,
                               nullptr);
    }

    static void on_start_response_received(g_maybe_unused GDBusConnection *connect,
                                           g_maybe_unused const char *sender_name,
                                           g_maybe_unused const char *object_path,
                                           g_maybe_unused const char *interface_name,
                                           g_maybe_unused const char *signal_name,
                                           GVariant *parameters,
                                           void *user_data)
    {
        CHECK(user_data);
        auto *closure = reinterpret_cast<BusCallbackClosure*>(user_data);
        auto portal = closure->desktop_portal_;
        // Disconnection of signals will be done in the destructor
        delete closure;

        uint32_t response;
        g_autoptr(GVariant) result = nullptr;

        g_variant_get(parameters, "(u@a{sv})", &response, &result);
        if (response != 0)
        {
            fmt::print("[portal] Failed to start screencast, denied or cancelled by user\n");
            return;
        }

        // Iterate to enumerate available streams
        g_autoptr(GVariant) streams = g_variant_lookup_value(result, "streams", G_VARIANT_TYPE_ARRAY);

        GVariantIter iter;
        g_variant_iter_init(&iter, streams);

        size_t n_streams = g_variant_iter_n_children(&iter);
        if (n_streams != 1)
        {
            fmt::print("[portal] Received more than one stream when only one was expected.\n");

            // The KDE Desktop portal implementation sometimes sends an invalid
            // response where more than one stream is attached, and only the
            // last one is the one we're looking for. This is the only known
            // buggy implementation, so let us at least try to make it work here.
            for (size_t i = 0; i < n_streams - 1; i++) {
                g_autoptr(GVariant) throwaway_properties = nullptr;
                uint32_t throwaway_pipewire_node;

                g_variant_iter_loop(&iter, "(u@a{sv})",
                                    &throwaway_pipewire_node,
                                    &throwaway_properties);
            }
        }

        uint32_t pipewire_node;
        g_autoptr(GVariant) stream_properties = nullptr;
        g_variant_iter_loop(&iter, "(u@a{sv})", &pipewire_node, &stream_properties);

        portal->SetPipewireNode(pipewire_node);

        if (portal->GetPortalVersion() >= 4)
        {
            g_autoptr(GVariant) restore_token = g_variant_lookup_value(result,
                                                                       "restore_token",
                                                                       G_VARIANT_TYPE_STRING);
            if (restore_token)
                portal->SetRestoreToken(g_variant_get_string(restore_token, nullptr));

            // TODO: do some save?
        }

        open_pipewire_remote(portal);
    }

    static void on_select_source_response_received(g_maybe_unused GDBusConnection *connect,
                                                    g_maybe_unused const char *sender_name,
                                                    g_maybe_unused const char *object_path,
                                                    g_maybe_unused const char *interface_name,
                                                    g_maybe_unused const char *signal_name,
                                                    GVariant *parameters,
                                                    void *user_data)
    {
        CHECK(user_data);
        auto *closure = reinterpret_cast<BusCallbackClosure*>(user_data);
        auto portal = closure->desktop_portal_;
        // Disconnection of signals will be done in the destructor
        delete closure;

        uint32_t response;
        g_autoptr(GVariant) result = nullptr;

        g_variant_get(parameters, "(u@a{sv})", &response, &result);
        if (response != 0)
        {
            fmt::print("[portal] Failed to select sources, denied or cancelled by user\n");
            return;
        }
        fmt::print("[portal] Sources selection has been accomplished\n");

        start_capture(portal);
    }

    static void on_create_session_response_received(g_maybe_unused GDBusConnection *connect,
                                                    g_maybe_unused const char *sender_name,
                                                    g_maybe_unused const char *object_path,
                                                    g_maybe_unused const char *interface_name,
                                                    g_maybe_unused const char *signal_name,
                                                    GVariant *parameters,
                                                    void *user_data)
    {
        CHECK(user_data);
        auto *closure = reinterpret_cast<BusCallbackClosure*>(user_data);
        auto portal = closure->desktop_portal_;
        // Disconnection of signals will be done in the destructor
        delete closure;

        uint32_t response;
        g_autoptr(GVariant) result = nullptr;

        g_variant_get(parameters, "(u@a{sv})", &response, &result);
        if (response != 0)
        {
            fmt::print("[portal] Failed to create session, denied or cancelled by user\n");
            return;
        }

        fmt::print("[portal] Screencast session created\n");

        g_autoptr(GVariant) gv = g_variant_lookup_value(result, "session_handle", nullptr);
        portal->SetSessionHandle(g_variant_dup_string(gv, nullptr));

        // After the creation of session is finished, we can select source now
        select_sources(portal, kCaptureType_Monitor);
    }

    static void on_session_created(GObject *source,
                                   GAsyncResult *res,
                                   g_maybe_unused void *user_data)
    {
        g_autoptr(GError) error = nullptr;

        g_maybe_unused
        g_autoptr(GVariant) result = g_dbus_proxy_call_finish(G_DBUS_PROXY(source), res, &error);
        if (error)
        {
            if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
            {
                fmt::print("[portal] Error creating screencast session: {}\n",
                           error->message);
            }
        }
    }

    static void on_source_selected(GObject *source,
                                   GAsyncResult *res,
                                   g_maybe_unused void *user_data)
    {
        g_autoptr(GError) error = nullptr;

        g_maybe_unused
        g_autoptr(GVariant) result = g_dbus_proxy_call_finish(G_DBUS_PROXY(source), res, &error);
        if (error)
        {
            if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
            {
                fmt::print("[portal] Error creating selecting source: {}\n",
                           error->message);
            }
        }
    }

    static void on_started(GObject *source,
                           GAsyncResult *res,
                           g_maybe_unused void *user_data)
    {
        g_autoptr(GError) error = nullptr;

        g_maybe_unused
        g_autoptr(GVariant) result = g_dbus_proxy_call_finish(G_DBUS_PROXY(source), res, &error);
        if (error)
        {
            if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
            {
                fmt::print("[portal] Error starting screencast: {}\n",
                           error->message);
            }
        }
    }

    static void on_pipewire_remote_opened(GObject *source,
                                          GAsyncResult *res,
                                          void *user_data)
    {
        CHECK(user_data);
        auto *portal = reinterpret_cast<DesktopPortal*>(user_data);

        g_autoptr(GError) error = nullptr;
        g_autoptr(GUnixFDList) fd_list = nullptr;
        g_autoptr(GVariant) result = g_dbus_proxy_call_with_unix_fd_list_finish(G_DBUS_PROXY(source),
                                                                                &fd_list,
                                                                                res,
                                                                                &error);
        if (error)
        {
            if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
                fmt::print("[portal] Error retrieving pipewire fd: {}\n", error->message);
            return;
        }

        int fd_index;
        g_variant_get(result, "(h)", &fd_index, &error);

        int pipewire_fd = g_unix_fd_list_get(fd_list, fd_index, &error);
        if (error)
        {
            if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
                fmt::print("[portal] Error retrieving pipewire fd: {}\n", error->message);
            return;
        }

        // All the things that the portal should do have been finished now, and we can
        // invoke pipewire to receive video streams.
        portal->SetPipewireInstance(Pipewire::Make(pipewire_fd, portal->GetPipewireNode()));
    }

    BusCallbackClosure() = default;

    ~BusCallbackClosure()
    {
        if (signal_id)
        {
            g_dbus_connection_signal_unsubscribe(desktop_portal_->GetConnection(),
                                                 signal_id);
        }

        if (cancelled_id)
        {
            g_signal_handler_disconnect(desktop_portal_->GetCancellable(),
                                        cancelled_id);
        }
    }

    DesktopPortal       *desktop_portal_{};
    std::string          request_path;
    guint                signal_id{};
    gulong               cancelled_id{};
};

namespace {

std::string get_connection_unique_name(GDBusConnection *c)
{
    const char *name_str = g_dbus_connection_get_unique_name(c);
    CHECK(name_str);

    // To skip the first ':'
    std::string result(name_str + 1);

    // Replace dots by underscores
    auto p = result.find_first_of('.');
    while (p < result.length())
    {
        result[p] = '_';
        p = result.find_first_of('.', p + 1);
    }

    return result;
}

using StringPair = std::pair<std::string, std::string>;

StringPair create_request_path(GDBusConnection *c)
{
    static uint32_t request_token_count = 0;
    request_token_count++;

    auto request_token = fmt::format("CocoaScreencast{}", request_token_count);
    auto request_path = fmt::format("/org/freedesktop/portal/desktop/request/{}/CocoaScreencast{}",
                                    get_connection_unique_name(c),
                                    request_token_count);

    return {request_token, request_path};
}

StringPair create_session_path(GDBusConnection *c)
{
    static uint32_t session_token_count = 0;
    session_token_count++;

    auto session_token = fmt::format("CocoaScreencast{}", session_token_count);
    auto session_path = fmt::format("/org/freedesktop/portal/desktop/session/{}/CocoaScreencast{}",
                                    get_connection_unique_name(c),
                                    session_token_count);

    return {session_token, session_path};
}

BusCallbackClosure *subscribe_to_signal(DesktopPortal *portal,
                                        const std::string& path,
                                        GDBusSignalCallback callback)
{
    auto *closure = BusCallbackClosure::New(portal, path);

    closure->signal_id = g_dbus_connection_signal_subscribe(portal->GetConnection(),
                                                            "org.freedesktop.portal.Desktop",
                                                            "org.freedesktop.portal.Request",
                                                            "Response",
                                                            path.c_str(),
                                                            nullptr,
                                                            G_DBUS_SIGNAL_FLAGS_NO_MATCH_RULE,
                                                            callback,
                                                            closure,
                                                            nullptr);

    return closure;
}

void create_session(const std::shared_ptr<DesktopPortal>& portal)
{
    GDBusConnection *connection = portal->GetConnection();

    auto [request_token, request_path] = create_request_path(connection);
    auto session_token = create_session_path(connection).first;

    BusCallbackClosure *call = subscribe_to_signal(portal.get(), request_path,
                                                   BusCallbackClosure::on_create_session_response_received);


    GVariantBuilder gv_builder;

    g_variant_builder_init(&gv_builder, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&gv_builder, "{sv}", "handle_token",
                          g_variant_new_string(request_token.c_str()));
    g_variant_builder_add(&gv_builder, "{sv}", "session_handle_token",
                          g_variant_new_string(session_token.c_str()));

    g_dbus_proxy_call(portal->GetProxy(),
                      "CreateSession",
                      g_variant_new("(a{sv})", &gv_builder),
                      G_DBUS_CALL_FLAGS_NONE,
                      -1,
                      portal->GetCancellable(),
                      BusCallbackClosure::on_session_created,
                      call);
}

enum CursorMode
{
    kCursorMode_Hidden   = 1 << 0,
    kCursorMode_Embedded = 1 << 1,
    kCursorMode_Metadata = 1 << 2
};

void select_sources(DesktopPortal *portal, CaptureType type)
{
    auto [request_token, request_path] = create_request_path(portal->GetConnection());

    BusCallbackClosure *call = subscribe_to_signal(portal,
                                                   request_path,
                                                   BusCallbackClosure::on_select_source_response_received);

    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&builder, "{sv}", "types", g_variant_new_uint32(type));
    g_variant_builder_add(&builder, "{sv}", "multiple", g_variant_new_boolean(false));
    g_variant_builder_add(&builder, "{sv}", "handle_token", g_variant_new_string(request_token.c_str()));

    uint32_t available_cursor_modes = portal->GetCursorModes();
    for (CursorMode mode : {kCursorMode_Metadata, kCursorMode_Embedded, kCursorMode_Hidden})
    {
        if (!(available_cursor_modes & mode))
            continue;
        g_variant_builder_add(&builder, "{sv}", "cursor_mode",
                              g_variant_new_uint32(mode));
        break;
    }

    if (portal->GetPortalVersion() >= 4)
    {
        g_variant_builder_add(&builder, "{sv}", "persist_mode", g_variant_new_uint32(2));
        if (!portal->GetRestoreToken().empty())
        {
            g_variant_builder_add(&builder, "{sv}", "restore_token",
                                  g_variant_new_string(portal->GetRestoreToken().c_str()));
        }
    }

    g_dbus_proxy_call(portal->GetProxy(),
                      "SelectSources",
                      g_variant_new("(oa{sv})",
                                    portal->GetSessionHandle().c_str(),
                                    &builder),
                      G_DBUS_CALL_FLAGS_NONE,
                      -1,
                      portal->GetCancellable(),
                      BusCallbackClosure::on_source_selected,
                      call);
}

void start_capture(DesktopPortal *portal)
{
    auto [request_token, request_path] = create_request_path(portal->GetConnection());

    BusCallbackClosure *call = subscribe_to_signal(portal,
                                                   request_path,
                                                   BusCallbackClosure::on_start_response_received);

    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&builder, "{sv}", "handle_token", g_variant_new_string(request_token.c_str()));

    g_dbus_proxy_call(portal->GetProxy(),
                      "Start",
                      g_variant_new("(osa{sv})",
                                    portal->GetSessionHandle().c_str(),
                                    "",
                                    &builder),
                      G_DBUS_CALL_FLAGS_NONE,
                      -1,
                      portal->GetCancellable(),
                      BusCallbackClosure::on_started,
                      call);
}

void open_pipewire_remote(DesktopPortal *portal)
{
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);

    g_dbus_proxy_call_with_unix_fd_list(portal->GetProxy(),
                                        "OpenPipeWireRemote",
                                        g_variant_new("(oa{sv})",
                                                      portal->GetSessionHandle().c_str(),
                                                      &builder),
                                        G_DBUS_CALL_FLAGS_NONE,
                                        -1,
                                        nullptr,
                                        portal->GetCancellable(),
                                        BusCallbackClosure::on_pipewire_remote_opened,
                                        portal);
}

} // namespace anonymous

std::shared_ptr<DesktopPortal> DesktopPortal::Make()
{
    // Try to connect to session bus
    g_autoptr(GError) error = nullptr;
    GDBusConnection *connection = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error);
    if (error)
    {
        fmt::print("[portal] Error retrieving DBus connection: {}\n", error->message);
        return nullptr;
    }

    // Get a proxy object of desktop portal interface on the bus
    GDBusProxy *proxy = g_dbus_proxy_new_sync(connection,
                                              G_DBUS_PROXY_FLAGS_NONE,
                                              nullptr,
                                              PORTAL_SERVICE,
                                              PORTAL_OBJECT_PATH,
                                              PORTAL_INTERFACE,
                                              nullptr,
                                              &error);
    if (error)
    {
        fmt::print("[portal] Failed to create a proxy object: {}\n", error->message);
        fmt::print("[portal]   service={}, object_path={}, interface={}\n",
                   PORTAL_SERVICE, PORTAL_OBJECT_PATH, PORTAL_INTERFACE);
        return nullptr;
    }

    auto desktop_portal = std::make_shared<DesktopPortal>();
    desktop_portal->connection_ = connection;
    desktop_portal->proxy_ = proxy;
    desktop_portal->cancellable_ = g_cancellable_new();

    // Query screencast version
    g_autoptr(GVariant) cached_version = g_dbus_proxy_get_cached_property(proxy, "version");
    desktop_portal->portal_version_ = cached_version ? g_variant_get_uint32(cached_version) : 0;

    // Query mode cursor modes
    g_autoptr(GVariant) cached_cursor_modes = g_dbus_proxy_get_cached_property(proxy, "AvailableCursorModes");
    desktop_portal->cursor_modes_ = cached_cursor_modes ? g_variant_get_uint32(cached_cursor_modes) : 0;

    // Select source type (display sharing or window sharing)
    g_autoptr(GVariant) cached_source_types = g_dbus_proxy_get_cached_property(proxy, "AvailableSourceTypes");;
    uint32_t available_source_types = cached_source_types ? g_variant_get_uint32(cached_source_types) : 0;

    fmt::print("[portal] supported source types mask: {}\n", available_source_types);

    create_session(desktop_portal);

    return desktop_portal;
}

DesktopPortal::DesktopPortal()
    : connection_(nullptr)
    , proxy_(nullptr)
    , cancellable_(nullptr)
    , pipewire_node_(0)
    , portal_version_(0)
    , cursor_modes_(0)
{
}

DesktopPortal::~DesktopPortal()
{
    CHECK(pipewire_instance_.unique());
    pipewire_instance_.reset();

    // FIXME: free proxy object
    if (connection_)
        g_dbus_connection_close_sync(connection_, nullptr, nullptr);
}

SCREENCAST_NAMESPACE_END
