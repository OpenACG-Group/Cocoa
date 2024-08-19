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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_VERTICES_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_VERTICES_H

#include "include/core/SkVertices.h"

#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/ArrayBuffer.h"
#include "Gallium/bindings/renderer/Types.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

//! TSDecl: @class @nonconstructible Vertices
class Vertices : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit Vertices(sk_sp<SkVertices> vertices) : vertices_(std::move(vertices)) {}
    ~Vertices() override = default;

    g_nodiscard sk_sp<SkVertices> GetSkVertices() const {
        return vertices_;
    }

    //! TSDecl: @method @static MakeCopy(mode: VertexMode,
    //! TSDecl:                          positions: @mem(f32),
    //! TSDecl:                          texCoords: @union(@mem(f32), null),
    //! TSDecl:                          colors: @union(@mem(u32), null),
    //! TSDecl:                          indices: @union(@mem(u16), null)): Vertices
    static ffi::RetLocal<v8::Value> MakeCopy(const ffi::Enum<SkVertices::VertexMode>& mode,
                                             const ffi::Mem<float>& positions,
                                             const ffi::Opt<ffi::Mem<float>>& tex_coords,
                                             const ffi::Opt<ffi::Mem<uint32_t>>& colors,
                                             const ffi::Opt<ffi::Mem<uint16_t>>& indices);

    //! TSDecl: @property @readonly uniqueID: u32
    ffi::Ret<uint32_t> get_uniqueID() {
        return vertices_->uniqueID();
    }

    //! TSDecl: @property @readonly approximateSize: u64
    ffi::Ret<size_t> get_approximateSize() {
        return vertices_->approximateSize();
    }

    //! TSDecl: @property @readonly bounds: Rect
    ffi::RetLocal<v8::Value> get_bounds();

private:
    sk_sp<SkVertices> vertices_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_VERTICES_H
