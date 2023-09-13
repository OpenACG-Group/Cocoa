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

#include "include/core/SkData.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/core/SkStream.h"

#include "Core/Errors.h"
#include "Gallium/binder/TypeTraits.h"
#include "Gallium/binder/ThrowExcept.h"
#include "Gallium/binder/Class.h"
#include "Gallium/bindings/pixencoder/Exports.h"
#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/CkImageWrap.h"
GALLIUM_BINDINGS_PIXENCODER_NS_BEGIN

namespace {

SkJpegEncoder::Options extract_options(v8::Isolate *isolate,
                                       v8::Local<v8::Value> options,
                                       SkData **out_external_data)
{
    *out_external_data = nullptr;

    // Initialized by default values
    SkJpegEncoder::Options result{};

    if (!binder::IsSome<v8::Object>(options))
        g_throw(TypeError, "Argument `options` must be an object");

    auto ctx = isolate->GetCurrentContext();
    auto obj = options.As<v8::Object>();

#define LLK(str) v8::String::NewFromUtf8Literal(isolate, str)
#define HASPROP(M) (!M.IsEmpty() && !M.ToLocalChecked()->IsNullOrUndefined())

    if (auto M = obj->Get(ctx, LLK("quality")); HASPROP(M))
    {
        auto prop = M.ToLocalChecked();
        if (!binder::IsSome<v8::Uint32>(prop))
            g_throw(TypeError, "Property `options.quality` must be an unsigned integer");
        uint32_t uint_value = prop->Uint32Value(ctx).ToChecked();
        if (uint_value > 100)
            g_throw(RangeError, "Property `options.quality` is out of range [0, 100]");
        result.fQuality = static_cast<int>(uint_value);
    }

    if (auto M = obj->Get(ctx, LLK("downsample")); HASPROP(M))
    {
        auto prop = M.ToLocalChecked();
        if (!binder::IsSome<v8::Uint32>(prop))
            g_throw(TypeError, "Property `options.downsample` must be an unsigned integer");
        uint32_t uint_value = prop->Uint32Value(ctx).ToChecked();
        if (uint_value > static_cast<uint32_t>(SkJpegEncoder::Downsample::k444))
            g_throw(RangeError, "Property `options.downsample` is an invalid enumeration");
        result.fDownsample = static_cast<SkJpegEncoder::Downsample>(uint_value);
    }

    if (auto M = obj->Get(ctx, LLK("alphaOption")); HASPROP(M))
    {
        auto prop = M.ToLocalChecked();
        if (!binder::IsSome<v8::Uint32>(prop))
            g_throw(TypeError, "Property `options.alphaOption` must be an unsigned integer");
        uint32_t uint_value = prop->Uint32Value(ctx).ToChecked();
        if (uint_value > static_cast<uint32_t>(SkJpegEncoder::AlphaOption::kBlendOnBlack))
            g_throw(RangeError, "Property `options.alphaOption` is an invalid enumeration");
        result.fAlphaOption = static_cast<SkJpegEncoder::AlphaOption>(uint_value);
    }

    if (auto M = obj->Get(ctx, LLK("xmpMetadata")); HASPROP(M))
    {
        auto prop = M.ToLocalChecked();
        auto memory = binder::GetTypedArrayMemory<v8::Uint8Array>(prop);
        if (!memory)
            g_throw(TypeError, "Property `options.xmpMetadata` is not an allocated Uint8Array");

        // The backing store of Uint8Array is alive in the scope of
        // the caller (`JPEGEncoder::Encode*` function), and the created
        // SkData object will be freed by the caller before return.
        // Consequently, it is safe to use `MakeWithoutCopy`.
        SkData *bare_data_ptr =
                SkData::MakeWithoutCopy(memory->ptr, memory->byte_size).release();

        *out_external_data = bare_data_ptr;
        result.xmpMetadata = bare_data_ptr;
    }

#undef LLK
#undef HASPROP

    return result;
}

} // namespace anonymous

v8::Local<v8::Value> JPEGEncoder::EncodeImage(v8::Local<v8::Value> img,
                                              v8::Local<v8::Value> options)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto *imgwrap = binder::UnwrapObject<glamor_wrap::CkImageWrap>(isolate, img);
    if (!imgwrap)
        g_throw(TypeError, "Argument `img` is not an instance of `glamor.CkImage`");

    SkData *external_data;
    SkJpegEncoder::Options opts = extract_options(isolate, options, &external_data);

    sk_sp<SkData> encoded = SkJpegEncoder::Encode(
            nullptr, imgwrap->getImage().get(), opts);
    if (external_data)
    {
        // Assert to prevent memory leaking
        CHECK(external_data->unique());
        external_data->unref();
    }

    if (!encoded)
        return v8::Null(isolate);

    void *writable_addr = encoded->writable_data();
    std::shared_ptr<v8::BackingStore> store = binder::CreateBackingStoreFromSmartPtrMemory(
            encoded, writable_addr, encoded->size());
    return v8::ArrayBuffer::New(isolate, store);
}

v8::Local<v8::Value> JPEGEncoder::EncodeMemory(v8::Local<v8::Value> info,
                                               v8::Local<v8::Value> pixels,
                                               int64_t row_bytes,
                                               v8::Local<v8::Value> options)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkImageInfo img_info = glamor_wrap::ExtractCkImageInfo(isolate, info);

    SkData *external_data;
    SkJpegEncoder::Options opts = extract_options(isolate, options, &external_data);

    auto memory = binder::GetTypedArrayMemory<v8::Uint8Array>(pixels);
    if (!memory)
        g_throw(TypeError, "Argument `pixels` must be an allocated Uint8Array");

    if (img_info.computeByteSize(row_bytes) > memory->byte_size)
        g_throw(Error, "Pixels buffer has an invalid size (conflicts with provided image info)");

    SkPixmap pixmap(img_info, memory->ptr, row_bytes);

    SkDynamicMemoryWStream stream;
    bool success = SkJpegEncoder::Encode(&stream, pixmap, opts);

    if (external_data)
    {
        CHECK(external_data->unique());
        external_data->unref();
    }

    if (!success)
        return v8::Null(isolate);

    sk_sp<SkData> data = stream.detachAsData();
    void *writable_ptr = data->writable_data();
    std::shared_ptr<v8::BackingStore> store = binder::CreateBackingStoreFromSmartPtrMemory(
            data, writable_ptr, data->size());
    return v8::ArrayBuffer::New(isolate, store);
}

GALLIUM_BINDINGS_PIXENCODER_NS_END
