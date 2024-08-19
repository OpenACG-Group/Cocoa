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

#ifndef COCOA_GALLIUM_BINDINGS_PIXENCODER_WEBPENCODER_H
#define COCOA_GALLIUM_BINDINGS_PIXENCODER_WEBPENCODER_H

#include "include/encode/SkWebpEncoder.h"

#include "Gallium/ffi/DataTypes.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/Class.h"
#include "Gallium/ffi/Interface.h"
#include "Gallium/bindings/pixencoder/Types.h"
#include "Gallium/bindings/renderer/Pixmap.h"
#include "Gallium/bindings/renderer/GpuDirectContext.h"
#include "Gallium/bindings/renderer/Image.h"
GALLIUM_BINDINGS_PIXENCODER_NS_BEGIN

//! TSDecl: @interface WebpEncoderOptions
struct WebpEncoderOptions
{
    SkWebpEncoder::Options Make() const;

    //! TSDecl: @property @optional quality: f32
    ffi::Opt<float> quality;

    //! TSDecl: @property @optional compression: WebpCompression
    ffi::Opt<ffi::Enum<SkWebpEncoder::Compression>> compression;
};
//! TSDecl: @end

//! TSDecl: @interface WebpFrame
struct WebpFrame
{
    ffi::Ret<SkEncoder::Frame> Make(v8::Isolate *isolate) const;

    //! TSDecl: @property pixmap: @import(renderer) Pixmap
    v8::Local<v8::Value> pixmap;

    //! TSDecl: @property duration: i32
    int32_t duration;
};
//! TSDecl: @end

//! TSDecl: @class @nonconstructible WebpEncoder
class WebpEncoder : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    //! TSDecl: @method @static EncodeImage(ctx: @union(@import(renderer) GpuDirectContext, null),
    //! TSDecl:                             image: @import(renderer) Image,
    //! TSDecl:                             options: WebpEncoderOptions): @mem(raw)
    static ffi::RetLocal<v8::Value> EncodeImage(ffi::Opt<ffi::Class<renderer::GpuDirectContext>> ctx,
                                                ffi::Class<renderer::Image> image,
                                                ffi::IFace<WebpEncoderOptions> options);

    //! TSDecl: @method @static EncodePixmap(pixmap: @import(renderer) Pixmap,
    //! TSDecl:                              options: WebpEncoderOptions): @mem(raw)
    static ffi::RetLocal<v8::Value> EncodePixmap(renderer::PixmapAdapter pixmap,
                                                 ffi::IFace<WebpEncoderOptions> options);

    //! TSDecl: @method @static EncodeAnimated(frames: @array(WebpFrame),
    //! TSDecl:                                options: WebpEncoderOptions): @mem(raw)
    static ffi::RetLocal<v8::Value> EncodeAnimated(v8::Local<v8::Array> frames,
                                                   ffi::IFace<WebpEncoderOptions> options);
};
//! TSDecl: @end

GALLIUM_BINDINGS_PIXENCODER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_PIXENCODER_WEBPENCODER_H
