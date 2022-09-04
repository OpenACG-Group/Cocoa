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

#ifndef COCOA_GLAMOR_WAYLAND_WAYLANDDISPLAY_H
#define COCOA_GLAMOR_WAYLAND_WAYLANDDISPLAY_H

#include <map>

#include <wayland-client-protocol.h>
#include <wayland-cursor.h>

#include "Glamor/Display.h"
#include "Glamor/Wayland/protos/xdg-shell-client-protocol.h"
#include "Glamor/Wayland/protos/xdg-decoration-unstable-protocol.h"
#include "Glamor/Wayland/protos/kde-server-decoration-protocol.h"
GLAMOR_NAMESPACE_BEGIN

class WaylandRoundtripScope;
class WaylandSeat;
class WaylandSurface;

class WaylandDisplay : public Display
{
public:
    friend class WaylandRoundtripScope;

    struct Globals
    {
        ~Globals();

        wl_compositor   *wl_compositor_{nullptr};
        xdg_wm_base     *xdg_wm_base_{nullptr};
        wl_shm          *wl_shm_{nullptr};
        zxdg_decoration_manager_v1      *zxdg_deco_manager{nullptr};
        org_kde_kwin_server_decoration_manager *kde_deco_manager{nullptr};
    };

    static g_inline WaylandDisplay *BareCast(void *data) {
        return reinterpret_cast<WaylandDisplay*>(data);
    }

    static Shared<WaylandDisplay> Connect(uv_loop_t *loop, const std::string& name);

    WaylandDisplay(uv_loop_t *loop, int fd);
    ~WaylandDisplay() override;

    g_nodiscard g_inline auto& GetGlobalsIdMap() {
        return globals_id_map_;
    }

    g_nodiscard g_inline const Unique<Globals>& GetGlobalsRef() {
        return globals_;
    }

    g_nodiscard g_inline wl_display *GetWaylandDisplay() {
        return wl_display_;
    }

    g_inline void AppendSeat(const Shared<WaylandSeat>& seat) {
        CHECK(seat && "Invalid seat");

        auto itr = std::find(seats_list_.begin(), seats_list_.end(), seat);
        if (itr == seats_list_.end())
            seats_list_.push_back(seat);
    }

    bool HasPointerDeviceInSeats();

    g_nodiscard Shared<WaylandSurface> GetPointerEnteredSurface(wl_pointer *pointer);

    /**
     * Try to remove a seat specified by the wayland global ID.
     * If the seat cannot be found in the existing seats list, return false;
     * If the seat is removed successfully, return true.
     */
    bool TryRemoveSeat(uint32_t id);

    std::vector<SkColorType> GetRasterColorFormats() override;

    static void RegistryHandleGlobal(void *data, wl_registry *registry, uint32_t id,
                                     const char *interface, uint32_t version);
    static void RegistryHandleGlobalRemove(void *data, wl_registry *registry, uint32_t name);
    static void ShmFormatHandler(void *data, wl_shm *shm, uint32_t format);

    void Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept override;

private:
    Shared<Surface> OnCreateSurface(int32_t width, int32_t height, SkColorType format,
                                    RenderTarget::RenderDevice device) override;
    Shared<Cursor> OnCreateCursor(const Shared<SkBitmap> &bitmap,
                                  int32_t hotspot_x, int32_t hotspot_y) override;
    Shared<CursorTheme> OnLoadCursorTheme(const std::string &name, int size) override;
    void OnDispose() override;

    static void PrepareCallback(uv_prepare_t *prepare);
    static void CheckCallback(uv_check_t *check);
    static void PollCallback(uv_poll_t *poll, int status, int events);

    wl_display                     *wl_display_;
    wl_registry                    *wl_registry_;
    std::map<uint32_t, std::string> globals_id_map_;

    std::unique_ptr<Globals>        globals_;
    std::vector<wl_shm_format>      wl_shm_formats_;

    std::list<Shared<WaylandSeat>>  seats_list_;

    uv_prepare_t                   *uv_prepare_handle_;
    uv_check_t                     *uv_check_handle_;
    uv_poll_t                      *uv_poll_handle_;
    bool                            display_is_reading_;
};

class WaylandRoundtripScope
{
public:
    explicit WaylandRoundtripScope(Shared<WaylandDisplay> display);
    ~WaylandRoundtripScope();

private:
    Shared<WaylandDisplay>   display_;
    bool                    changed_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_WAYLANDDISPLAY_H
