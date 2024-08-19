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

#include "fmt/format.h"

#include "Gallium/bindings/renderer/ImageAsyncReadResult.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

ffi::Ret<void> ImageAsyncReadResult::dispose()
{
    result_.reset();
    NotifyDisposeState(DisposeState::kDisposed);
    return {};
}

#define CHECK_PLANE_INDEX(plane) \
    if (plane < 0 || plane >= plane_vec_.size()) \
    { \
        return ffi::Fail(ffi::kRangeErr, fmt::format( \
            "plane index is not of range [0, {})", plane_vec_.size())); \
    }

ffi::Ret<std::tuple<int32_t, int32_t>>
ImageAsyncReadResult::dimensionsOf(int32_t plane)
{
    CHECK_PLANE_INDEX(plane)
    Plane& p = plane_vec_[plane];
    return std::make_tuple(p.size.width(), p.size.height());
}

ffi::Ret<int32_t> ImageAsyncReadResult::bytesPerPixelOf(int32_t plane)
{
    CHECK_PLANE_INDEX(plane)
    Plane& p = plane_vec_[plane];
    return p.bytes_per_pixel;
}

ffi::Ret<void> ImageAsyncReadResult::readPlane(int32_t plane,
                                               const ffi::Mem<uint8_t>& dst,
                                               int32_t dst_row_bytes,
                                               RectAdapter src_rect_adapter)
{
    CHECK_PLANE_INDEX(plane)
    const Plane& plane_ref = plane_vec_[plane];

    SkIRect src_rect = (*src_rect_adapter).round();
    if (!SkIRect::MakeSize(plane_ref.size).contains(src_rect))
        return ffi::Fail(ffi::kRangeErr, "srcRect is out of bounds");

    size_t dst_min_byte_size = dst_row_bytes * src_rect.height();
    if (dst.ByteSize() < dst_min_byte_size)
        return ffi::Fail(ffi::kErr, "dst buffer is incompatible with the rowBytes and dimensions");

    size_t src_row_bytes = result_->rowBytes(plane);
    int bytes_per_pixel = plane_ref.bytes_per_pixel;
    const uint8_t *src_copy_start_addr = static_cast<const uint8_t*>(result_->data(plane))
                                       + src_row_bytes * src_rect.y()
                                       + bytes_per_pixel * src_rect.x();

    for (uint32_t line = 0; line < src_rect.height(); line++)
    {
        const uint8_t *src_addr = src_copy_start_addr + line * src_row_bytes;
        uint8_t *dst_addr = dst.Address() + line * dst_row_bytes;
        std::memcpy(dst_addr, src_addr, src_rect.width() * bytes_per_pixel);
    }
    return {};
}

GALLIUM_BINDINGS_RENDERER_NS_END
