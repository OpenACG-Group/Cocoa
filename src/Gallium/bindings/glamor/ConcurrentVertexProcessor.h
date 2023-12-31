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

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_CONCURRENTVERTEXPROCESSOR_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_CONCURRENTVERTEXPROCESSOR_H

#include <stack>
#include <vector>
#include <memory>

#include "include/v8.h"
#include "include/core/SkMatrix.h"

#include "Core/Project.h"
#include "Gallium/bindings/ExportableObjectBase.h"
#include "Gallium/bindings/glamor/Types.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

//! TSDecl: class VertexBatch
class VertexBatch : public ExportableObjectBase
{
public:
    struct VertexGroup
    {
        VertexGroup(VertexGroup&& other) noexcept
            : positions(std::move(other.positions))
            , tex_coords(std::move(other.tex_coords))
            , pos_mat_id(other.pos_mat_id)
            , uv_mat_id(other.uv_mat_id) {}

        VertexGroup() = default;

        v8::Global<v8::Float32Array> positions;
        v8::Global<v8::Float32Array> tex_coords;
        int32_t pos_mat_id = -1;
        int32_t uv_mat_id = -1;
    };

    VertexBatch(std::vector<SkMatrix> matrix_store, std::vector<VertexGroup> groups)
        : matrix_store_(std::move(matrix_store))
        , vertex_groups_(std::move(groups)) {}

    g_nodiscard g_inline std::vector<SkMatrix>& GetMatrixStore() {
        return matrix_store_;
    }

    g_nodiscard g_inline std::vector<VertexGroup>& GetVertexGroups() {
        return vertex_groups_;
    }

private:
    std::vector<SkMatrix> matrix_store_;
    std::vector<VertexGroup> vertex_groups_;
};

//! TSDecl: class VertexBatchBuilder
class VertexBatchBuilder : public ExportableObjectBase
{
public:
    //! TSDecl: constructor()
    VertexBatchBuilder() = default;
    ~VertexBatchBuilder() = default;

    //! TSDecl: function pushPositionMatrix(matrix: CkMat3x3): VertexBatchBuilder
    v8::Local<v8::Value> pushPositionMatrix(v8::Local<v8::Value> matrix);

    //! TSDecl: function pushTexCoordMatrix(matrix: CkMat3x3): VertexBatchBuilder
    v8::Local<v8::Value> pushTexCoordMatrix(v8::Local<v8::Value> matrix);

    //! TSDecl: function popPositionMatrix(): VertexBatchBuilder
    v8::Local<v8::Value> popPositionMatrix();

    //! TSDecl: function popTexCoordMatrix(): VertexBatchBuilder
    v8::Local<v8::Value> popTexCoordMatrix();

    //! TSDecl: function addVertexGroup(positions: Float32Array,
    //!                                 texCoords: Float32Array | null): VertexBatchBuilder
    v8::Local<v8::Value> addVertexGroup(v8::Local<v8::Value> positions,
                                        v8::Local<v8::Value> texCoords);

    //! TSDecl: function build(): VertexBatch
    v8::Local<v8::Value> build();

private:
    v8::Local<v8::Object> GetSelf(v8::Isolate *isolate);

    v8::Global<v8::Object> self_;
    std::stack<int32_t> pos_matrix_stack_;
    std::stack<int32_t> uvs_matrix_stack_;
    std::vector<SkMatrix> matrix_store_;
    std::vector<VertexBatch::VertexGroup> groups_;
};


//! TSDecl: class ConcurrentVertexProcessor
class ConcurrentVertexProcessor : public ExportableObjectBase
{
public:
    constexpr static uint32_t kMinPreallocateVertexCount = 32;

    //! TSDecl: constructor(vertexCountHint: number, uvCountHint: number)
    ConcurrentVertexProcessor(uint32_t vertexCountHint, uint32_t uvCountHint);
    ~ConcurrentVertexProcessor() = default;

    //! TSDecl:
    //! interface TransformResultGroup {
    //!   positions: Float32Array;
    //!   texCoords: Float32Array | null;
    //! }

    //! TSDecl: function transform(batch: VertexBatch): Promise<Array<TransformResultGroup>>
    v8::Local<v8::Value> transform(v8::Local<v8::Value> batch);

private:
    void TryReallocateOutputBuffers(uint32_t vert_count, uint32_t uv_count);

    std::shared_ptr<v8::BackingStore> out_vertex_buffer_;
    std::shared_ptr<v8::BackingStore> out_uv_buffer_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_CONCURRENTVERTEXPROCESSOR_H
