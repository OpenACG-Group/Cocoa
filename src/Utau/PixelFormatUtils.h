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

#ifndef COCOA_UTAU_SKIAIMAGE_H
#define COCOA_UTAU_SKIAIMAGE_H

#include <optional>

#include "include/core/SkSamplingOptions.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkImage.h"
#include "include/core/SkYUVAPixmaps.h"

#include "Utau/ffwrappers/libavutil-frame.h"
#include "Utau/Utau.h"
UTAU_NAMESPACE_BEGIN

std::optional<std::tuple<SkColorType, SkAlphaType>> PixelFormatToSkiaTypes(AVPixelFormat format);
std::optional<AVPixelFormat> SkiaColorTypesToPixelFormat(SkColorType ct, SkAlphaType at);

/**
 * Create a Skia colorspace from the speicifed `primaries` and `trc`.
 * `primaries` defines the color gamut: three points and a white point in the
 * IEC 1931 xy coordinates. `trc` defines the gamma transfer function.
 *
 * Supported color primaries are: BT709, SMPTE432, BT2020, SMPTE170M, SMPTE240M,
 * BT470M, BT470BG (BT610), SMPTE431, EBU3213
 *
 * Supported trc are: BT709, GAMMA22, GAMMA28, SMPTE170M, SMPTE240M, LINEAR,
 * IEC61966-2-1 (sRGB), BT2020
 *
 * Returns `fallback` if either the primaries or trc is not supported.
 */
sk_sp<SkColorSpace> ResolveFrameColorCharacteristicsToSkColorSpace(AVColorPrimaries primaries,
                                                                   AVColorTransferCharacteristic trc,
                                                                   const sk_sp<SkColorSpace>& fallback);

/**
 * Converts the combination of `AVPixelFormat`, `AVColorSpace`, `AVColorRange` parameters
 * to the corresponding `SkYUVAInfo` structure.
 * Returns an invalid `SkYUVAInfo` for unsupported combinations.
 */
SkYUVAInfo ResolveFrameSkYUVAInfo(const SkISize& dimensions,
                                  AVPixelFormat format, AVColorSpace spc, AVColorRange range);

enum class FrameBlitErrorCode : uint32_t
{
    // Internal, never used by any public interfaces
    kUndefined,

    kSuccess,
    kSrcUnsupportedSAR,
    kSrcUnsupportedFormat,
    kSrcFrameInvalid,
    kSrcRegionOutOfRange,
    kDstPixmapInvalid,
    kDstRegionOutOfRange,
    kIncompatibleColorSpace,
    kConversionFailure
};

/**
 * Blit the pixels in `src_frame` to `dst_pixmap`, with necessary scaling and conversions
 * of format and colorspace.
 *
 * Note that the scaling is always gamma-correct.
 *
 * @param src_frame         Source frame from which the pixels are read.
 * @param src_region        Specifies a region of source frame to read pixels.
 * @param dst_pixmap        Destination pixmap to which the pixels are written. This pixmap must
 *                          has valid SkColorType, SkAlphaType, dimensions, and a valid address of
 *                          pixel buffer. If the pixmap also contains a valid SkColorSpace, it is
 *                          guranteed that pixels written to the pixmap are in that colorspace.
 *                          Otherwise, pixels written to the pixmap are in the colorspace of source
 *                          frame, and we will set an SkColorSpace resolved from `src_frame` for
 *                          the pixmap (using `dst_pixmap.setColorSpace()`).
 * @param dst_region        Specifies a region of destination pixmap to write pixels.
 * @param scale_resampler   Specifies the sampling method when scaling pixels.
 * @return
 */
FrameBlitErrorCode BlitFrameToSkPixmap(const AVFrame *src_frame, const SkIRect& src_region,
                                       SkPixmap& dst_pixmap, const SkIRect& dst_region,
                                       ScaleResampler scale_resampler);

sk_sp<SkImage> BlitFrameToSkImage(const AVFrame *src_frame, const SkIRect& src_region,
                                  const SkImageInfo& dst_image_info,
                                  ScaleResampler scale_resampler,
                                  FrameBlitErrorCode *status_code = nullptr);

SkYUVAPixmaps WrapFrameToSkYUVAPixmaps(const AVFrame *frame);

/**
 * Create an `AVFrame` that follows the YUVA format of `pixmaps`, and copy pixels from `pixmaps`.
 * `color_primaries` and `color_trc` specifies the additional color information, and will only affect
 * the corresponding fields (`AVFrame::color_primaries` and `AVFrame::color_trc`) of the result.
 */
AVFrame *MakeFrameFromSkYUVAPixmapsWithCopy(const SkYUVAPixmaps& pixmaps,
                                            AVColorPrimaries color_primaries,
                                            AVColorTransferCharacteristic color_trc);

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_SKIAIMAGE_H
