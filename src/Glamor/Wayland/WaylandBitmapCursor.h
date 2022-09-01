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

#ifndef COCOA_GLAMOR_WAYLAND_WAYLANDBITMAPCURSOR_H
#define COCOA_GLAMOR_WAYLAND_WAYLANDBITMAPCURSOR_H

#include <wayland-client.h>

#include "Glamor/Wayland/WaylandCursor.h"
GLAMOR_NAMESPACE_BEGIN

class WaylandSharedMemoryHelper;

class WaylandBitmapCursor : public WaylandCursor
{
public:
    WaylandBitmapCursor(Shared<WaylandSharedMemoryHelper> helper,
                        wl_buffer *buffer,
                        wl_surface *surface,
                        const SkIVector& hotspot);
    ~WaylandBitmapCursor() override = default;

    void OnDispose() override;
    SkIVector OnGetHotspotVector() override;
    bool OnHasAnimation() override;
    void OnTryAbortAnimation() override;
    void OnTryStartAnimation() override;

private:
    Shared<WaylandSharedMemoryHelper> shm_pool_helper_;
    wl_buffer               *bitmap_buffer_;
    wl_surface              *surface_;
    SkIVector                hotspot_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_WAYLANDBITMAPCURSOR_H
