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
#include "Gallium/bindings/pixencoder/WebpEncoder.h"
GALLIUM_BINDINGS_PIXENCODER_NS_BEGIN

SkWebpEncoder::Options WebpEncoderOptions::Make() const
{
    SkWebpEncoder::Options result;
    if (quality)
        result.fQuality = *quality;
    if (compression)
        result.fCompression = **compression;
    return result;
}

ffi::RetLocal<v8::Value>
WebpEncoder::EncodeImage(ffi::Opt<ffi::Class<renderer::GpuDirectContext>> ctx,
                         ffi::Class<renderer::Image> image,
                         ffi::IFace<WebpEncoderOptions> options)
{
    sk_sp<SkData> encode = SkWebpEncoder::Encode(
            ctx ? (*ctx)->GetGrDirectContext().get() : nullptr,
            image->GetSkImage().get(), options->Make());
    if (!encode)
        return ffi::Fail(ffi::kErr, "failed to encode image into Webp format");

    // Only when we are the only owner of SkData, `SkData::writable_data()`
    // can be called properly.
    CHECK(encode->unique());
    void *writable_addr = encode->writable_data();
    size_t size = encode->size();

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::Mem<uint8_t>(isolate, std::move(encode), writable_addr, size)
            .TypedArray();
}

ffi::RetLocal<v8::Value> WebpEncoder::EncodePixmap(renderer::PixmapAdapter pixmap,
                                                   ffi::IFace<WebpEncoderOptions> options)
{
    SkDynamicMemoryWStream stream;
    if (!SkWebpEncoder::Encode(&stream, *pixmap, options->Make()))
        return ffi::Fail(ffi::kErr, "failed to encode pixmap into PNG format");

    sk_sp<SkData> encode = stream.detachAsData();
    CHECK(encode->unique());
    void *writable_addr = encode->writable_data();
    size_t size = encode->size();

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::Mem<uint8_t>(isolate, std::move(encode), writable_addr, size)
            .TypedArray();
}

ffi::Ret<SkEncoder::Frame> WebpFrame::Make(v8::Isolate *isolate) const
{
    auto maybe = renderer::PixmapAdapter::Cast(isolate, pixmap);
    if (maybe.HasError())
    {
        return ffi::Fail(ffi::kErr, fmt::format(
                "property `pixmap` of interface `WebpFrame`: {}", maybe.GetError().message));
    }

    return SkEncoder::Frame{ *maybe.Extract(), duration };
}

ffi::RetLocal<v8::Value> WebpEncoder::EncodeAnimated(v8::Local<v8::Array> frames,
                                                     ffi::IFace<WebpEncoderOptions> options)
{
    if (frames->Length() == 0)
        return ffi::Fail(ffi::kErr, "empty frame list");
    uint32_t frame_count = frames->Length();
    std::vector<SkEncoder::Frame> frames_vec(frame_count);

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    v8::TryCatch try_catch(isolate);
    for (uint32_t i = 0; i < frame_count; i++)
    {
        v8::Local<v8::Value> element;
        if (!frames->Get(ctx, i).ToLocal(&element))
            return ffi::Fail(try_catch, "failed to access the frame object in array: ");
        auto maybe_webp_frame = ffi::Cast<ffi::IFace<WebpFrame>>::From(isolate, element);
        if (maybe_webp_frame.HasError())
        {
            return ffi::Fail(ffi::kTypeErr, fmt::format(
                    "element #{} of frame array: {}", i, maybe_webp_frame.GetError().message));
        }

        auto maybe_frame = maybe_webp_frame.Extract()->Make(isolate);
        if (maybe_frame.HasError())
        {
            return ffi::Fail(ffi::kTypeErr, fmt::format(
                    "element #{} of frame array: {}", i, maybe_frame.GetError().message));
        }
        frames_vec[i] = maybe_frame.Extract();
    }

    SkDynamicMemoryWStream stream;
    if (!SkWebpEncoder::EncodeAnimated(&stream, frames_vec, options->Make()))
        return ffi::Fail(ffi::kErr, "failed to encode pixmap into Webp format");

    sk_sp<SkData> encode = stream.detachAsData();
    CHECK(encode->unique());
    void *writable_addr = encode->writable_data();
    size_t size = encode->size();

    return ffi::Mem<uint8_t>(isolate, std::move(encode), writable_addr, size)
            .TypedArray();
}

GALLIUM_BINDINGS_PIXENCODER_NS_END
