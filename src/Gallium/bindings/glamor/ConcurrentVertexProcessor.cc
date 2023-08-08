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

#include "fmt/format.h"

#include "Core/Errors.h"
#include "Core/EventLoop.h"
#include "Core/TraceEvent.h"
#include "Gallium/bindings/glamor/ConcurrentVertexProcessor.h"
#include "Gallium/bindings/glamor/CkMatrixWrap.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

namespace {

CkMatrix *unwrap_matrix(v8::Isolate *isolate, v8::Local<v8::Value> object,
                        const std::string_view& argname)
{
    CHECK(isolate);
    if (!object->IsObject())
        g_throw(TypeError, fmt::format("Argument `{}` must be a CkMatrix", argname));
    CkMatrix *w = binder::UnwrapObject<CkMatrix>(isolate, object);
    if (!w)
        g_throw(TypeError, fmt::format("Argument `{}` must be a CkMatrix", argname));
    return w;
}

#define UNWRAP_MATRIX(isolate, arg) unwrap_matrix(isolate, arg, #arg)

} // namespace anonymous

v8::Local<v8::Object> VertexBatchBuilder::GetSelf(v8::Isolate *isolate)
{
    if (self_.IsEmpty())
    {
        v8::HandleScope scope(isolate);
        v8::Local<v8::Object> obj = binder::Class<VertexBatchBuilder>::find_object(isolate, this);
        self_.Reset(isolate, obj);
        // Set self handle weak to avoid circular reference
        self_.SetWeak();
    }

    return self_.Get(isolate);
}

v8::Local<v8::Value> VertexBatchBuilder::pushPositionMatrix(v8::Local<v8::Value> matrix)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CkMatrix *mat = UNWRAP_MATRIX(isolate, matrix);
    matrix_store_.emplace_back(mat->GetMatrix());
    pos_matrix_stack_.push(static_cast<int32_t>(matrix_store_.size()) - 1);
    return GetSelf(isolate);
}

v8::Local<v8::Value> VertexBatchBuilder::pushTexCoordMatrix(v8::Local<v8::Value> matrix)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CkMatrix *mat = UNWRAP_MATRIX(isolate, matrix);
    matrix_store_.emplace_back(mat->GetMatrix());
    uvs_matrix_stack_.push(static_cast<int32_t>(matrix_store_.size()) - 1);
    return GetSelf(isolate);
}

v8::Local<v8::Value> VertexBatchBuilder::popPositionMatrix()
{
    if (pos_matrix_stack_.empty())
        g_throw(Error, "Empty position matrix stack");
    pos_matrix_stack_.pop();
    return GetSelf(v8::Isolate::GetCurrent());
}

v8::Local<v8::Value> VertexBatchBuilder::popTexCoordMatrix()
{
    if (uvs_matrix_stack_.empty())
        g_throw(Error, "Empty UV matrix stack");
    uvs_matrix_stack_.pop();
    return GetSelf(v8::Isolate::GetCurrent());
}

v8::Local<v8::Value> VertexBatchBuilder::addVertexGroup(v8::Local<v8::Value> positions,
                                                        v8::Local<v8::Value> texCoords)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!positions->IsFloat32Array())
        g_throw(TypeError, "Argument `positions` must be a Float32Array");

    size_t pos_length = positions.As<v8::Float32Array>()->Length();
    if (pos_length & 1)
        g_throw(Error, "Invalid array length for argument `positions`");

    if (pos_matrix_stack_.empty())
        g_throw(Error, "Empty position matrix stack");

    VertexBatch::VertexGroup group;
    group.positions.Reset(isolate, positions.As<v8::Float32Array>());
    group.pos_mat_id = pos_matrix_stack_.top();

    if (!texCoords->IsNullOrUndefined())
    {
        if (!texCoords->IsFloat32Array())
            g_throw(TypeError, "Argument `texCoords` must be a Float32Array or null");
        size_t uv_length = texCoords.As<v8::Float32Array>()->Length();
        if (uv_length != pos_length)
            g_throw(Error, "Invalid array length for argument `texCoords`");
        if (uvs_matrix_stack_.empty())
            g_throw(Error, "Empty UV matrix stack");

        group.tex_coords.Reset(isolate, texCoords.As<v8::Float32Array>());
        group.uv_mat_id = uvs_matrix_stack_.top();
    }

    groups_.emplace_back(std::move(group));

    return GetSelf(isolate);
}

v8::Local<v8::Value> VertexBatchBuilder::build()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Object> batch = binder::NewObject<VertexBatch>(
            isolate, std::move(matrix_store_), std::move(groups_));

    std::stack<int32_t> empty_st1, empty_st2;
    pos_matrix_stack_.swap(empty_st1);
    uvs_matrix_stack_.swap(empty_st2);
    return batch;
}

ConcurrentVertexProcessor::ConcurrentVertexProcessor(uint32_t vertexCountHint,
                                                     uint32_t uvCountHint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    uint32_t vertex_count = std::max(kMinPreallocateVertexCount, vertexCountHint);
    uint32_t uv_count = std::max(kMinPreallocateVertexCount, uvCountHint);

    out_vertex_buffer_ = v8::ArrayBuffer::NewBackingStore(
            isolate, vertex_count * 2 * sizeof(float));
    out_uv_buffer_ = v8::ArrayBuffer::NewBackingStore(
            isolate, uv_count * 2 * sizeof(float));
}

namespace {

// NOLINTNEXTLINE
struct TransformContext
{
    constexpr static int32_t kPerWorkerPayloadHint = 1000;

    struct DataStore
    {
        static DataStore FromArray(v8::Local<v8::Float32Array> array) {
            return {
                .buffer = array->Buffer()->GetBackingStore(),
                .byte_offset = array->ByteOffset(),
                .byte_length = array->ByteLength()
            };
        }

        g_nodiscard SkPoint *GetPointer() {
            return reinterpret_cast<SkPoint*>(
                    reinterpret_cast<uint8_t*>(buffer->Data()) + byte_offset);
        }

        g_nodiscard int32_t GetPointCount() const {
            return static_cast<int32_t>(byte_length / (sizeof(float) * 2));
        }

        std::shared_ptr<v8::BackingStore> buffer;
        size_t byte_offset;
        size_t byte_length;
    };

    // A task is a vertices group
    struct PerTaskContext
    {
        int32_t index;
        int32_t count;
        DataStore out_pos;
        DataStore out_uvs;
        DataStore in_pos;
        DataStore in_uvs;
        int32_t pos_mat_id;
        int32_t uvs_mat_id;
    };

    // A worker is a request in the threadpool, which may contains
    // multiple tasks, as small tasks are merged into one worker.
    struct PerWorkerContext
    {
        std::vector<int32_t> task_indices;

        // The number of vertices that this worker carries in total
        int32_t total_count = 0;
    };

    void ResolvePromise()
    {
        v8::HandleScope scope(isolate);

        std::vector<v8::Local<v8::Value>> results(per_task_contexts.size());
        v8::Local<v8::Name> prop_names[] = {
            v8::String::NewFromUtf8Literal(isolate, "positions"),
            v8::String::NewFromUtf8Literal(isolate, "texCoords")
        };

        // All the tasks exactly share the same output buffer, but they
        // use different slices to store the corresponding data.
        auto out_pos_arrbuf = v8::ArrayBuffer::New(isolate, shared_out_pos_buffer);
        auto out_uv_arrbuf = v8::ArrayBuffer::New(isolate, shared_out_uv_buffer);

        for (int32_t i = 0; i < per_task_contexts.size(); i++)
        {
            auto& ctx = per_task_contexts[i];

            v8::Local<v8::Value> props[] = {
                v8::Float32Array::New(out_pos_arrbuf,
                                      ctx.out_pos.byte_offset,
                                      ctx.out_pos.byte_length / sizeof(float)),
                v8::Null(isolate)
            };

            if (ctx.out_uvs.buffer)
            {
                props[1] = v8::Float32Array::New(out_uv_arrbuf,
                                                 ctx.out_uvs.byte_offset,
                                                 ctx.out_uvs.byte_length / sizeof(float));
            }

            results[i] = v8::Object::New(isolate, v8::Null(isolate), prop_names, props, 2);
        }

        resolver.Get(isolate)->Resolve(isolate->GetCurrentContext(),
                                       v8::Array::New(isolate, results.data(), results.size()))
                                       .Check();
    }

    void ComputePerWorkerPayload(std::vector<std::pair<int32_t, int32_t>> idx_count_pairs)
    {
        std::sort(idx_count_pairs.begin(), idx_count_pairs.end(),
                  [](const auto& a, const auto& b) { return a.second < b.second; });

        per_worker_contexts.emplace_back();
        auto *last_worker = &per_worker_contexts.back();

        for (const auto& icpair : idx_count_pairs)
        {
            last_worker->task_indices.push_back(icpair.first);
            last_worker->total_count += icpair.second;
            if (last_worker->total_count > kPerWorkerPayloadHint)
            {
                per_worker_contexts.emplace_back();
                last_worker = &per_worker_contexts.back();
            }
        }

        if (last_worker->total_count == 0)
            per_worker_contexts.erase(per_worker_contexts.end() - 1);
    }

    std::vector<SkMatrix> matrix_store;

    std::vector<PerTaskContext> per_task_contexts;
    std::vector<PerWorkerContext> per_worker_contexts;

    int32_t finished_count;

    v8::Isolate *isolate;
    v8::Persistent<v8::Promise::Resolver> resolver;
    std::shared_ptr<v8::BackingStore> shared_out_pos_buffer;
    std::shared_ptr<v8::BackingStore> shared_out_uv_buffer;
};

} // namespace anonymous

v8::Local<v8::Value> ConcurrentVertexProcessor::transform(v8::Local<v8::Value> batch)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto *batch_ptr = binder::UnwrapObject<VertexBatch>(isolate, batch);
    if (!batch_ptr)
        g_throw(TypeError, "Argument `batch` must be an instance of `VertexBatch`");

    auto transform_ctx = std::make_unique<TransformContext>();
    transform_ctx->finished_count = 0;
    transform_ctx->isolate = isolate;
    transform_ctx->matrix_store = std::move(batch_ptr->GetMatrixStore());

    std::vector<TransformContext::PerTaskContext> per_task_contexts;

    uint32_t vertex_count = 0, uvs_count = 0;
    std::vector<std::pair<int32_t, int32_t>> per_task_idx_count_pairs;

    for (const auto& group : batch_ptr->GetVertexGroups())
    {
        CHECK(!group.positions.IsEmpty());

        auto count = static_cast<int32_t>(group.positions.Get(isolate)->Length() / 2);

        TransformContext::PerTaskContext ctx{
            .index = static_cast<int32_t>(per_task_contexts.size()),
            .count = count,
            .out_pos = {},
            .out_uvs = {},
            .in_pos = TransformContext::DataStore::FromArray(group.positions.Get(isolate)),
            .in_uvs = {},
            .pos_mat_id = group.pos_mat_id,
            .uvs_mat_id = group.uv_mat_id
        };

        vertex_count += ctx.in_pos.byte_length / (sizeof(float) * 2);
        if (!group.tex_coords.IsEmpty())
        {
            ctx.in_uvs = TransformContext::DataStore::FromArray(group.tex_coords.Get(isolate));
            uvs_count += ctx.in_uvs.byte_length / (sizeof(float) * 2);
        }

        per_task_contexts.emplace_back(ctx);
        per_task_idx_count_pairs.emplace_back(ctx.index, ctx.count);
    }

    TryReallocateOutputBuffers(vertex_count, uvs_count);
    transform_ctx->shared_out_pos_buffer = out_vertex_buffer_;
    transform_ctx->shared_out_uv_buffer = out_uv_buffer_;

    size_t out_vertex_offset = 0, out_uv_offset = 0;
    for (auto& per_task : per_task_contexts)
    {
        per_task.out_pos.buffer = out_vertex_buffer_;
        per_task.out_pos.byte_length = per_task.in_pos.byte_length;
        per_task.out_pos.byte_offset = out_vertex_offset;
        out_vertex_offset += per_task.out_pos.byte_length;

        if (per_task.in_uvs.buffer)
        {
            per_task.out_uvs.buffer = out_uv_buffer_;
            per_task.out_uvs.byte_length = per_task.in_uvs.byte_length;
            per_task.out_uvs.byte_offset = out_uv_offset;
            out_uv_offset += per_task.out_uvs.byte_length;
        }
    }

    transform_ctx->per_task_contexts = std::move(per_task_contexts);
    transform_ctx->ComputePerWorkerPayload(std::move(per_task_idx_count_pairs));

    auto resolver = CHECKED(v8::Promise::Resolver::New(isolate->GetCurrentContext()));
    transform_ctx->resolver.Reset(isolate, resolver);

    EventLoop *loop = EventLoop::GetCurrent();
    for (int32_t i = 0; i < transform_ctx->per_worker_contexts.size(); i++)
    {
        loop->enqueueThreadPoolTrivialTask([ctx = transform_ctx.get(), idx = i]() {
            TRACE_EVENT("rendering", "ConcurrentVertexProcessor::Transform");

            auto& per_worker = ctx->per_worker_contexts[idx];

            for (int32_t task_id : per_worker.task_indices)
            {
                auto& per_task = ctx->per_task_contexts[task_id];
                SkPoint *src = per_task.in_pos.GetPointer();
                SkPoint *dst = per_task.out_pos.GetPointer();
                ctx->matrix_store[per_task.pos_mat_id].mapPoints(
                        dst, src, per_task.in_pos.GetPointCount());
                if (per_task.in_uvs.buffer)
                {
                    src = per_task.in_uvs.GetPointer();
                    dst = per_task.out_uvs.GetPointer();
                    ctx->matrix_store[per_task.uvs_mat_id].mapPoints(
                            dst, src, per_task.in_uvs.GetPointCount());
                }
            }
        }, [ctx = transform_ctx.get()]() {
            TRACE_EVENT("rendering", "ConcurrentVertexProcessor::PostTransform");

            ctx->finished_count++;
            if (ctx->finished_count < ctx->per_worker_contexts.size())
                return;

            ctx->ResolvePromise();
            delete ctx;
        });
    }

    (void) transform_ctx.release();
    return resolver->GetPromise();
}

void ConcurrentVertexProcessor::TryReallocateOutputBuffers(uint32_t vert_count,
                                                           uint32_t uv_count)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    size_t vert_byte_length = vert_count * sizeof(float) * 2;
    if (vert_byte_length > out_vertex_buffer_->ByteLength())
        out_vertex_buffer_ = v8::ArrayBuffer::NewBackingStore(isolate, vert_byte_length);

    size_t uvs_byte_length = uv_count * sizeof(float) * 2;
    if (uvs_byte_length > out_uv_buffer_->ByteLength())
        out_uv_buffer_ = v8::ArrayBuffer::NewBackingStore(isolate, uvs_byte_length);
}

GALLIUM_BINDINGS_GLAMOR_NS_END
