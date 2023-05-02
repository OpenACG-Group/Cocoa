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

#include "include/v8.h"

#include "Gallium/bindings/glamor/CkVerticesWrap.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

v8::Local<v8::Value> CkVertices::MakeCopy(int32_t mode, v8::Local<v8::Value> positions,
                                          v8::Local<v8::Value> texCoords,
                                          v8::Local<v8::Value> colors,
                                          v8::Local<v8::Value> indices)
{
    static_assert(std::is_pod<SkPoint>::value && sizeof(SkPoint) == 2 * sizeof(float));
    static_assert(std::is_same<float, decltype(SkPoint::fX)>::value);
    static_assert(std::is_same<SkColor, uint32_t>::value);

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (mode < 0 || mode > static_cast<int32_t>(SkVertices::kLast_VertexMode))
        g_throw(RangeError, "Invalid enumeration value for argument `mode`");

    if (!positions->IsFloat32Array())
        g_throw(TypeError, "Argument `positions` must be a Float32Array");

    auto pos_f32arr = v8::Local<v8::Float32Array>::Cast(positions);
    if (pos_f32arr->Length() == 0)
        g_throw(Error, "Empty vertices buffer");
    if (pos_f32arr->Length() & 1)
        g_throw(Error, "Length of `positions` cannot be interpreted as vertices");

    int vert_count = static_cast<int>(pos_f32arr->Length()) >> 1;
    auto *pos_addr = reinterpret_cast<SkPoint*>(static_cast<uint8_t*>(pos_f32arr->Buffer()->Data())
                                                + pos_f32arr->ByteOffset());
    CHECK(pos_addr);

    SkPoint *tex_coords_addr = nullptr;
    if (!texCoords->IsNullOrUndefined())
    {
        if (!texCoords->IsFloat32Array())
            g_throw(TypeError, "Argument `texCoords` must be a Float32Array or null");
        auto texCoords_f32arr = v8::Local<v8::Float32Array>::Cast(texCoords);
        if (texCoords_f32arr->Length() != (vert_count << 1))
            g_throw(Error, "Length of `texCoords` does not match the number of vertices");
        tex_coords_addr = reinterpret_cast<SkPoint*>(static_cast<uint8_t*>(texCoords_f32arr->Buffer()->Data())
                                                     + texCoords_f32arr->ByteOffset());
        CHECK(tex_coords_addr);
    }

    SkColor *colors_addr = nullptr;
    if (!colors->IsNullOrUndefined())
    {
        if (!colors->IsUint32Array())
            g_throw(TypeError, "Argument `colors` must be a Uint32Array or null");
        auto colors_u32array = v8::Local<v8::Uint32Array>::Cast(colors);
        if (colors_u32array->Length() != vert_count)
            g_throw(Error, "Length of `colors` does not match the number of vertices");
        colors_addr = reinterpret_cast<SkColor*>(static_cast<uint8_t*>(colors_u32array->Buffer()->Data())
                                                 + colors_u32array->ByteOffset());
    }

    int index_count = 0;
    uint16_t *indices_addr = nullptr;
    if (!indices->IsNullOrUndefined())
    {
        if (!indices->IsUint16Array())
            g_throw(TypeError, "Argument `indices` must be a Uint16Array or null");
        auto indices_u16array = v8::Local<v8::Uint16Array>::Cast(indices);
        if (indices_u16array->Length() != 0)
        {
            index_count = static_cast<int>(indices_u16array->Length());
            indices_addr = reinterpret_cast<uint16_t*>(
                    static_cast<uint8_t*>(indices_u16array->Buffer()->Data())
                    + indices_u16array->ByteOffset());
            CHECK(indices_addr);
        }
    }

    sk_sp<SkVertices> result = SkVertices::MakeCopy(static_cast<SkVertices::VertexMode>(mode),
                                                    vert_count,
                                                    pos_addr,
                                                    tex_coords_addr,
                                                    colors_addr,
                                                    index_count,
                                                    indices_addr);
    if (!result)
        return v8::Null(isolate);

    return binder::Class<CkVertices>::create_object(isolate, result);
}

CkVertices::CkVertices(const sk_sp<SkVertices>& vertices)
    : SkiaObjectWrapper(vertices)
{
    approximate_size_bytes_ =
            static_cast<ssize_t>(vertices->approximateSize());
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    isolate->AdjustAmountOfExternalAllocatedMemory(approximate_size_bytes_);
}

CkVertices::~CkVertices()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (isolate)
        isolate->AdjustAmountOfExternalAllocatedMemory(-approximate_size_bytes_);
}

v8::Local<v8::Value> CkVertices::getBounds()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return NewCkRect(isolate, GetSkObject()->bounds());
}

GALLIUM_BINDINGS_GLAMOR_NS_END
