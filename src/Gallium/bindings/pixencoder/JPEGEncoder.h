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

#ifndef COCOA_GALLIUM_BINDINGS_PIXENCODER_JPEGENCODER_H
#define COCOA_GALLIUM_BINDINGS_PIXENCODER_JPEGENCODER_H

#include "include/encode/SkJpegEncoder.h"

#include "Gallium/ffi/DataTypes.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/ArrayBuffer.h"
#include "Gallium/ffi/Interface.h"
#include "Gallium/ffi/Class.h"
#include "Gallium/bindings/pixencoder/Types.h"
#include "Gallium/bindings/renderer/Pixmap.h"
#include "Gallium/bindings/renderer/GpuDirectContext.h"
#include "Gallium/bindings/renderer/Image.h"
GALLIUM_BINDINGS_PIXENCODER_NS_BEGIN

//! TSDecl: @interface JPEGEncoderOptions
struct JPEGEncoderOptions
{
    std::tuple<SkJpegEncoder::Options, sk_sp<SkData>> Make() const;

    //! TSDecl: @property @optional quality: i32
    ffi::Opt<int32_t> quality;

    //! TSDecl: @property @optional downsample: JPEGDownsample
    ffi::Opt<ffi::Enum<SkJpegEncoder::Downsample>> downsample;

    //! TSDecl: @property @optional alphaOption: JPEGAlphaOption
    ffi::Opt<ffi::Enum<SkJpegEncoder::AlphaOption>> alpha_option;

    //! TSDecl: @property @optional xmpMetadata: @mem(u8)
    ffi::Opt<ffi::Mem<uint8_t>> xmp_metadata;
};
//! TSDecl: @end

//! TSDecl: @class @nonconstructible JPEGEncoder
class JPEGEncoder : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    //! TSDecl: @method @static EncodeImage(ctx: @union(@import(renderer) GpuDirectContext, null),
    //! TSDecl:                             image: @import(renderer) Image,
    //! TSDecl:                             options: JPEGEncoderOptions): @mem(raw)
    static ffi::RetLocal<v8::Value> EncodeImage(ffi::Opt<ffi::Class<renderer::GpuDirectContext>> ctx,
                                                ffi::Class<renderer::Image> image,
                                                ffi::IFace<JPEGEncoderOptions> options);

    //! TSDecl: @method @static EncodePixmap(pixmap: @import(renderer) Pixmap,
    //! TSDecl:                              options: JPEGEncoderOptions): @mem(raw)
    static ffi::RetLocal<v8::Value> EncodePixmap(renderer::PixmapAdapter pixmap,
                                                 ffi::IFace<JPEGEncoderOptions> options);
};
//! TSDecl: @end

GALLIUM_BINDINGS_PIXENCODER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_PIXENCODER_JPEGENCODER_H
