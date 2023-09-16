#ifndef COCOA_GLAMOR_WAYLAND_WAYLANDCURSOR_H
#define COCOA_GLAMOR_WAYLAND_WAYLANDCURSOR_H

#include <wayland-client.h>

#include "include/core/SkBitmap.h"

#include "Glamor/Cursor.h"
GLAMOR_NAMESPACE_BEGIN

class WaylandCursorTheme;
class WaylandDisplay;

class WaylandCursor : public Cursor
{
public:
    explicit WaylandCursor(const std::shared_ptr<CursorTheme>& theme,
                           wl_surface *surface)
        : Cursor(theme), surface_(surface) {}
    ~WaylandCursor() override = default;

    g_nodiscard static std::shared_ptr<WaylandCursor>
    MakeFromBitmap(const std::shared_ptr<WaylandDisplay>& display,
                   const std::shared_ptr<SkBitmap>& bitmap,
                   const SkIVector& hotspot);

    g_nodiscard g_inline wl_surface *GetCursorSurface() const {
        return surface_;
    }

private:
    wl_surface  *surface_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_WAYLANDCURSOR_H
