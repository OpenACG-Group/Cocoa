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

#ifndef COCOA_NATIVES_LIBTESS2_SRC_EXPORTS_H
#define COCOA_NATIVES_LIBTESS2_SRC_EXPORTS_H

#include "include/v8.h"
#include "tesselator.h"

#define LIBTESS2_BEGIN_NS       namespace libtess2 {
#define LIBTESS2_END_NS         }

LIBTESS2_BEGIN_NS

//! TSDecl: class Tessellator
class TessellatorWrap
{
public:
    TessellatorWrap();
    ~TessellatorWrap();

    //! TSDecl: function addContour2D(vertices: Float32Array,
    //!                               strideInBytes: number,
    //!                               count: number): void
    void addContour2D(v8::Local<v8::Value> vertices, int32_t stride, int32_t count);

    //! TSDecl: function addContour3D(vertices: Float32Array,
    //!                               strideInBytes: number,
    //!                               count: number): void
    void addContour3D(v8::Local<v8::Value> vertices, int32_t stride, int32_t count);

    //! TSDecl: function tessellate(windingRule: number,
    //!                             elementType: number,
    //!                             polygon_size: number,
    //!                             vertexDimension: number,
    //!                             normal?: Array<number>): void
    void tessellate(int winding_rule,
                    int element_type,
                    int polygon_size,
                    int vertex_dimension,
                    v8::Local<v8::Value> normal);

    //! TSDecl: property outputVertices: Float32Array
    v8::Local<v8::Value> getOutputVertices();

private:
    void AddContour(v8::Local<v8::Value> vertices,
                    int32_t dimension,
                    int32_t stride_in_bytes,
                    int32_t count);

    TESStesselator *tess_;
    v8::Global<v8::Float32Array> output_vertices_cache_;
};

LIBTESS2_END_NS
#endif //COCOA_NATIVES_LIBTESS2_SRC_EXPORTS_H
