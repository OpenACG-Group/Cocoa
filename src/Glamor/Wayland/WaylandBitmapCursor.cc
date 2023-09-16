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

#include <wayland-client.h>

#include <utility>

#include "include/core/SkColor.h"
#include "include/core/SkBitmap.h"

#include "Core/Journal.h"
#include "Glamor/Wayland/WaylandBitmapCursor.h"
#include "Glamor/Wayland/WaylandDisplay.h"
#include "Glamor/Wayland/WaylandSharedMemoryHelper.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.Wayland.BitmapCursor)

namespace {

// To make sure that the uploaded bitmap is in `WL_SHM_FORMAT_ARGB8888` format.
std::shared_ptr<SkBitmap>
convert_to_argb_format_bitmap(const std::shared_ptr<SkBitmap>& src)
{
    SkImageInfo dst_info = SkImageInfo::Make(src->width(),
                                             src->height(),
                                             SkColorType::kBGRA_8888_SkColorType,
                                             SkAlphaType::kUnpremul_SkAlphaType);

    if (src->info() == dst_info)
        return src;

    auto dst = std::make_shared<SkBitmap>();
    dst->allocPixels(dst_info, dst_info.minRowBytes());
    dst->writePixels(src->pixmap(), 0, 0);

    return dst;
}

} // namespace anonymous

std::shared_ptr<WaylandCursor>
WaylandCursor::MakeFromBitmap(const std::shared_ptr<WaylandDisplay>& display,
                              const std::shared_ptr<SkBitmap>& origin_bitmap,
                              const SkIVector& hotspot)
{
    CHECK(display && origin_bitmap);

    std::shared_ptr<SkBitmap> bitmap = convert_to_argb_format_bitmap(origin_bitmap);
    size_t pixels_byte_size = bitmap->computeByteSize();

    wl_shm *shm = display->GetGlobalsRef()->wl_shm_;
    auto pool_helper = WaylandSharedMemoryHelper::Make(shm, pixels_byte_size,
                                                       WaylandSharedMemoryHelper::kCursorSurface_Role);
    if (!pool_helper)
    {
        QLOG(LOG_ERROR, "Failed to upload cursor image: invalid shared memory pool");
        return nullptr;
    }

    // Upload pixels into shared memory buffer
    std::memcpy(pool_helper->GetMappedAddress(), bitmap->getPixels(), pixels_byte_size);

    wl_buffer *buffer = wl_shm_pool_create_buffer(pool_helper->GetShmPool(),
                                                  0,
                                                  bitmap->width(),
                                                  bitmap->height(),
                                                  static_cast<int32_t>(bitmap->rowBytes()),
                                                  WL_SHM_FORMAT_ARGB8888);

    if (!buffer)
    {
        QLOG(LOG_ERROR, "Failed to request a buffer from shm pool");
        return nullptr;
    }

    wl_compositor *compositor = display->GetGlobalsRef()->wl_compositor_;
    wl_surface *surface = wl_compositor_create_surface(compositor);
    if (!surface)
    {
        QLOG(LOG_ERROR, "Failed to create a cursor surface");
        return nullptr;
    }

    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_damage(surface, 0, 0, bitmap->width(), bitmap->height());
    wl_surface_commit(surface);

    return std::make_shared<WaylandBitmapCursor>(pool_helper, buffer, surface, hotspot);
}

WaylandBitmapCursor::WaylandBitmapCursor(std::shared_ptr<WaylandSharedMemoryHelper> helper,
                                         wl_buffer *buffer,
                                         wl_surface *surface,
                                         const SkIVector& hotspot)
    : WaylandCursor(nullptr, surface)
    , shm_pool_helper_(std::move(helper))
    , bitmap_buffer_(buffer)
    , surface_(surface)
    , hotspot_(hotspot)
{
}

void WaylandBitmapCursor::OnDispose()
{
    wl_surface_destroy(surface_);
    wl_buffer_destroy(bitmap_buffer_);
    shm_pool_helper_.reset();
}

bool WaylandBitmapCursor::OnHasAnimation()
{
    return false;
}

SkIVector WaylandBitmapCursor::OnGetHotspotVector()
{
    return hotspot_;
}

void WaylandBitmapCursor::OnTryStartAnimation()
{
}

void WaylandBitmapCursor::OnTryAbortAnimation()
{
}

GLAMOR_NAMESPACE_END
