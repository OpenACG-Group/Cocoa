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

#ifndef COCOA_GALLIUM_BINDINGS_UTILS_TEXTCODEC_H
#define COCOA_GALLIUM_BINDINGS_UTILS_TEXTCODEC_H

#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/ReturnValue.h"
#include "Gallium/ffi/ArrayBuffer.h"
#include "Gallium/bindings/utils/Types.h"
GALLIUM_BINDINGS_UTILS_NS_BEGIN

//! TSDecl: @enum TextCodec
enum class TextCodec
{
    //! TSDecl: @enumitem Latin1
    kLatin1,

    //! TSDecl: @enumitem UTF8
    kUTF8,

    //! TSDecl: @enumitem UCS2
    kUCS2,

    //! TSDecl: @enumitem Hex
    kHex
};
//! TSDecl: @end

//! TSDecl: @function computeTextByteSize(text: string, codec: TextCodec): u64
ffi::Ret<uint64_t> computeTextByteSize(v8::Local<v8::String> text,
                                       const ffi::Enum<TextCodec>& codec);

//! TSDecl: @function encodeText(text: string, codec: TextCodec): @mem(u8)
ffi::RetLocal<v8::Value> encodeText(v8::Local<v8::String> text,
                                    const ffi::Enum<TextCodec>& codec);

//! TSDecl: @interface EncodeTextIntoResult
//! TSDecl: @property readChars: u32
//! TSDecl: @property writtenBytes: u32
//! TSDecl: @end

//! TSDecl: @function encodeTextInto(text: string, codec: TextCodec,
//! TSDecl:                          dst: @mem(u8)): EncodeTextIntoResult
ffi::RetLocal<v8::Value> encodeTextInto(v8::Local<v8::String> text,
                                        const ffi::Enum<TextCodec>& codec,
                                        const ffi::Mem<uint8_t>& dst);

//! TSDecl: @function decodeText(buffer: @mem(u8), codec: TextCodec): string
ffi::RetLocal<v8::Value> decodeText(const ffi::Mem<uint8_t>& buffer,
                                    const ffi::Enum<TextCodec>& codec);

GALLIUM_BINDINGS_UTILS_NS_END
#endif //COCOA_GALLIUM_BINDINGS_UTILS_TEXTCODEC_H
