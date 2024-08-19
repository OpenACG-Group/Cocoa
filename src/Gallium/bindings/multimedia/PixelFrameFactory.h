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

#ifndef COCOA_GALLIUM_BINDINGS_MULTIMEDIA_PIXELFRAMEFACTORY_H
#define COCOA_GALLIUM_BINDINGS_MULTIMEDIA_PIXELFRAMEFACTORY_H

#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/Interface.h"
#include "Gallium/ffi/ArrayBuffer.h"
#include "Gallium/bindings/multimedia/Types.h"
#include "Gallium/bindings/renderer/Image.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

//! TSDecl: @interface FrameMakeFromEncodedOptions
struct FrameMakeFromEncodedOptions
{
    //! @tsdocbegin
    //! Force to decode the input into Y'CbCr (YUV) format. Otherwise, always decode into
    //! RGB format. Default is false.
    //!
    //! If enabled, option `reinterpretToSRGB` will be enabled, and any other options are ignored.
    //! @tsdocend
    //! TSDecl: @property @optional forceDecodeToYCbCr: boolean
    ffi::Opt<bool> force_decode_to_ycbcr;

    //! @tsdocbegin
    //! Whatever the actual color space is, set the result's color space info (in `FrameSpecification`)
    //! to sRGB, including color primaries and gamma. It does not change the behaviour of `*ColorSpace`
    //! option: color space conversion still happens, and pixels should be in the expected color space.
    //! But the color space info in frame specification is set to sRGB.
    //! @tsdocend
    //! TSDecl: @property @optional reinterpretToSRGB: boolean
    ffi::Opt<bool> reinterpret_to_srgb;

    //! @tsdocbegin
    //! Convert the result to be linear gamma. Default is false.
    //! @tsdocend
    //! TSDecl: @property @optional toLinearGamma: boolean
    ffi::Opt<bool> to_linear_gamma;

    //! @tsdocbegin
    //! Convert the result to be sRGB color space. Default is false.
    //! @tsdocend
    //! TSDecl: @property @optional toSRGBColorSpace: boolean
    ffi::Opt<bool> to_srgb_color_space;

    //! @tsdocbegin
    //! Set the pixel format that the caller expects. The result is guaranteed to be in that format.
    //! If not set, decoder will choose the most appropriate format.
    //! @tsdocend
    //! TSDecl: @property @optional format: PixelFormat
    ffi::Opt<ffi::Enum<PixelFormat>> format;
};
//! TSDecl: @end

//! @tsdocbegin
//! Create `Frame` instances from various sources.
//! @tsdocend
//! TSDecl: @class @nonconstructible PixelFrameFactory
class PixelFrameFactory : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    //! @tsdocbegin
    //! Make a `Frame` instance whose underlying buffers are filled with pixels decoded from the
    //! given data. Input data could be encoded image formats, including JPG, PNG, Webp, GIF, etc.
    //!
    //! Throws an exception on failure.
    //! @tsdocend
    //! TSDecl: @method @static FromEncoded(data: @mem(u8), options: FrameMakeFromEncodedOptions): Frame
    static ffi::RetLocal<v8::Value> FromEncoded(const ffi::Mem<uint8_t>& data,
                                                const ffi::IFace<FrameMakeFromEncodedOptions>& options);

    //! @tsdocbegin
    //! Make a `Frame` instance by copying pixels in `image`. It is impossible to safely create a `Frame`
    //! from `renderer.Image` without copy.
    //!
    //! Detecting the accurate color space info from `Image` is hard, since it only stores a transform
    //! matrix and a gamma curve in numeric representation. So user must provide color space info manually.
    //! @tsdocend
    //! TSDecl: @method @static FromImage(image: @import(renderer) Image, colorPrimaries: ColorPrimaries,
    //! TSDecl:                           colorTrc: ColorTransferCharacteristic): Frame
    static ffi::RetLocal<v8::Value> FromImage(ffi::Class<renderer::Image> image,
                                              ffi::Enum<ColorPrimaries> color_primaries,
                                              ffi::Enum<ColorTransferCharacteristic> color_trc);

};
//! TSDecl: @end

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
#endif //COCOA_GALLIUM_BINDINGS_MULTIMEDIA_PIXELFRAMEFACTORY_H
