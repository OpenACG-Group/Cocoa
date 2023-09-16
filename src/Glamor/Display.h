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

#ifndef COCOA_GLAMOR_DISPLAY_H
#define COCOA_GLAMOR_DISPLAY_H

#include <list>

#include "uv.h"
#include "Glamor/Glamor.h"
#include "Glamor/RenderTarget.h"
#include "Glamor/PresentRemoteHandle.h"
#include "Glamor/GraphicsResourcesTrackable.h"

#include "include/core/SkColor.h"
#include "include/core/SkBitmap.h"
GLAMOR_NAMESPACE_BEGIN

#define GLOP_DISPLAY_CLOSE                      1
#define GLOP_DISPLAY_CREATE_RASTER_SURFACE      2
#define GLOP_DISPLAY_CREATE_HW_COMPOSE_SURFACE  3
#define GLOP_DISPLAY_REQUEST_MONITOR_LIST       4
#define GLOP_DISPLAY_CREATE_CURSOR              5
#define GLOP_DISPLAY_LOAD_CURSOR_THEME          6

#define GLSI_DISPLAY_CLOSED     1
#define GLSI_DISPLAY_MONITOR_ADDED      2
#define GLSI_DISPLAY_MONITOR_REMOVED    3

class Surface;
class Monitor;
class CursorTheme;
class Cursor;

class Display : public PresentRemoteHandle,
                public GraphicsResourcesTrackable
{
public:
    using MonitorList = std::list<std::shared_ptr<Monitor>>;

    static std::shared_ptr<Display> Connect(uv_loop_t *loop, const std::string& name);

    explicit Display(uv_loop_t *eventLoop);
    ~Display() override;

    g_nodiscard g_inline uv_loop_t *GetEventLoop() const {
        return event_loop_;
    }

    g_nodiscard g_inline const std::list<std::shared_ptr<Surface>>&
    GetSurfacesList() const {
        return surfaces_list_;
    }

    g_nodiscard g_inline virtual std::vector<SkColorType> GetRasterColorFormats() = 0;

    g_async_api void Close();

    g_async_api MonitorList RequestMonitorList();
    g_async_api std::shared_ptr<Surface> CreateRasterSurface(int32_t width, int32_t height, SkColorType format);
    g_async_api std::shared_ptr<Surface> CreateHWComposeSurface(int32_t width, int32_t height, SkColorType format);

    g_async_api std::shared_ptr<Cursor> CreateCursor(const std::shared_ptr<SkBitmap>& bitmap,
                                                     int32_t hotspot_x, int32_t hotspot_y);
    g_async_api std::shared_ptr<CursorTheme> LoadCursorTheme(const std::string& name, int size);

    g_private_api void RemoveSurfaceFromList(const std::shared_ptr<Surface>& s);

    g_sync_api std::shared_ptr<CursorTheme> GetDefaultCursorTheme() const {
        CHECK(!cursor_themes_list_.empty());
        return cursor_themes_list_.front();
    }

    void Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept override;

protected:
    virtual std::shared_ptr<Surface> OnCreateSurface(
            int32_t width, int32_t height, SkColorType format,
            RenderTarget::RenderDevice device) = 0;

    virtual std::shared_ptr<Cursor> OnCreateCursor(
            const std::shared_ptr<SkBitmap>& bitmap,
            int32_t hotspot_x, int32_t hotspot_y);

    virtual std::shared_ptr<CursorTheme> OnLoadCursorTheme(
            const std::string& name, int size);

    virtual void OnDispose() = 0;

    void AppendSurface(const std::shared_ptr<Surface>& surface);

    void AppendMonitor(const std::shared_ptr<Monitor>& monitor);
    bool RemoveMonitor(const std::shared_ptr<Monitor>& monitor);

    void AppendDefaultCursorTheme(const std::shared_ptr<CursorTheme>& theme);

    uv_loop_t                               *event_loop_;
    bool                                     has_disposed_;
    std::list<std::shared_ptr<Monitor>>      monitors_list_;
    std::list<std::shared_ptr<Surface>>      surfaces_list_;

    // Only stores cursor objects that were "created" by application,
    // not loaded from cursor themes. Cursors that were loaded from
    // cursor themes are stored in `CursorTheme` objects.
    std::list<std::shared_ptr<Cursor>>       created_cursors_list_;
    std::list<std::shared_ptr<CursorTheme>>  cursor_themes_list_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_DISPLAY_H
