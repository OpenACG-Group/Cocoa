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
#ifndef COCOA_GLAMOR_WAYLAND_WAYLANDCURSORTHEME_H
#define COCOA_GLAMOR_WAYLAND_WAYLANDCURSORTHEME_H

#include <wayland-cursor.h>

#include "Glamor/Glamor.h"
#include "Glamor/CursorTheme.h"
GLAMOR_NAMESPACE_BEGIN

class WaylandDisplay;

class WaylandCursorTheme : public CursorTheme
{
public:
    WaylandCursorTheme(WaylandDisplay *display, wl_cursor_theme *theme, int size);
    ~WaylandCursorTheme() override = default;

    g_nodiscard static Shared<WaylandCursorTheme> MakeDefault(const Shared<WaylandDisplay>& display);
    g_nodiscard static Shared<WaylandCursorTheme> MakeFromName(const Shared<WaylandDisplay>& display,
                                                               const std::string& name, int size);

    void OnDispose() override;
    Shared<Cursor> OnLoadCursorFromName(const std::string &name) override;

    g_nodiscard g_inline int GetCursorSize() const {
        return cursor_size_;
    }

private:
    WaylandDisplay      *display_;
    wl_cursor_theme     *cursor_theme_;
    int                  cursor_size_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_WAYLANDCURSORTHEME_H
