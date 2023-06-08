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

#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"

#include "Gallium/binder/TypeTraits.h"
#include "Gallium/binder/ThrowExcept.h"
#include "Gallium/binder/Class.h"
#include "Gallium/bindings/pixencoder/Exports.h"
#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/TrivialInterface.h"
GALLIUM_BINDINGS_PIXENCODER_NS_BEGIN

namespace {

SkPngEncoder::Options extract_options(v8::Isolate *isolate,
                                      v8::Local<v8::Value> options)
{
    // Initialized by default values
    SkPngEncoder::Options result{};

    if (!binder::IsSome<v8::Object>(options))
        g_throw(TypeError, "Argument `options` must be an object");

    auto ctx = isolate->GetCurrentContext();

#define LLK(str) v8::String::NewFromUtf8Literal(isolate, str)
#define HASPROP(M) (!M.IsEmpty() && !M.ToLocalChecked()->IsNullOrUndefined())

    auto obj = options.As<v8::Object>();

    if (auto M = obj->Get(ctx, LLK("filterFlags")); HASPROP(M))
    {
        auto v = M.ToLocalChecked();
        if (!binder::IsSome<v8::Uint32>(v))
            g_throw(TypeError, "Property `options.filterFlags` must be an unsigned integer");
        result.fFilterFlags = static_cast<SkPngEncoder::FilterFlag>(
                v->Uint32Value(ctx).ToChecked());
    }

    if (auto M = obj->Get(ctx, LLK("zlibLevel")); HASPROP(M))
    {
        auto v = M.ToLocalChecked();
        if (!binder::IsSome<v8::Uint32>(v))
            g_throw(TypeError, "Property `options.zlibLevel` must be an unsigned integer");
        uint32_t p = v->Uint32Value(ctx).ToChecked();
        if (p > 9)
            g_throw(RangeError, "Property `options.zlibLevel` is out of range [0, 9]");
        result.fZLibLevel = static_cast<int32_t>(p);
    }

    if (auto M = obj->Get(ctx, LLK("comments")); HASPROP(M))
    {
        auto v = M.ToLocalChecked();
        if (!binder::IsSome<v8::Array>(v))
            g_throw(TypeError, "Property `options.comments` must be an array of Uint8Array");
        auto array = v.As<v8::Array>();

        // Skia requires that the 2i-th entry of `comments` is the keyword for
        // the i-th comment, and the (2i + 1)-th entry is the text for the i-th
        // comment. So the number of entries should be an even number.
        if (array->Length() & 1)
            g_throw(TypeError, "Property `options.comments` has an invalid size");

        uint32_t count = array->Length();
        std::vector<void*> ptrs(count);
        std::vector<size_t> sizes(count);
        for (int32_t i = 0; i < count; i++)
        {
            auto buf = array->Get(ctx, i).FromMaybe(v8::Local<v8::Value>());
            if (buf.IsEmpty() || !binder::IsSome<v8::Uint8Array>(buf))
                g_throw(TypeError, "Property `options.comments` must be an array of Uint8Array");

            auto memory = binder::GetTypedArrayMemory<v8::Uint8Array>(buf);
            if (!memory)
                g_throw(TypeError, "Property `options.comments` has invalid buffers");

            ptrs[i] = memory->ptr;
            sizes[i] = memory->byte_size;
        }

        result.fComments = SkDataTable::MakeCopyArrays(
                ptrs.data(), sizes.data(), static_cast<int>(count));
    }

#undef LLK
#undef HASPROP

    return result;
}

} // namespace anonymous

v8::Local<v8::Value> PNGEncoder::EncodeImage(v8::Local<v8::Value> img,
                                             v8::Local<v8::Value> options)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto *imgwrap = binder::UnwrapObject<glamor_wrap::CkImageWrap>(isolate, img);
    if (!imgwrap)
        g_throw(TypeError, "Argument `img` is not a `glamor.CkImage`");

    SkPngEncoder::Options opts = extract_options(isolate, options);
    sk_sp<SkData> data = SkPngEncoder::Encode(nullptr, imgwrap->getImage().get(), opts);

    if (!data)
        return v8::Null(isolate);

    // `SkData::writable_data` method asserts that the reference count
    // is 1, as a result we should get the address in a single statement
    // before `data` is copied for an argument of `CreateBackingStoreFromSmartPtrMemory`.
    void *writable_ptr = data->writable_data();

    std::shared_ptr<v8::BackingStore> store =
            binder::CreateBackingStoreFromSmartPtrMemory(data, writable_ptr, data->size());
    return v8::ArrayBuffer::New(isolate, store);
}

v8::Local<v8::Value> PNGEncoder::EncodeMemory(v8::Local<v8::Value> info,
                                              v8::Local<v8::Value> pixels,
                                              int64_t row_bytes,
                                              v8::Local<v8::Value> options)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkImageInfo img_info = glamor_wrap::ExtractCkImageInfo(isolate, info);
    SkPngEncoder::Options opts = extract_options(isolate, options);

    auto memory = binder::GetTypedArrayMemory<v8::Uint8Array>(pixels);
    if (!memory)
        g_throw(TypeError, "Argument `pixels` must be an allocated Uint8Array");

    if (img_info.computeByteSize(row_bytes) > memory->byte_size)
        g_throw(Error, "Pixels buffer has an invalid size (conflicts with provided image info)");

    SkPixmap pixmap(img_info, memory->ptr, row_bytes);

    SkDynamicMemoryWStream stream;
    if (!SkPngEncoder::Encode(&stream, pixmap, opts))
        return v8::Null(isolate);

    sk_sp<SkData> data = stream.detachAsData();
    void *writable_ptr = data->writable_data();

    std::shared_ptr<v8::BackingStore> store =
            binder::CreateBackingStoreFromSmartPtrMemory(data, writable_ptr, data->size());
    return v8::ArrayBuffer::New(isolate, store);
}

GALLIUM_BINDINGS_PIXENCODER_NS_END
