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

#include "Gallium/bindings/renderer/Matrix.h"
#include "Gallium/ffi/ArrayBuffer.h"
#include "Gallium/ffi/Context.h"
#include "Gallium/bindings/renderer/Module.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

namespace {

ffi::Ret<ffi::Mem<float>> get_matrix_memory_storage(v8::Isolate *isolate,
                                                    v8::Local<v8::Value> value)
{
    if (!value->IsObject())
        return ffi::Fail(ffi::kTypeErr, "cannot interpret the value as a Rect");

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    auto key = v8::String::NewFromUtf8Literal(isolate, "__mem__");

    v8::Local<v8::Value> mem_prop;

    v8::TryCatch try_catch(isolate);
    if (!value.As<v8::Object>()->Get(ctx, key).ToLocal(&mem_prop))
        return ffi::Fail(try_catch, "failed to read `__mem__` property: ");

    ffi::Ret<ffi::Mem<float>> mem = ffi::Cast<ffi::Mem<float>>::From(isolate, mem_prop);
    if (mem.HasError())
    {
        return ffi::Fail(ffi::kTypeErr, fmt::format(
                "failed to read `__mem__` property: {}", mem.GetError().message));
    }

    return mem;
}

ffi::Ret<void> fill_matrix_3x3(v8::Isolate *isolate,
                               v8::Local<v8::Value> value, SkMatrix& mat)
{
    v8::HandleScope handle_scope(isolate);
    auto maybe_mem = get_matrix_memory_storage(isolate, value);
    if (maybe_mem.HasError())
        return maybe_mem.GetError();
    ffi::Mem<float>& mem = maybe_mem.Extract();

    if (mem.Size() != 9)
        return ffi::Fail(ffi::kErr, "Mat3x3 `__mem__` property: requires a 9-element Float32Array");

    // Scalars are stored in column-major, see `./renderer.js`
    float *s = mem.Address();
    mat.setAll(s[0], s[3], s[6],
               s[1], s[4], s[7],
               s[2], s[5], s[8]);
    return {};
}

ffi::Ret<void> fill_matrix_4x4(v8::Isolate *isolate,
                               v8::Local<v8::Value> value, SkM44& mat)
{
    v8::HandleScope handle_scope(isolate);
    auto maybe_mem = get_matrix_memory_storage(isolate, value);
    if (maybe_mem.HasError())
        return maybe_mem.GetError();
    ffi::Mem<float>& mem = maybe_mem.Extract();

    if (mem.Size() != 16)
        return ffi::Fail(ffi::kErr, "Mat3x3 `__mem__` property: requires a 16-element Float32Array");

    // Scalars are stored in column-major, see `./renderer.js`
    float *s = mem.Address();
    mat.setCol(0, SkV4{ s[0], s[1], s[2], s[3] });
    mat.setCol(1, SkV4{ s[4], s[5], s[6], s[7] });
    mat.setCol(2, SkV4{ s[8], s[9], s[10], s[11] });
    mat.setCol(3, SkV4{ s[12], s[13], s[14], s[15] });
    return {};
}

} // namespace anonymous

ffi::Ret<SkMatrix> UnwrapJSMat3x3(v8::Isolate *isolate, v8::Local<v8::Value> value)
{
    SkMatrix matrix;
    auto err = fill_matrix_3x3(isolate, value, matrix);
    if (err.HasError())
        return err.GetError();
    return matrix;
}

ffi::Ret<SkM44> UnwrapJSMat4x4(v8::Isolate *isolate, v8::Local<v8::Value> value)
{
    SkM44 matrix;
    auto err = fill_matrix_4x4(isolate, value, matrix);
    if (err.HasError())
        return err.GetError();
    return matrix;
}

v8::Local<v8::Object> CreateJSMat3x3(v8::Isolate *isolate, const SkMatrix& from)
{
    v8::EscapableHandleScope handle_scope(isolate);

    auto ab = v8::ArrayBuffer::New(isolate, 9 * sizeof(float));
    float *mat = static_cast<float*>(ab->Data());
    mat[0] = from.get(SkMatrix::kMScaleX);
    mat[1] = from.get(SkMatrix::kMSkewY);
    mat[2] = from.get(SkMatrix::kMPersp0);
    mat[3] = from.get(SkMatrix::kMSkewX);
    mat[4] = from.get(SkMatrix::kMScaleY);
    mat[5] = from.get(SkMatrix::kMPersp1);
    mat[6] = from.get(SkMatrix::kMTransX);
    mat[7] = from.get(SkMatrix::kMTransY);
    mat[8] = from.get(SkMatrix::kMPersp2);

    auto *storage = ffi::TypeContext::FromIsolate(isolate)->GetValueStorage();
    auto ctor = storage->Load(FFI_GVSTORE_USE_ID(ctor_Mat3x3)).As<v8::Function>();
    CHECK(!ctor.IsEmpty());

    v8::Local<v8::Value> args[] = { v8::Float32Array::New(ab, 0, 9) };
    return handle_scope.Escape(
            ctor->NewInstance(isolate->GetCurrentContext(), 1, args).ToLocalChecked());
}

v8::Local<v8::Object> CreateJSMat4x4(v8::Isolate *isolate, const SkM44& from)
{
    v8::EscapableHandleScope handle_scope(isolate);

    auto ab = v8::ArrayBuffer::New(isolate, 16 * sizeof(float));
    float *dstaddr = static_cast<float*>(ab->Data());

    constexpr size_t kColSize = 4 * sizeof(float);
    std::memcpy(dstaddr, from.col(0).ptr(), kColSize);
    std::memcpy(dstaddr + 4, from.col(1).ptr(), kColSize);
    std::memcpy(dstaddr + 8, from.col(2).ptr(), kColSize);
    std::memcpy(dstaddr + 12, from.col(3).ptr(), kColSize);

    auto *storage = ffi::TypeContext::FromIsolate(isolate)->GetValueStorage();
    auto ctor = storage->Load(FFI_GVSTORE_USE_ID(ctor_Mat4x4)).As<v8::Function>();
    CHECK(!ctor.IsEmpty());

    v8::Local<v8::Value> args[] = { v8::Float32Array::New(ab, 0, 16) };
    return handle_scope.Escape(
            ctor->NewInstance(isolate->GetCurrentContext(), 1, args).ToLocalChecked());
}

ffi::Ret<Mat3x3Adapter> Mat3x3Adapter::Cast(v8::Isolate *isolate,
                                            v8::Local<v8::Value> value)
{
    auto result = UnwrapJSMat3x3(isolate, value);
    if (result.HasError())
        return result.GetError();
    return Mat3x3Adapter{ .matrix = result.Extract() };
}

ffi::Ret<Mat4x4Adapter> Mat4x4Adapter::Cast(v8::Isolate *isolate,
                                            v8::Local<v8::Value> value)
{
    auto result = UnwrapJSMat4x4(isolate, value);
    if (result.HasError())
        return result.GetError();
    return Mat4x4Adapter{ .matrix = result.Extract() };
}

ffi::Ret<RSXformArrayAdapter> RSXformArrayAdapter::Cast(v8::Isolate *isolate, v8::Local<v8::Value> value)
{
    if (!value->IsObject())
        return ffi::Fail(ffi::kTypeErr, "cannot interpret the value as a RSXformArray");

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    auto key = v8::String::NewFromUtf8Literal(isolate, "__mem__");

    v8::Local<v8::Value> mem_prop;

    v8::TryCatch try_catch(isolate);
    if (!value.As<v8::Object>()->Get(ctx, key).ToLocal(&mem_prop))
        return ffi::Fail(try_catch, "failed to read `__mem__` property of RSXformArray: ");

    ffi::Ret<ffi::Mem<float>> mem = ffi::Cast<ffi::Mem<float>>::From(isolate, mem_prop);
    if (mem.HasError())
    {
        return ffi::Fail(ffi::kTypeErr, fmt::format(
                "failed to read `__mem__` property of RSXformArray: {}", mem.GetError().message));
    }

    RSXformArrayAdapter adapter;
    adapter.memory = mem.Extract();
    if (adapter.memory.Size() % 4 != 0)
        return ffi::Fail(ffi::kErr, "internal storage of RSXformArray has an incompatible size");
    adapter.address = reinterpret_cast<SkRSXform*>(adapter.memory.Address());
    adapter.count = adapter.memory.Size() >> 2;

    return adapter;
}

GALLIUM_BINDINGS_RENDERER_NS_END
