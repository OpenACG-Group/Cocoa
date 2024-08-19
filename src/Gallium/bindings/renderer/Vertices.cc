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

#include "Gallium/bindings/renderer/Vertices.h"
#include "Gallium/bindings/renderer/Rect.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

ffi::RetLocal<v8::Value> Vertices::MakeCopy(const ffi::Enum<SkVertices::VertexMode>& mode,
                                            const ffi::Mem<float>& positions,
                                            const ffi::Opt<ffi::Mem<float>>& tex_coords,
                                            const ffi::Opt<ffi::Mem<uint32_t>>& colors,
                                            const ffi::Opt<ffi::Mem<uint16_t>>& indices)
{
    if (positions.Size() & 1)
        return ffi::Fail(ffi::kErr, "invalid size of `positions` argument");
    int vertex_count = static_cast<int>(positions.Size() >> 1);

    if (tex_coords && tex_coords->Size() != positions.Size())
        return ffi::Fail(ffi::kErr, "size of texCoords does not match the vertices count");
    if (colors && colors->Size() != vertex_count)
        return ffi::Fail(ffi::kErr, "size of colors does not match the vertices count");

    sk_sp<SkVertices> vertices = SkVertices::MakeCopy(
        *mode, vertex_count, reinterpret_cast<SkPoint*>(positions.Address()),
        tex_coords ? reinterpret_cast<SkPoint*>(tex_coords->Address()) : nullptr,
        colors ? colors->Address() : nullptr,
        indices ? static_cast<int>(indices->Size()) : 0,
        indices ? indices->Address() : nullptr);
    CHECK(vertices);

    return ffi::JSObject::New<Vertices>(v8::Isolate::GetCurrent(), vertices);
}

ffi::RetLocal<v8::Value> Vertices::get_bounds()
{
    return CreateJSRect(v8::Isolate::GetCurrent(), vertices_->bounds());
}

GALLIUM_BINDINGS_RENDERER_NS_END
