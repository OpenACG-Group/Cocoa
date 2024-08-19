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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_PIXMAP_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_PIXMAP_H

#include "include/core/SkPixmap.h"

#include "Gallium/bindings/renderer/Types.h"
#include "Gallium/bindings/renderer/ImageInfo.h"
#include "Gallium/bindings/renderer/Rect.h"
#include "Gallium/bindings/renderer/SamplingOptions.h"
#include "Gallium/ffi/ReturnValue.h"
#include "Gallium/ffi/ArrayBuffer.h"
#include "Gallium/ffi/Interface.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

//! TSDecl: @interface Pixmap
struct Pixmap
{
    SkPixmap ToSkPixmap() const {
        return { (*image_info)->GetImageInfo(), pixels.Address(), row_bytes };
    }

    //! TSDecl: @property @readonly pixels: @mem(u8)
    ffi::Mem<uint8_t> pixels;

    //! TSDecl: @property @readonly rowBytes: u64
    size_t row_bytes;

    //! TSDecl: @property @readonly imageInfo: ImageInfo
    ffi::Class<ImageInfo> image_info;
};
//! TSDecl: @end

class PixmapAdapter : public ffi::ArgAdapter
{
public:
    static ffi::Ret<PixmapAdapter> Cast(v8::Isolate *isolate, v8::Local<v8::Value> value);

    SkPixmap& operator*() {
        return pixmap;
    }

    SkPixmap *operator->() {
        return &pixmap;
    }

    ffi::Mem<uint8_t> memory;
    SkPixmap pixmap;
};

//! TSDecl: @class @nonconstructible PixmapUtils
class PixmapUtils : ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    //! TSDecl: @method @static ComputeIsOpaque(src: Pixmap): boolean
    static ffi::Ret<bool> ComputeIsOpaque(PixmapAdapter src);

    //! TSDecl: @method @static Copy(src: Pixmap, dst: Pixmap, srcX: i32, srcY: i32): boolean
    static ffi::Ret<bool> Copy(PixmapAdapter src, PixmapAdapter dst, int32_t src_x, int32_t src_y);

    //! TSDecl: @method @static EraseSubset(dst: Pixmap, color: u32, subset: Rect): boolean
    static ffi::Ret<bool> EraseSubset(PixmapAdapter dst, uint32_t color, RectAdapter subset);

    //! TSDecl: @method @static Erase(color: u32): boolean
    static ffi::Ret<bool> Erase(PixmapAdapter dst, uint32_t color);

    //! TSDecl: @method @static Scale(src: Pixmap, dst: Pixmap, sampling: SamplingOptions): boolean
    static ffi::Ret<bool> Scale(PixmapAdapter src, PixmapAdapter dst, SamplingOptionsAdapter sampling);

    //! TSDecl: @method @static PickColor(src: Pixmap, x: i32, y: i32): u32
    static ffi::Ret<uint32_t> PickColor(PixmapAdapter src, int32_t x, int32_t y);
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_PIXMAP_H
