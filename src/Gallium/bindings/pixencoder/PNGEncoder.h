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

#ifndef COCOA_GALLIUM_BINDINGS_PIXENCODER_PNGENCODER_H
#define COCOA_GALLIUM_BINDINGS_PIXENCODER_PNGENCODER_H

#include "include/encode/SkPngEncoder.h"

#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/ArrayBuffer.h"
#include "Gallium/ffi/Class.h"
#include "Gallium/ffi/Interface.h"
#include "Gallium/bindings/pixencoder/Types.h"
#include "Gallium/bindings/renderer/GpuDirectContext.h"
#include "Gallium/bindings/renderer/Image.h"
#include "Gallium/bindings/renderer/Pixmap.h"
GALLIUM_BINDINGS_PIXENCODER_NS_BEGIN

//! TSDecl: @interface PNGEncoderOptions
struct PNGEncoderOptions
{
    //! TSDecl: @property @optional filterFlags: u32
    ffi::Opt<uint32_t> filter_flags;

    //! TSDecl: @property @optional zlibLevel: i32
    ffi::Opt<int32_t> zlib_level;

    //! TSDecl: @property @optional comments: @array(@mem(u8))
    ffi::Opt<v8::Local<v8::Array>> comments;
};
//! TSDecl: @end

struct PNGEncoderOptionsAdapter : public ffi::ArgAdapter
{
    static ffi::Ret<PNGEncoderOptionsAdapter> Cast(
            v8::Isolate *isolate, v8::Local<v8::Value> value);

    const SkPngEncoder::Options& operator*() const {
        return options;
    }

    SkPngEncoder::Options options;
};

//! TSDecl: @class @nonconstructible PNGEncoder
class PNGEncoder : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    //! TSDecl: @method @static EncodeImage(ctx: @union(@import(renderer) GpuDirectContext, null),
    //! TSDecl:                             image: @import(renderer) Image,
    //! TSDecl:                             options: PNGEncoderOptions): @mem(raw)
    static ffi::RetLocal<v8::Value> EncodeImage(ffi::Opt<ffi::Class<renderer::GpuDirectContext>> ctx,
                                                ffi::Class<renderer::Image> image,
                                                const PNGEncoderOptionsAdapter& options);

    //! TSDecl: @method @static EncodePixmap(pixmap: @import(renderer) Pixmap,
    //! TSDecl:                              options: PNGEncoderOptions): @mem(raw)
    static ffi::RetLocal<v8::Value> EncodePixmap(renderer::PixmapAdapter pixmap,
                                                 const PNGEncoderOptionsAdapter& options);
};
//! TSDecl: @end

GALLIUM_BINDINGS_PIXENCODER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_PIXENCODER_PNGENCODER_H
