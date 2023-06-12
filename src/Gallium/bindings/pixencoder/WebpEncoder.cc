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

#include "include/encode/SkEncoder.h"
#include "include/encode/SkWebpEncoder.h"
#include "include/core/SkStream.h"

#include "Gallium/binder/ThrowExcept.h"
#include "Gallium/binder/TypeTraits.h"
#include "Gallium/binder/Class.h"
#include "Gallium/bindings/pixencoder/Exports.h"
#include "Gallium/bindings/glamor/Exports.h"
GALLIUM_BINDINGS_PIXENCODER_NS_BEGIN

#define LLK(str) v8::String::NewFromUtf8Literal(isolate, str)
#define HASPROP(M) (!M.IsEmpty() && !M.ToLocalChecked()->IsNullOrUndefined())

namespace {

SkWebpEncoder::Options extract_options(v8::Isolate *isolate,
                                       v8::Local<v8::Value> options)
{
    // Initialized by default values
    SkWebpEncoder::Options result{};

    if (!binder::IsSome<v8::Object>(options))
        g_throw(TypeError, "Argument `options` is not an object");

    auto obj = options.As<v8::Object>();
    auto ctx = isolate->GetCurrentContext();

    if (auto M = obj->Get(ctx, LLK("compression")); HASPROP(M))
    {
        auto value = M.ToLocalChecked();
        if (!binder::IsSome<v8::Uint32>(value))
            g_throw(TypeError, "Property `options.compression` is not an unsigned integer");
        uint32_t intv = value->Uint32Value(ctx).ToChecked();
        if (intv > static_cast<uint32_t>(SkWebpEncoder::Compression::kLossless))
            g_throw(RangeError, "Property `options.compression` has an invalid enumeration");
        result.fCompression = static_cast<SkWebpEncoder::Compression>(intv);
    }

    if (auto M = obj->Get(ctx, LLK("quality")); HASPROP(M))
    {
        auto value = M.ToLocalChecked();
        if (!binder::IsSome<v8::Number>(value))
            g_throw(TypeError, "Property `options.quality` is not a number");
        double floatv = value->NumberValue(ctx).ToChecked();
        if (floatv < 0 || floatv > 100)
            g_throw(RangeError, "Property `options.quality` is out of range [0, 100]");
        result.fQuality = static_cast<float>(floatv);
    }

    return result;
}

} // namespace anonymous

v8::Local<v8::Value> WebpEncoder::EncodeImage(v8::Local<v8::Value> img,
                                              v8::Local<v8::Value> options)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto *imgwrap = binder::UnwrapObject<glamor_wrap::CkImageWrap>(isolate, img);
    if (!imgwrap)
        g_throw(TypeError, "Argument `img` is not a `glamor.CkImage`");

    SkWebpEncoder::Options opts = extract_options(isolate, options);
    sk_sp<SkData> data = SkWebpEncoder::Encode(nullptr, imgwrap->getImage().get(), opts);

    if (!data)
        return v8::Null(isolate);

    void *writable_ptr = data->writable_data();

    std::shared_ptr<v8::BackingStore> store =
            binder::CreateBackingStoreFromSmartPtrMemory(data, writable_ptr, data->size());
    return v8::ArrayBuffer::New(isolate, store);
}

v8::Local<v8::Value> WebpEncoder::EncodeMemory(v8::Local<v8::Value> info,
                                               v8::Local<v8::Value> pixels,
                                               int64_t row_bytes,
                                               v8::Local<v8::Value> options)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkImageInfo img_info = glamor_wrap::ExtractCkImageInfo(isolate, info);
    SkWebpEncoder::Options opts = extract_options(isolate, options);

    auto memory = binder::GetTypedArrayMemory<v8::Uint8Array>(pixels);
    if (!memory)
        g_throw(TypeError, "Argument `pixels` must be an allocated Uint8Array");

    if (img_info.computeByteSize(row_bytes) > memory->byte_size)
        g_throw(Error, "Pixels buffer has an invalid size (conflicts with provided image info)");

    SkPixmap pixmap(img_info, memory->ptr, row_bytes);

    SkDynamicMemoryWStream stream;
    if (!SkWebpEncoder::Encode(&stream, pixmap, opts))
        return v8::Null(isolate);

    sk_sp<SkData> data = stream.detachAsData();
    void *writable_ptr = data->writable_data();

    std::shared_ptr<v8::BackingStore> store =
            binder::CreateBackingStoreFromSmartPtrMemory(data, writable_ptr, data->size());
    return v8::ArrayBuffer::New(isolate, store);
}

namespace {

SkEncoder::Frame extract_image_frame(v8::Isolate *isolate,
                                     v8::Local<v8::Value> value)
{
    if (!binder::IsSome<v8::Object>(value))
        g_throw(TypeError, "Argument `frames` must be an array of objects");

    auto ctx = isolate->GetCurrentContext();
    auto obj = value.As<v8::Object>();

    SkEncoder::Frame frame;

    if (auto M = obj->Get(ctx, LLK("image")); HASPROP(M))
    {
        auto prop = M.ToLocalChecked();
        auto *wrap = binder::UnwrapObject<glamor_wrap::CkImageWrap>(isolate, prop);
        if (!wrap)
            g_throw(TypeError, "Invalid property `image` on frame object");

        sk_sp<SkImage> image = wrap->getImage();
        SkPixmap pixmap;
        if (!image->peekPixels(&pixmap))
            g_throw(TypeError, "Invalid image: pixels is inaccessible");

        frame.pixmap = pixmap;
    }
    else
    {
        g_throw(TypeError, "Invalid frame object: missing property `image`");
    }

    if (auto M = obj->Get(ctx, LLK("duration")); HASPROP(M))
    {
        auto prop = M.ToLocalChecked();
        if (!binder::IsSome<v8::Uint32>(prop))
            g_throw(TypeError, "Invalid property `duration` on frame object");
        uint32_t u32v = prop->Uint32Value(ctx).ToChecked();
        CHECK(u32v <= INT_MAX);
        frame.duration = static_cast<int>(u32v);
    }
    else
    {
        g_throw(TypeError, "Invalid frame object: missing property `duration`");
    }

    return frame;
}

SkEncoder::Frame extract_memory_frame(v8::Isolate *isolate,
                                      v8::Local<v8::Value> value)
{
    if (!binder::IsSome<v8::Object>(value))
        g_throw(TypeError, "Argument `frames` must be an array of objects");

    auto ctx = isolate->GetCurrentContext();
    auto obj = value.As<v8::Object>();

    SkEncoder::Frame frame;

    SkImageInfo image_info;
    int64_t row_bytes;

    if (auto M = obj->Get(ctx, LLK("info")); HASPROP(M))
        image_info = glamor_wrap::ExtractCkImageInfo(isolate, M.ToLocalChecked());
    else
        g_throw(TypeError, "Invalid frame object: missing property `image`");

    if (auto M = obj->Get(ctx, LLK("rowBytes")); HASPROP(M))
    {
        auto prop = M.ToLocalChecked();
        if (!binder::IsSome<v8::Number>(prop))
            g_throw(TypeError, "Invalid property `rowBytes` on frame object");
        row_bytes = prop->IntegerValue(ctx).ToChecked();
    }
    else
    {
        g_throw(TypeError, "Invalid frame object: missing property `rowBytes`");
    }

    if (auto M = obj->Get(ctx, LLK("pixels")); HASPROP(M))
    {
        auto prop = M.ToLocalChecked();
        auto memory = binder::GetTypedArrayMemory<v8::Uint8Array>(prop.As<v8::Uint8Array>());
        if (!memory)
            g_throw(TypeError, "Invalid property `pixels` on frame object");

        if (memory->byte_size < image_info.computeByteSize(row_bytes))
            g_throw(TypeError, "Invalid frame object: invalid size of pixels buffer");

        frame.pixmap = SkPixmap(image_info, memory->ptr, row_bytes);
    }
    else
    {
        g_throw(TypeError, "Invalid frame object: missing property `pixels`");
    }

    if (auto M = obj->Get(ctx, LLK("duration")); HASPROP(M))
    {
        auto prop = M.ToLocalChecked();
        if (!binder::IsSome<v8::Uint32>(prop))
            g_throw(TypeError, "Invalid property `duration` on frame object");
        uint32_t u32v = prop->Uint32Value(ctx).ToChecked();
        CHECK(u32v <= INT_MAX);
        frame.duration = static_cast<int>(u32v);
    }
    else
    {
        g_throw(TypeError, "Invalid frame object: missing property `duration`");
    }

    return frame;
}

v8::Local<v8::Value> final_encode_animated(v8::Isolate *isolate,
                                           const std::vector<SkEncoder::Frame>& frames,
                                           const SkWebpEncoder::Options& opts)
{
    SkDynamicMemoryWStream stream;
    if (!SkWebpEncoder::EncodeAnimated(&stream, frames, opts))
        return v8::Null(isolate);

    sk_sp<SkData> data = stream.detachAsData();
    CHECK(data);

    void *writable_addr = data->writable_data();
    auto store = binder::CreateBackingStoreFromSmartPtrMemory(
            data, writable_addr, data->size());

    return v8::ArrayBuffer::New(isolate, store);
}

using FrameObjectExtractor = SkEncoder::Frame(*)(v8::Isolate*, v8::Local<v8::Value>);

v8::Local<v8::Value> encode_animated(v8::Local<v8::Value> frames,
                                     v8::Local<v8::Value> options,
                                     FrameObjectExtractor extractor)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    if (!binder::IsSome<v8::Array>(frames))
        g_throw(TypeError, "Argument `frames` must be an array of objects");

    auto frames_array = frames.As<v8::Array>();
    uint32_t count = frames_array->Length();

    if (count == 0)
        return v8::Null(isolate);

    std::vector<SkEncoder::Frame> frames_vec(count);
    auto ctx = isolate->GetCurrentContext();
    for (uint32_t i = 0; i < count; i++)
    {
        auto M = frames_array->Get(ctx, i);
        if (!HASPROP(M))
            g_throw(TypeError, "Argument `frames` must be an array of objects");
        frames_vec[i] = extractor(isolate, M.ToLocalChecked());
    }

    SkWebpEncoder::Options opts = extract_options(isolate, options);
    return final_encode_animated(isolate, frames_vec, opts);
}

} // namespace anonymous

v8::Local<v8::Value> WebpEncoder::EncodeAnimatedImage(v8::Local<v8::Value> frames,
                                                      v8::Local<v8::Value> options)
{
    return encode_animated(frames, options, extract_image_frame);
}

v8::Local<v8::Value> WebpEncoder::EncodeAnimatedMemory(v8::Local<v8::Value> frames,
                                                       v8::Local<v8::Value> options)
{
    return encode_animated(frames, options, extract_memory_frame);
}

GALLIUM_BINDINGS_PIXENCODER_NS_END
