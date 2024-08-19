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

#include <vector>

#include "include/core/SkStream.h"

#include "Gallium/bindings/pixencoder/PNGEncoder.h"
GALLIUM_BINDINGS_PIXENCODER_NS_BEGIN

ffi::Ret<PNGEncoderOptionsAdapter>
PNGEncoderOptionsAdapter::Cast(v8::Isolate *isolate, v8::Local<v8::Value> value)
{
    auto result = ffi::Cast<ffi::IFace<PNGEncoderOptions>>::From(isolate, value);
    if (result.HasError())
        return result.GetError();

    ffi::IFace<PNGEncoderOptions>& iface = result.Extract();
    PNGEncoderOptionsAdapter adapter;

    if (iface->filter_flags)
    {
        adapter.options.fFilterFlags = static_cast<SkPngEncoder::FilterFlag>(
                *iface->filter_flags);
    }
    if (iface->zlib_level)
        adapter.options.fZLibLevel = *iface->zlib_level;

    if (iface->comments)
    {
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Array> array = *iface->comments;
        v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
        // Skia requires that the 2i-th entry of `comments` is the keyword for
        // the i-th comment, and the (2i + 1)-th entry is the text for the i-th
        // comment. So the number of entries should be an even number.
        if (array->Length() & 1)
            return ffi::Fail(ffi::kErr, "`comments` property must be an array of flattened key-value pairs");

        uint32_t count = array->Length();
        std::vector<void*> ptrs(count);
        std::vector<size_t> sizes(count);
        v8::TryCatch try_catch(isolate);
        for (int32_t i = 0; i < count; i++)
        {
            v8::Local<v8::Value> element;
            if (!array->Get(ctx, i).ToLocal(&element))
                return ffi::Fail(try_catch, "failed to access array `comments`");

            auto mem = ffi::Cast<ffi::Mem<uint8_t>>::From(isolate, element);
            if (mem.HasError())
            {
                return ffi::Fail(ffi::kErr, fmt::format(
                        "array `comments` contains invalid Uint8Array: {}", mem.GetError().message));
            }

            ptrs[i] = mem.Extract().Address();
            sizes[i] = mem.Extract().ByteSize();
        }

        adapter.options.fComments = SkDataTable::MakeCopyArrays(
                ptrs.data(), sizes.data(), static_cast<int>(count));
    }

    return std::move(adapter);
}

ffi::RetLocal<v8::Value> PNGEncoder::EncodeImage(ffi::Opt<ffi::Class<renderer::GpuDirectContext>> ctx,
                                                 ffi::Class<renderer::Image> image,
                                                 const PNGEncoderOptionsAdapter& options)
{
    sk_sp<SkData> encode = SkPngEncoder::Encode(
            ctx ? (*ctx)->GetGrDirectContext().get() : nullptr,
            image->GetSkImage().get(), *options);
    if (!encode)
        return ffi::Fail(ffi::kErr, "failed to encode image into PNG format");

    // Only when we are the only owner of SkData, `SkData::writable_data()`
    // can be called properly.
    CHECK(encode->unique());
    void *writable_addr = encode->writable_data();
    size_t size = encode->size();

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::Mem<uint8_t>(isolate, std::move(encode), writable_addr, size)
           .TypedArray();
}

ffi::RetLocal<v8::Value> PNGEncoder::EncodePixmap(renderer::PixmapAdapter pixmap,
                                                  const PNGEncoderOptionsAdapter& options)
{
    SkDynamicMemoryWStream stream;
    if (!SkPngEncoder::Encode(&stream, *pixmap, *options))
        return ffi::Fail(ffi::kErr, "failed to encode pixmap into PNG format");

    sk_sp<SkData> encode = stream.detachAsData();
    CHECK(encode->unique());
    void *writable_addr = encode->writable_data();
    size_t size = encode->size();

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::Mem<uint8_t>(isolate, std::move(encode), writable_addr, size)
            .TypedArray();
}

GALLIUM_BINDINGS_PIXENCODER_NS_END
