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

#ifndef COCOA_GLAMOR_WAYLAND_SCREENCAST_DESKTOPPORTAL_H
#define COCOA_GLAMOR_WAYLAND_SCREENCAST_DESKTOPPORTAL_H

#include <gio/gio.h>

#include "Screencast.h"
SCREENCAST_NAMESPACE_BEGIN

class BusCallbackClosure;
class Pipewire;

class DesktopPortal
{
    friend class BusCallbackClosure;

public:
    DesktopPortal();
    ~DesktopPortal();

    g_nodiscard static std::shared_ptr<DesktopPortal> Make();

    g_nodiscard g_inline GDBusConnection *GetConnection() const {
        return connection_;
    }

    g_nodiscard g_inline GDBusProxy *GetProxy() const {
        return proxy_;
    }

    g_nodiscard g_inline GCancellable *GetCancellable() const {
        return cancellable_;
    }

    g_nodiscard g_inline uint32_t GetPortalVersion() const {
        return portal_version_;
    }

    g_nodiscard g_inline uint32_t GetCursorModes() const {
        return cursor_modes_;
    }

    g_nodiscard g_inline const std::string& GetRestoreToken() const {
        return restore_token_;
    }

    g_nodiscard g_inline const std::string& GetSessionHandle() const {
        return session_handle_;
    }

    g_nodiscard g_inline uint32_t GetPipewireNode() const {
        return pipewire_node_;
    }

    g_nodiscard g_inline const std::shared_ptr<Pipewire>& GetPipewireInstance() const {
        return pipewire_instance_;
    }

    g_inline void SetPipewireNode(uint32_t node) {
        pipewire_node_ = node;
    }

    g_inline void SetRestoreToken(const std::string& token) {
        restore_token_ = token;
    }

    g_inline void SetSessionHandle(const std::string& session) {
        session_handle_ = session;
    }

    g_inline void SetPipewireInstance(const std::shared_ptr<Pipewire>& pw) {
        pipewire_instance_ = pw;
    }

private:
    GDBusConnection         *connection_;
    GDBusProxy              *proxy_;
    GCancellable            *cancellable_;
    uint32_t                 pipewire_node_;
    std::string              restore_token_;
    std::string              session_handle_;
    uint32_t                 portal_version_;
    uint32_t                 cursor_modes_;
    std::shared_ptr<Pipewire> pipewire_instance_;
};

SCREENCAST_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_SCREENCAST_DESKTOPPORTAL_H
