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

#ifndef COCOA_GALLIUM_BINDINGS_PIXENCODER_EXPORTS_H
#define COCOA_GALLIUM_BINDINGS_PIXENCODER_EXPORTS_H

#include "include/v8.h"

#define GALLIUM_BINDINGS_PIXENCODER_NS_BEGIN    namespace cocoa::gallium::bindings::pixencoder_wrap {
#define GALLIUM_BINDINGS_PIXENCODER_NS_END      }

GALLIUM_BINDINGS_PIXENCODER_NS_BEGIN

void SetInstanceProperties(v8::Local<v8::Object> instance);

class PNGEncoder
{
public:
    //! TSDecl:
    //! interface PNGEncoderOptions {
    //!   filterFlags?: Bitfield<PNGEncoderFilterFlags>;
    //!   zlibLevel?: number;
    //!   comments?: Array<Uint8Array>;
    //! }

    //! TSDecl: function EncodeImage(img: glamor.CkImage,
    //!                              options: PNGEncoderOptions): ArrayBuffer | null
    static v8::Local<v8::Value> EncodeImage(v8::Local<v8::Value> img,
                                            v8::Local<v8::Value> options);

    //! TSDecl: function EncodeMemory(info: glamor.CkImageInfo,
    //!                               pixels: Uint8Array,
    //!                               rowBytes: number,
    //!                               options: PNGEncoderOptions): ArrayBuffer | null
    static v8::Local<v8::Value> EncodeMemory(v8::Local<v8::Value> info,
                                             v8::Local<v8::Value> pixels,
                                             int64_t row_bytes,
                                             v8::Local<v8::Value> options);
};

class JPEGEncoder
{
public:
    //! TSDecl:
    //! interface JPEGEncoderOptions {
    //!   quality?: number;
    //!   downsample?: Enum<JPEGEncoderDownsample>;
    //!   alphaOption?: Enum<JPEGEncoderAlphaOption>;
    //!   xmpMetadata?: Uint8Array;
    //! }

    //! TSDecl: function EncodeImage(img: glamor.CkImage,
    //!                              options: JPEGEncoderOptions): ArrayBuffer | null
    static v8::Local<v8::Value> EncodeImage(v8::Local<v8::Value> img,
                                            v8::Local<v8::Value> options);

    //! TSDecl: function EncodeMemory(info: glamor.CkImageInfo,
    //!                               pixels: Uint8Array,
    //!                               rowBytes: number,
    //!                               options: JPEGEncoderOptions): ArrayBuffer | null
    static v8::Local<v8::Value> EncodeMemory(v8::Local<v8::Value> info,
                                             v8::Local<v8::Value> pixels,
                                             int64_t row_bytes,
                                             v8::Local<v8::Value> options);
};

class WebpEncoder
{
public:
    //! TSDecl:
    //! interface WebpEncoderOptions {
    //!   compression?: Enum<WebpEncoderCompression>;
    //!   quality?: number;
    //! }

    //! TSDecl: function EncodeImage(img: glamor.CkImage,
    //!                              options: WebpEncoderOptions): ArrayBuffer | null
    static v8::Local<v8::Value> EncodeImage(v8::Local<v8::Value> img,
                                            v8::Local<v8::Value> options);

    //! TSDecl: function EncodeMemory(info: glamor.CkImageInfo,
    //!                               pixels: Uint8Array,
    //!                               rowBytes: number,
    //!                               options: WebpEncoderOptions): ArrayBuffer | null
    static v8::Local<v8::Value> EncodeMemory(v8::Local<v8::Value> info,
                                             v8::Local<v8::Value> pixels,
                                             int64_t row_bytes,
                                             v8::Local<v8::Value> options);

    //! TSDecl:
    //! interface WebpImageFrame {
    //!   image: glamor.CkImage;
    //!   duration: number;
    //! }

    //! TSDecl:
    //! interface WebpMemoryFrame {
    //!   info: glamor.CkImageInfo;
    //!   pixels: Uint8Array;
    //!   rowBytes: number;
    //!   duration: number;
    //! }

    //! TSDecl: function EncodeAnimatedImage(frames: Array<WebpImageFrame>,
    //!                                      options: WebpEncoderOptions): ArrayBuffer | null
    static v8::Local<v8::Value> EncodeAnimatedImage(v8::Local<v8::Value> frames,
                                                    v8::Local<v8::Value> options);

    //! TSDecl: function EncodeAnimatedMemory(frames: Array<WebpMemoryFrame>,
    //!                                       options: WebpEncoderOptions): ArrayBuffer | null
    static v8::Local<v8::Value> EncodeAnimatedMemory(v8::Local<v8::Value> frames,
                                                     v8::Local<v8::Value> options);
};

GALLIUM_BINDINGS_PIXENCODER_NS_END
#endif // COCOA_GALLIUM_BINDINGS_PIXENCODER_EXPORTS_H
