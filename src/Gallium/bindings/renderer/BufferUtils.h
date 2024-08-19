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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_BUFFERUTILS_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_BUFFERUTILS_H

#include "include/core/SkData.h"
#include "include/v8-typed-array.h"

#include "Gallium/bindings/renderer/Types.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

enum MemoryFlag
{
    kNone_MemoryFlag = 0,

    // This allows us to share the memory of ArrayBuffer
    // instead of copying its contents, avoiding the possible overheads.
    // If set, the ArrayBuffer must NOT be resizable by user JavaScript.
    kNoCopy_MemoryFlag = 0x01,

    // If set, the original ArrayBuffer is detached when the function
    // returns. If ArrayBuffer is not detachable, or fails to detach,
    // for instance, the [[ArrayBufferDetachKey]] check fails, the function
    // will return nullptr.
    // Note that a `detach_key` should be provided from arguments.
    kDetachBuffer_MemoryFlag = 0x02
};

sk_sp<SkData> WrapArrayBufferToSkData(v8::Isolate *isolate,
                                      v8::Local<v8::ArrayBufferView> view,
                                      uint32_t memory_flag,
                                      v8::Local<v8::Value> detach_key = {});

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_BUFFERUTILS_H
