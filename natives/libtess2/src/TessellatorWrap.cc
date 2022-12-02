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

#include <type_traits>

#include "Exports.h"

#include "Gallium/binder/Convert.h"
#include "Gallium/binder/ThrowExcept.h"
#include "Core/Errors.h"
#include "fmt/format.h"
LIBTESS2_BEGIN_NS

namespace binder = cocoa::gallium::binder;

TessellatorWrap::TessellatorWrap()
{
    tess_ = tessNewTess(nullptr);
    if (!tess_)
        throw std::runtime_error("Failed to initialize tessellation context");
}

TessellatorWrap::~TessellatorWrap()
{
    CHECK(tess_);
    tessDeleteTess(tess_);
}

void TessellatorWrap::addContour2D(v8::Local<v8::Value> vertices, int32_t stride, int32_t count)
{
    AddContour(vertices, 2, stride, count);
}

void TessellatorWrap::addContour3D(v8::Local<v8::Value> vertices, int32_t stride, int32_t count)
{
    AddContour(vertices, 3, stride, count);
}

void TessellatorWrap::AddContour(v8::Local<v8::Value> vertices,
                                 int32_t dimension,
                                 int32_t stride_in_bytes,
                                 int32_t count)
{
    static_assert(std::is_same<TESSreal, float>::value,
                  "TESSreal must be an alias of float");

    if (!vertices->IsFloat32Array())
        g_throw(TypeError, "`vertices` must be an instance of Float32Array");

    auto array = v8::Local<v8::Float32Array>::Cast(vertices);
    if (array->ByteLength() < stride_in_bytes * count)
        g_throw(Error, "Vertices buffer has an inappropriate size");

    tessAddContour(tess_, dimension, array->Buffer()->Data(),
                   stride_in_bytes, count);
}

void TessellatorWrap::tessellate(int winding_rule,
                                 int element_type,
                                 int polygon_size,
                                 int vertex_dimension,
                                 v8::Local<v8::Value> normal)
{
    constexpr static TessWindingRule kLastWindingRule =
            TessWindingRule::TESS_WINDING_ABS_GEQ_TWO;
    constexpr static TessElementType kLastElementType =
            TessElementType::TESS_BOUNDARY_CONTOURS;

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    // No matter whether the tessellation succeed, the previous vertices output
    // will be cleared.
    output_vertices_cache_.Reset();

    if (winding_rule > kLastWindingRule || winding_rule < 0)
        g_throw(RangeError, "Invalid enumeration value for argument `winding_rule`");

    if (element_type > kLastElementType || element_type < 0)
        g_throw(RangeError, "Invalid enumeration value for argument `element_type`");

    if (vertex_dimension != 2 && vertex_dimension != 3)
        g_throw(RangeError, "Invalid vertex dimension, must be 2 or 3");

    std::unique_ptr<float[]> normal_vf32;
    if (!normal->IsNullOrUndefined())
    {
        if (!normal->IsArray())
            g_throw(TypeError, "Argument `normal` must be an array");

        auto normal_array = v8::Local<v8::Array>::Cast(normal);
        CHECK(!normal_array.IsEmpty());

        if (normal_array->Length() != 3)
            g_throw(Error, "Argument `normal` has an invalid size (must be 3)");

        normal_vf32.reset(new float[3]);
        for (int i = 0; i < 3; i++)
        {
            v8::Local<v8::Value> c = normal_array->Get(context, i).ToLocalChecked();
            if (!c->IsNumber())
                g_throw(TypeError, "Invalid normal vector (components must be numbers)");
            normal_vf32[i] = binder::from_v8<float>(isolate, c);
        }
    }

    int result = tessTesselate(tess_, winding_rule, element_type, polygon_size,
                               vertex_dimension, normal_vf32.get());
    if (!result)
        g_throw(Error, "Failed to tessellate vertices");
}

v8::Local<v8::Value> TessellatorWrap::getOutputVertices()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    if (!output_vertices_cache_.IsEmpty())
        return output_vertices_cache_.Get(isolate);

    int vertices_count = tessGetVertexCount(tess_);
    if (vertices_count == 0)
        return v8::Null(isolate);

    // TODO(sora): implement this.
    return v8::Null(isolate);
}

LIBTESS2_END_NS
