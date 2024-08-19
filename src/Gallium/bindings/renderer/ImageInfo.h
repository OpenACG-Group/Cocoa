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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_IMAGEINFO_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_IMAGEINFO_H

#include <utility>

#include "include/core/SkImageInfo.h"

#include "Gallium/bindings/renderer/Types.h"
#include "Gallium/bindings/renderer/Color.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/Class.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

//! TSDecl: @function ColorTypeBytesPerPixel(ct: ColorType): i32
ffi::Ret<int32_t> ColorTypeBytesPerPixel(ffi::Enum<SkColorType> ct);

//! TSDecl: @function ColorTypeIsAlwaysOpaque(ct: ColorType): boolean
ffi::Ret<bool> ColorTypeIsAlwaysOpaque(ffi::Enum<SkColorType> ct);

//! TSDecl: @class @nonconstructible ImageInfo
class ImageInfo : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit ImageInfo(SkImageInfo info) : image_info_(std::move(info)) {}

    const SkImageInfo& GetImageInfo() const {
        return image_info_;
    }

    //! TSDecl: @method @static Make(width: i32, height: i32,
    //! TSDecl:                      colorType: ColorType, alphaType: AlphaType,
    //! TSDecl:                      colorSpace: @union(ColorSpace, null)): ImageInfo
    static ffi::RetLocal<v8::Object> Make(int32_t width, int32_t height,
                                          ffi::Enum<SkColorType> color_type,
                                          ffi::Enum<SkAlphaType> alpha_type,
                                          ffi::Opt<ffi::Class<ColorSpace>> color_space);

    //! TSDecl: @method @static MakeN32(width: i32, height: i32,
    //! TSDecl:                         alphaType: AlphaType,
    //! TSDecl:                         colorSpace: @union(ColorSpace, null)): ImageInfo
    static ffi::RetLocal<v8::Object> MakeN32(int32_t width, int32_t height,
                                             ffi::Enum<SkAlphaType> alpha_type,
                                             ffi::Opt<ffi::Class<ColorSpace>> color_space);

    //! TSDecl: @method @static MakeS32(width: i32, height: i32,
    //! TSDecl:                         alphaType: AlphaType): ImageInfo
    static ffi::RetLocal<v8::Object> MakeS32(int32_t width, int32_t height,
                                             ffi::Enum<SkAlphaType> alpha_type);

    //! TSDecl: @method @static MakeN32Premul(width: i32, height: i32,
    //! TSDecl:                               colorSpace: @union(ColorSpace, null)): ImageInfo
    static ffi::RetLocal<v8::Object> MakeN32Premul(int32_t width, int32_t height,
                                                   ffi::Opt<ffi::Class<ColorSpace>> color_space);

    //! TSDecl: @method @static MakeA8(width: i32, height: i32): ImageInfo
    static ffi::RetLocal<v8::Object> MakeA8(int32_t width, int32_t height);

    //! TSDecl: @method @static MakeUnknown(width: i32, height: i32): ImageInfo
    static ffi::RetLocal<v8::Object> MakeUnknown(int32_t width, int32_t height);


    //! TSDecl: @property @readonly width: i32
    ffi::Ret<int32_t> getWidth() {
        return image_info_.width();
    }

    //! TSDecl: @property @readonly height: i32
    ffi::Ret<int32_t> getHeight() {
        return image_info_.height();
    }

    //! TSDecl: @property @readonly colorType: ColorType
    ffi::Ret<ffi::Enum<SkColorType>> getColorType() {
        return image_info_.colorType();
    }

    //! TSDecl: @property @readonly alphaType: AlphaType
    ffi::Ret<ffi::Enum<SkAlphaType>> getAlphaType() {
        return image_info_.alphaType();
    }

    //! TSDecl: @method isEmpty(): boolean
    ffi::Ret<bool> isEmpty() {
        return image_info_.isEmpty();
    }

    //! TSDecl: @method isOpaque(): boolean
    ffi::Ret<bool> isOpaque() {
        return image_info_.isOpaque();
    }

    //! TSDecl: @method refColorSpace(): @union(ColorSpace, null)
    ffi::RetLocal<v8::Value> refColorSpace();

    //! TSDecl: @method gammaCloseToSRGB(): boolean
    ffi::Ret<bool> gammaCloseToSRGB() {
        return image_info_.gammaCloseToSRGB();
    }

    //! TSDecl: @method makeWH(width: i32, height: i32): ImageInfo
    ffi::RetLocal<v8::Object> makeWH(int32_t width, int32_t height);

    //! TSDecl: @method makeAlphaType(alphaType: AlphaType): ImageInfo
    ffi::RetLocal<v8::Object> makeAlphaType(ffi::Enum<SkAlphaType> alpha_type);

    //! TSDecl: @method makeColorType(colorType: ColorType): ImageInfo
    ffi::RetLocal<v8::Object> makeColorType(ffi::Enum<SkColorType> color_type);

    //! TSDecl: @method makeColorSpace(cs: @union(ColorSpace, null)): ImageInfo
    ffi::RetLocal<v8::Object> makeColorSpace(ffi::Opt<ffi::Class<ColorSpace>> cs);

    //! TSDecl: @property @readonly bytesPerPixel: i32
    ffi::Ret<int32_t> getBytesPerPixel() {
        return image_info_.bytesPerPixel();
    }

    //! TSDecl: @property @readonly shiftPerPixel: i32
    ffi::Ret<int32_t> getShiftPerPixel() {
        return image_info_.shiftPerPixel();
    }

    //! TSDecl: @property @readonly minRowBytes: u64
    ffi::Ret<size_t> getMinRowBytes() {
        return image_info_.minRowBytes();
    }

    //! TSDecl: @method computeOffset(x: i32, y: i32, rowBytes: u64): u64
    ffi::Ret<size_t> computeOffset(int32_t x, int32_t y, size_t row_bytes);

    //! TSDecl: @method equalsTo(other: ImageInfo): boolean
    ffi::Ret<bool> equalsTo(ffi::Class<ImageInfo> other);

    //! TSDecl: @method computeByteSize(rowBytes: u64): u64
    ffi::Ret<size_t> computeByteSize(size_t row_bytes);

    //! TSDecl: @method computeMinByteSize(): u64
    ffi::Ret<size_t> computeMinByteSize();

    //! TSDecl: @method validRowBytes(rowBytes: u64): boolean
    ffi::Ret<bool> validRowBytes(size_t row_bytes);

    //! TSDecl: @method reset(): void
    ffi::Ret<void> reset();

private:
    SkImageInfo image_info_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_IMAGEINFO_H
