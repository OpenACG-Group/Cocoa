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

#ifndef COCOA_GALLIUM_BINDINGS_STREAM_STREAM_H
#define COCOA_GALLIUM_BINDINGS_STREAM_STREAM_H

#include "Gallium/bindings/stream/Types.h"
#include "Gallium/ffi/JSObject.h"
GALLIUM_BINDINGS_STREAM_NS_BEGIN

//! @tsdocbegin
//! Readable is an abstraction for a source of bytes.
//! The underlying implementation can be backed by memory, a file, or something else.
//! Stream is readonly, and for a writable stream, use `WStream` instead.
//! @tsdocend
//! TSDecl: @class @nonconstructible Stream
class Stream : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)
};
//! TSDecl: @end

GALLIUM_BINDINGS_STREAM_NS_END
#endif //COCOA_GALLIUM_BINDINGS_STREAM_STREAM_H
