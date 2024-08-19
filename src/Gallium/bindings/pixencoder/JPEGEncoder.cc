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

#include "Gallium/bindings/pixencoder/JPEGEncoder.h"
GALLIUM_BINDINGS_PIXENCODER_NS_BEGIN

std::tuple<SkJpegEncoder::Options, sk_sp<SkData>>
JPEGEncoderOptions::Make() const
{
    SkJpegEncoder::Options options;
    sk_sp<SkData> data;
    // Quality must be in [0, 100], and we don't check the value range here.
    // Just let Skia clamp the value.
    if (quality)
        options.fQuality = *quality;
    if (downsample)
        options.fDownsample = **downsample;
    if (alpha_option)
        options.fAlphaOption = **alpha_option;
    if (xmp_metadata)
    {
        data = SkData::MakeWithoutCopy(xmp_metadata->Address(),
                                       xmp_metadata->ByteSize());
        options.xmpMetadata = data.get();
    }
    return {options, data};
}

ffi::RetLocal<v8::Value> JPEGEncoder::EncodePixmap(renderer::PixmapAdapter pixmap,
                                                   ffi::IFace<JPEGEncoderOptions> options_iface)
{
    auto [options, xmp_data] = options_iface->Make();

    SkDynamicMemoryWStream stream;
    if (!SkJpegEncoder::Encode(&stream, *pixmap, options))
        return ffi::Fail(ffi::kErr, "failed to encode pixmap into JPEG format");

    sk_sp<SkData> encode = stream.detachAsData();
    CHECK(encode->unique());
    void *writable_addr = encode->writable_data();
    size_t size = encode->size();

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::Mem<uint8_t>(isolate, std::move(encode), writable_addr, size)
            .TypedArray();
}

ffi::RetLocal<v8::Value>
JPEGEncoder::EncodeImage(ffi::Opt<ffi::Class<renderer::GpuDirectContext>> ctx,
                         ffi::Class<renderer::Image> image,
                         ffi::IFace<JPEGEncoderOptions> options_iface)
{
    auto [options, xmp_data] = options_iface->Make();

    sk_sp<SkData> encode = SkJpegEncoder::Encode(
            ctx ? (*ctx)->GetGrDirectContext().get() : nullptr,
            image->GetSkImage().get(), options);
    if (!encode)
        return ffi::Fail(ffi::kErr, "failed to encode image into PNG format");
    CHECK(encode->unique());
    void *writable_addr = encode->writable_data();
    size_t size = encode->size();

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::Mem<uint8_t>(isolate, std::move(encode), writable_addr, size)
            .TypedArray();
}

GALLIUM_BINDINGS_PIXENCODER_NS_END
