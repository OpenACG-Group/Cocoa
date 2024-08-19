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
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/codec/SkCodec.h"

#include "Utau/PixelFormatUtils.h"
#include "Gallium/bindings/multimedia/Frame.h"
#include "Gallium/bindings/multimedia/PixelFrameFactory.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

ffi::RetLocal<v8::Value> PixelFrameFactory::FromEncoded(const ffi::Mem<uint8_t>& data,
                                                        const ffi::IFace<FrameMakeFromEncodedOptions>& options)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    sk_sp<SkData> skdata = SkData::MakeWithoutCopy(data.Address(), data.ByteSize());
    CHECK(skdata);
    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(std::move(skdata));
    if (!codec)
        return ffi::Fail(ffi::kErr, "invalid encoded data");

    if (codec->getOrigin() != SkEncodedOrigin::kTopLeft_SkEncodedOrigin)
    {
        return ffi::Fail(ffi::kErr, "unsupported encoded origin "
            "(note: try using renderer module to decode)");
    }

    if (options->force_decode_to_ycbcr && *options->force_decode_to_ycbcr)
    {
        SkYUVAPixmapInfo::SupportedDataTypes supported_types;
        supported_types.enableDataType(SkYUVAPixmapInfo::DataType::kUnorm8, 1);

        SkYUVAPixmapInfo pixmap_info;
        if (!codec->queryYUVAInfo(supported_types, &pixmap_info))
            return ffi::Fail(ffi::kErr, "failed to decode input into YCbCr format");
        CHECK(pixmap_info.isValid());

        SkYUVAPixmaps pixmaps = SkYUVAPixmaps::Allocate(pixmap_info);
        if (SkCodec::Result result = codec->getYUVAPlanes(pixmaps); result != SkCodec::Result::kSuccess)
        {
            return ffi::Fail(ffi::kErr, fmt::format(
                "failed to decode: {}", SkCodec::ResultToString(result)));
        }

        // There is no enough information of color space, and I just reinterpret it to sRGB,
        // as I have claimed in the documentation.
        AVFrame *frame = utau::MakeFrameFromSkYUVAPixmapsWithCopy(pixmaps,
            AVCOL_PRI_BT709, AVCOL_TRC_IEC61966_2_1);
        if (!frame)
            return ffi::Fail(ffi::kErr, "decoded YCbCr format is not supported");

        return ffi::JSObject::New<Frame>(isolate, frame);
    }

    SkImageInfo info = codec->getInfo();
    bool origin_is_srgb = info.colorSpace()->isSRGB();

    // Adjust image info
    if (options->to_srgb_color_space && *options->to_srgb_color_space)
        info = info.makeColorSpace(SkColorSpace::MakeSRGB());
    if (options->to_linear_gamma && *options->to_linear_gamma)
        info = info.makeColorSpace(info.refColorSpace()->makeLinearGamma());

    AVPixelFormat av_format;
    if (options->format)
    {
        av_format = static_cast<AVPixelFormat>(**options->format);
        auto ct_at_opt = utau::PixelFormatToSkiaTypes(av_format);
        if (!ct_at_opt)
            return ffi::Fail(ffi::kErr, "options.format: incompatible pixel format");

        info = info.makeColorType(std::get<0>(*ct_at_opt));
    }
    else
    {
        if (auto opt = utau::SkiaColorTypesToPixelFormat(info.colorType(), info.alphaType()))
            av_format = *opt;
        else
            return ffi::Fail(ffi::kErr, "incompatile pixel format");
    }

    AVFrame *frame = av_frame_alloc();
    CHECK(frame && "allocation failed");
    frame->width = info.width();
    frame->height = info.height();
    frame->sample_aspect_ratio = {1, 1};
    frame->format = av_format;
    CHECK(av_frame_get_buffer(frame, 0) >= 0 && "allocation failed");

    frame->color_range = AVCOL_RANGE_JPEG;

    // Not YUV format, so we ignore AVFrame::{colorspace, chroma_location}
    SkColorSpace *colorspace = info.colorSpace();

    frame->color_trc = AVCOL_TRC_UNSPECIFIED;
    if (colorspace->gammaIsLinear())
        frame->color_trc = AVCOL_TRC_LINEAR;
    else if (colorspace->gammaCloseToSRGB())
        frame->color_trc = AVCOL_TRC_IEC61966_2_1;

    frame->color_primaries = AVCOL_PRI_UNSPECIFIED;
    if (origin_is_srgb)
    {
        // BT.709 uses the same primaries to sRGB
        frame->color_primaries = AVCOL_PRI_BT709;
    }

    // Reinterpret the color space info if needed, including color primaries and
    // color transfer characteristics (gamma).
    if (options->reinterpret_to_srgb && *options->reinterpret_to_srgb)
    {
        frame->color_primaries = AVCOL_PRI_BT709;
        frame->color_trc = AVCOL_TRC_IEC61966_2_1;
    }

    if (SkCodec::Result res = codec->getPixels(info, frame->data[0], frame->linesize[0]);
        res != SkCodec::Result::kSuccess)
    {
        av_frame_free(&frame);
        return ffi::Fail(ffi::kErr, fmt::format(
            "failed to decode: {}", SkCodec::ResultToString(res)));
    }

    return ffi::JSObject::New<Frame>(isolate, frame);
}

ffi::RetLocal<v8::Value> PixelFrameFactory::FromImage(ffi::Class<renderer::Image> image,
                                                      ffi::Enum<ColorPrimaries> color_primaries,
                                                      ffi::Enum<ColorTransferCharacteristic> color_trc)
{
    if (image->GetDisposeState() != DisposeState::kNot)
        return ffi::Fail(ffi::kErr, "Image object has been disposed");
    sk_sp<SkImage> skimage = image->GetSkImage();
    if (skimage->isTextureBacked())
        return ffi::Fail(ffi::kErr, "texture-backed image is not supported");

    std::optional<AVPixelFormat> opt_format = utau::SkiaColorTypesToPixelFormat(
        skimage->colorType(), skimage->alphaType());
    AVPixelFormat format;
    if (opt_format)
    {
        format = *opt_format;
        CHECK(av_pix_fmt_count_planes(format) == 1);
    }
    else
        return ffi::Fail(ffi::kErr, "format not supported");

    AVFrame *frame = av_frame_alloc();
    CHECK(frame && "allocation failed");

    frame->width = skimage->width();
    frame->height = skimage->height();
    frame->sample_aspect_ratio = {1, 1};
    frame->format = format;
    frame->color_primaries = static_cast<AVColorPrimaries>(*color_primaries);
    frame->color_trc = static_cast<AVColorTransferCharacteristic>(*color_trc);
    frame->color_range = AVCOL_RANGE_JPEG;
    // SkImage cannot be YUV format, so we just leave the following fields
    // UNSPECIFIED.
    frame->colorspace = AVCOL_SPC_UNSPECIFIED;
    frame->chroma_location = AVCHROMA_LOC_UNSPECIFIED;
    CHECK(av_frame_get_buffer(frame, 0) >= 0 && "allocation failed");

    bool result = skimage->readPixels(
        nullptr, skimage->imageInfo(), frame->data[0], frame->linesize[0],
        0, 0, SkImage::CachingHint::kDisallow_CachingHint);

    if (!result)
    {
        av_frame_free(&frame);
        return ffi::Fail(ffi::kErr, "could not read pixels from image");
    }

    return ffi::JSObject::New<Frame>(v8::Isolate::GetCurrent(), frame);
}

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
