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

#include "Gallium/bindings/multimedia/ffwrappers/libavutil.h"
#include "Utau/PixelFormatUtils.h"
#include "Gallium/bindings/renderer/Image.h"
#include "Gallium/bindings/multimedia/PixelFrameView.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

PixelFrameView::PixelFrameView(const ffi::Class<Frame>& frame)
    : proxy_(frame)
{
}

#define TRY_RETRIEVE_HANDLE(var) \
    AVFrame *var = proxy_.TryRetrieveHandle();                                 \
    if (!var) {                                                                \
        return ffi::Fail(ffi::kErr, "the `Frame` object has been disposed");   \
    }

#define FAIL_ERR_FMT(fmtstr, ...) \
    ffi::Fail(ffi::kErr, fmt::format(fmtstr __VA_OPT__(,) __VA_ARGS__))

ffi::Ret<bool> PixelFrameView::isWritable()
{
    TRY_RETRIEVE_HANDLE(f)
    return av_frame_is_writable(f);
}

ffi::Ret<void> PixelFrameView::makeWritable()
{
    TRY_RETRIEVE_HANDLE(f)
    if (int res = av_frame_make_writable(f); res < 0)
        return FAIL_ERR_FMT("failed to make the frame writable: {}", av_err2str(res));
    return {};
}

ffi::Ret<void> PixelFrameView::writeImageFrom(const std::vector<v8::Local<v8::Uint8Array>>& src_buffers,
                                              const std::vector<int32_t>& src_row_bytes)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    TRY_RETRIEVE_HANDLE(frame)
    const AVPixelFormat format = static_cast<AVPixelFormat>(frame->format);
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(format);
    CHECK(desc);

    const int32_t nb_planes = av_pix_fmt_count_planes(format);
    CHECK(nb_planes <= 4);
    if (src_buffers.size() != nb_planes || src_row_bytes.size() != nb_planes)
    {
        return ffi::Fail(ffi::kErr, "the number of provided buffers/rowBytes"
            " does not match the number of planes");
    }

    int min_linesizes[4];
    CHECK(av_image_fill_linesizes(min_linesizes, format, frame->width) >= 0);

    const uint8_t *src_ptrs[4];
    for (int32_t i = 0; i < src_buffers.size(); i++)
    {
        if (src_row_bytes[i] < min_linesizes[i])
        {
            return ffi::Fail(ffi::kErr, fmt::format(
                "srcRowBytes[{}]: insufficient row bytes for the plane", i));
        }
        ffi::Ret<ffi::Mem<uint8_t>> mem = ffi::Cast<ffi::Mem<uint8_t>>::From(isolate, src_buffers[i]);
        if (mem.HasError())
        {
            return ffi::Fail(ffi::kErr, fmt::format(
                "srcBuffers[{}]: {}", i, mem.GetError().message));
        }
        if (mem.Extract().ByteSize() < src_row_bytes[i] * frame->height)
        {
            return ffi::Fail(ffi::kErr, fmt::format(
                "srcBuffers[{}]: insufficient buffer size for the plane", i));
        }
        src_ptrs[i] = mem.Extract().Address();
    }

    av_image_copy(
        frame->data, frame->linesize,
        src_ptrs, src_row_bytes.data(),
        format, frame->width, frame->height
    );
    return {};
}

ffi::Ret<void> PixelFrameView::readPlaneRectTo(int32_t plane, const renderer::RectAdapter& src_rect_adapter,
                                               const ffi::Mem<uint8_t>& dst, int32_t dst_row_bytes)
{
    if ((*src_rect_adapter).isEmpty())
        return {};
    if (!(*src_rect_adapter).isFinite())
        return ffi::Fail(ffi::kRangeErr, "infinite boundary is not allowed");

    TRY_RETRIEVE_HANDLE(frame)
    AVPixelFormat pixfmt = static_cast<AVPixelFormat>(frame->format);
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(pixfmt);
    CHECK(desc && "invalid pixel format");
    if (int nb_planes = av_pix_fmt_count_planes(pixfmt); plane >= nb_planes)
    {
        return ffi::Fail(ffi::kRangeErr, fmt::format(
            "plane index is out of range (idx #{} of {} planes)", plane, nb_planes));
    }

    if (desc->flags & AV_PIX_FMT_FLAG_BITSTREAM || desc->flags & AV_PIX_FMT_FLAG_PAL)
    {
        return ffi::Fail(ffi::kErr, fmt::format(
            "unsupported pixel format {}, maybe it is bitstream or has palettes", desc->name));
    }

    // Bytes per pixel of CURRENT PLANE
    int bytes_per_pixel = -1;
    for (int comp = 0; comp < desc->nb_components; comp++)
    {
        if (desc->comp[comp].plane != plane)
            continue;

        if (bytes_per_pixel < 0)
            bytes_per_pixel = desc->comp[comp].step;
        else
        {
            // All the components of the same plane must have the same step in bytes
            CHECK(bytes_per_pixel == desc->comp[comp].step);
        }
    }

    // Compute start address and check size
    SkISize plane_dimensions = SkISize::Make(frame->width, frame->height);
    bool plane_is_alpha = (desc->flags & (AV_PIX_FMT_FLAG_ALPHA & AV_PIX_FMT_FLAG_PLANAR)) &&
                           desc->comp[desc->nb_components - 1].plane == plane;
    if (!plane_is_alpha && (desc->log2_chroma_w > 0 || desc->log2_chroma_h > 0) && plane > 0)
    {
        plane_dimensions = SkISize::Make(AV_CEIL_RSHIFT(frame->width, desc->log2_chroma_w),
                                         AV_CEIL_RSHIFT(frame->height, desc->log2_chroma_h));
    }

    SkIRect src_rect = (*src_rect_adapter).round();
    if (!SkIRect::MakeSize(plane_dimensions).contains(src_rect))
        return ffi::Fail(ffi::kRangeErr, "source rect is out of boundary");

    const uint8_t *src_addr = frame->data[plane]
                            + static_cast<uint64_t>(src_rect.y()) * frame->linesize[plane]
                            + static_cast<uint64_t>(src_rect.x()) * bytes_per_pixel;

    // Check size of destination buffer
    if (dst_row_bytes < src_rect.width() * bytes_per_pixel ||
        dst.ByteSize() < src_rect.height64() * dst_row_bytes)
    {
        return ffi::Fail(ffi::kRangeErr, "insufficient size for destination buffer");
    }

    av_image_copy_plane(
        dst.Address(),
        dst_row_bytes,
        src_addr,
        frame->linesize[plane],
        bytes_per_pixel * src_rect.width(),
        src_rect.height()
    );
    return {};
}

ffi::RetLocal<v8::Value>
PixelFrameView::resolveColorSpace(const ffi::Opt<ffi::Class<renderer::ColorSpace>>& fallback)
{
    TRY_RETRIEVE_HANDLE(frame)
    sk_sp<SkColorSpace> colorspace = utau::ResolveFrameColorCharacteristicsToSkColorSpace(
        frame->color_primaries, frame->color_trc,
        fallback ? (*fallback)->GetColorSpace() : nullptr
    );
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!colorspace)
        return v8::Null(isolate);
    return ffi::JSObject::New<renderer::ColorSpace>(isolate, colorspace);
}

ffi::RetLocal<v8::Value> PixelFrameView::blitToPixmap(const renderer::RectAdapter& src_region,
                                                      const renderer::RectAdapter& dst_region,
                                                      ffi::Enum<utau::ScaleResampler> scale_resampler,
                                                      const renderer::PixmapAdapter& dst)
{
    TRY_RETRIEVE_HANDLE(frame)
    SkPixmap dst_pixmap = dst.pixmap;
    utau::FrameBlitErrorCode status = utau::BlitFrameToSkPixmap(
        frame, (*src_region).round(), dst_pixmap,
        (*dst_region).round(), *scale_resampler
    );
    if (status != utau::FrameBlitErrorCode::kSuccess)
        return ffi::Fail(ffi::kErr, "failed to blit frame to pixmap");

    return ffi::JSObject::New<renderer::ColorSpace>(v8::Isolate::GetCurrent(), dst_pixmap.refColorSpace());
}

ffi::RetLocal<v8::Value> PixelFrameView::wrapToImage()
{
    TRY_RETRIEVE_HANDLE(frame)
    if (frame->color_range != AVCOL_RANGE_JPEG)
        return ffi::Fail(ffi::kErr, "color range must be JPEG (full range)");

    auto color_info = utau::PixelFormatToSkiaTypes(static_cast<AVPixelFormat>(frame->format));
    if (!color_info)
        return ffi::Fail(ffi::kErr, "unsupported pixel format (hint: use blitToPixmap to convert)");

    auto [color_type, alpha_type] = *color_info;
    sk_sp<SkColorSpace> color_space = utau::ResolveFrameColorCharacteristicsToSkColorSpace(
        frame->color_primaries, frame->color_trc, nullptr);
    if (!color_space)
        return ffi::Fail(ffi::kErr, "unsupported color space");

    SkImageInfo image_info = SkImageInfo::Make(frame->width, frame->height, color_type, alpha_type, color_space);

    // Frame becomes not writable here. It satisfies the Skia's requirements
    // of pixel sharing (immutable buffer contents for `SkImage`).
    AVFrame *external_own = av_frame_clone(frame);
    CHECK(external_own && "allocation failed");

    sk_sp<SkImage> image = SkImages::RasterFromPixmap(
        {image_info, frame->data[0], static_cast<size_t>(frame->linesize[0])},
        [](const void *pixels, void *userdata) {
            AVFrame *frame = static_cast<AVFrame*>(userdata);
            CHECK(pixels == frame->data[0]);
            av_frame_free(&frame);
        },
        external_own
    );
    if (!image)
    {
        // Skia does not call the rasterReleaseProc on failure
        av_frame_free(&external_own);
        return ffi::Fail(ffi::kErr, "failed to create a renderer.Image");
    }

    return ffi::JSObject::New<renderer::Image>(v8::Isolate::GetCurrent(), image);
}

ffi::RetLocal<v8::Value> PixelFrameView::applyCropping()
{
    TRY_RETRIEVE_HANDLE(frame)
    AVFrame *cropped = av_frame_clone(frame);
    CHECK(cropped && "allocation failed");

    if (int res = av_frame_apply_cropping(cropped, 0); res < 0)
    {
        return ffi::Fail(ffi::kErr, fmt::format(
            "failed to apply cropping settings: {}", av_err2str(res)));
    }
    return ffi::JSObject::New<Frame>(v8::Isolate::GetCurrent(), cropped);
}

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
