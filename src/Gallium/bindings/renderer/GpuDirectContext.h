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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_GPUDIRECTCONTEXT_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_GPUDIRECTCONTEXT_H

#include "include/gpu/GrDirectContext.h"

#include "Gallium/ffi/JSObject.h"
#include "Gallium/bindings/renderer/Types.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

//! TSDecl: @class @nonconstructible GpuDirectContext
class GpuDirectContext : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    const sk_sp<GrDirectContext>& GetGrDirectContext() const {
        return context_;
    }

    //! TSDecl: @method dispose(): void
    ffi::Ret<void> dispose();

private:
    sk_sp<GrDirectContext> context_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_GPUDIRECTCONTEXT_H
