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

#ifndef COCOA_GLAMOR_WAYLAND_WAYLANDSYSTEMCURSOR_H
#define COCOA_GLAMOR_WAYLAND_WAYLANDSYSTEMCURSOR_H

#include <memory>

#include <wayland-client.h>
#include "uv.h"

// Wayland cursor is not a part of wayland protocol.
// It is a separated library which helps to deal with the loading
// and management of cursor images.
#include <wayland-cursor.h>

#include "Glamor/Glamor.h"
#include "Glamor/Cursor.h"
#include "Glamor/Wayland/WaylandCursor.h"
GLAMOR_NAMESPACE_BEGIN

class WaylandDisplay;
class WaylandCursorTheme;

class WaylandSystemCursor : public WaylandCursor
{
public:
    WaylandSystemCursor(const std::shared_ptr<CursorTheme>& theme,
                        wl_cursor *cursor,
                        wl_surface *cursor_surface);
    ~WaylandSystemCursor() override;

    g_private_api void PrepareCursorSurfaceAndAnimation();

    void OnDispose() override;
    SkIVector OnGetHotspotVector() override;

    bool OnHasAnimation() override;
    void OnTryAbortAnimation() override;
    void OnTryStartAnimation() override;

private:
    static void on_animation_timer(uv_timer_t *handle);

    wl_surface              *cursor_surface_;

    wl_cursor               *current_cursor_;
    int                      current_cursor_image_idx_;
    uv_timer_t               animation_timer_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_WAYLANDSYSTEMCURSOR_H
