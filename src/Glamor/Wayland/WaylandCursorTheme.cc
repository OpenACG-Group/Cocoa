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

#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Glamor/Wayland/WaylandCursorTheme.h"
#include "Glamor/Wayland/WaylandDisplay.h"
#include "Glamor/Wayland/WaylandSystemCursor.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Wayland.CursorTheme)

Shared<WaylandCursorTheme> WaylandCursorTheme::MakeDefault(const Shared<WaylandDisplay>& disp)
{
    CHECK(disp && "Invalid WaylandDisplay pointer");

    // Get cursor settings from environment variable
    const char *env_cursor_theme = getenv(ENV_GL_XCURSOR_THEME);
    const char *env_cursor_size = getenv(ENV_GL_XCURSOR_SIZE);

    if (!env_cursor_theme)
    {
        QLOG(LOG_ERROR, "Failed to load default cursor theme, missing XCURSOR_THEME.");
        return nullptr;
    }

    // A default cursor size
    int cursor_size = 32;

    if (env_cursor_size)
    {
        errno = 0;
        char *end_ptr;

        // This conversion may cause so many errors that it needs
        // a strict check.
        long size_in_long = strtol(env_cursor_size, &end_ptr, 10);
        if (*end_ptr == '\0' && !errno && size_in_long > 0)
            cursor_size = static_cast<int>(size_in_long);
    }

    return MakeFromName(disp, env_cursor_theme, cursor_size);
}

Shared<WaylandCursorTheme> WaylandCursorTheme::MakeFromName(const Shared<WaylandDisplay>& display,
                                                            const std::string& name, int size)
{
    wl_cursor_theme *theme = wl_cursor_theme_load(name.c_str(), size,
                                                  display->GetGlobalsRef()->wl_shm_);
    if (!theme)
    {
        QLOG(LOG_ERROR, "Unable to load default cursor theme \"{}\" with size {}", name, size);
        return nullptr;
    }

    return std::make_shared<WaylandCursorTheme>(display.get(), theme, size);
}

WaylandCursorTheme::WaylandCursorTheme(cocoa::glamor::WaylandDisplay *display,
                                       wl_cursor_theme *theme, int size)
    : display_(display)
    , cursor_theme_(theme)
    , cursor_size_(size)
{
    CHECK(display && theme);
}

void WaylandCursorTheme::OnDispose()
{
    wl_cursor_theme_destroy(cursor_theme_);
}

Shared<Cursor> WaylandCursorTheme::OnLoadCursorFromName(const std::string& name)
{
    wl_cursor *cursor = wl_cursor_theme_get_cursor(cursor_theme_, name.c_str());
    if (!cursor)
    {
        QLOG(LOG_ERROR, "Failed to load a cursor named \"{}\" from theme", name);
        return nullptr;
    }

    wl_surface *surface = wl_compositor_create_surface(
            display_->GetGlobalsRef()->wl_compositor_);
    if (!surface)
    {
        QLOG(LOG_ERROR, "Failed to create a surface for cursor from compositor");
        return nullptr;
    }

    auto system_cursor = std::make_shared<WaylandSystemCursor>(Self()->As<WaylandCursorTheme>(),
                                                               cursor, surface);
    CHECK(system_cursor);
    system_cursor->PrepareCursorSurfaceAndAnimation();

    return system_cursor;
}

GLAMOR_NAMESPACE_END
