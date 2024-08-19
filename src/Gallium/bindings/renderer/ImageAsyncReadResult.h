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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_IMAGEASYNCREADRESULT_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_IMAGEASYNCREADRESULT_H

#include <vector>

#include "include/core/SkImage.h"

#include "Gallium/bindings/renderer/Types.h"
#include "Gallium/bindings/renderer/Rect.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/ArrayBuffer.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

//! TSDecl: @class @nonconstructible ImageAsyncReadResult
class ImageAsyncReadResult : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    struct Plane
    {
        SkISize size;
        int bytes_per_pixel;
    };
    using PlaneVec = std::vector<Plane>;

    explicit ImageAsyncReadResult(std::unique_ptr<const SkImage::AsyncReadResult> result,
                                  PlaneVec planes)
        : result_(std::move(result)), plane_vec_(std::move(planes)) {}

    //! TSDecl: @method dispose(): void
    ffi::Ret<void> dispose();

    //! TSDecl: @property @readonly count: i32
    ffi::Ret<int> getCount() {
        return result_->count();
    }

    //! TSDecl: @method dimensionsOf(plane: i32): @tuple(i32, i32)
    ffi::Ret<std::tuple<int32_t, int32_t>> dimensionsOf(int32_t plane);

    //! TSDecl: @method bytesPerPixelOf(plane: i32): i32
    ffi::Ret<int32_t> bytesPerPixelOf(int32_t plane);

    //! TSDecl: @method readPlane(plane: i32, dst: @mem(u8), dstRowBytes: i32, srcRect: Rect): void
    ffi::Ret<void> readPlane(int32_t plane, const ffi::Mem<uint8_t>& dst,
                             int32_t dst_row_bytes, RectAdapter src_rect);

private:
    std::unique_ptr<const SkImage::AsyncReadResult> result_;
    PlaneVec plane_vec_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_IMAGEASYNCREADRESULT_H
