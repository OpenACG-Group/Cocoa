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

#include <cstring>

#include "Core/Errors.h"

#include "Utau/ffwrappers/pixfmt.h"
#include "Utau/Utau.h"
#include "Utau/VideoBufferInfo.h"
UTAU_NAMESPACE_BEGIN

VideoColorInfo::VideoColorInfo(AVPixelFormat format,
                               AVColorPrimaries color_primaries,
                               AVColorTransferCharacteristic color_transfer_chara,
                               AVColorSpace color_space,
                               AVColorRange color_range,
                               AVChromaLocation chroma_location)
    : format_(format)
    , format_desc_(nullptr)
    , color_primaries_(color_primaries)
    , color_transfer_chara_(color_transfer_chara)
    , color_space_(color_space)
    , color_range_(color_range)
    , chroma_location_(chroma_location)
{
    CHECK(format != AV_PIX_FMT_NONE && format < AV_PIX_FMT_NB);
    format_desc_ = av_pix_fmt_desc_get(format);
    CHECK(format_desc_);
}

const char *VideoColorInfo::GetFormatName() const
{
    return format_desc_->name;
}

int VideoColorInfo::GetFormatComponents() const
{
    return format_desc_->nb_components;
}

#define FORMAT_FLAG_TEST_FUNC(func, flag) \
    bool VideoColorInfo::func() const { return format_desc_->flags & flag; }

FORMAT_FLAG_TEST_FUNC(FormatHasPalette, AV_PIX_FMT_FLAG_PAL)
FORMAT_FLAG_TEST_FUNC(FormatIsHWAccel, AV_PIX_FMT_FLAG_HWACCEL)
FORMAT_FLAG_TEST_FUNC(FormatIsPlanar, AV_PIX_FMT_FLAG_PLANAR)
FORMAT_FLAG_TEST_FUNC(FormatIsRGBLike, AV_PIX_FMT_FLAG_RGB)
FORMAT_FLAG_TEST_FUNC(FormatIsBayer, AV_PIX_FMT_FLAG_BAYER)
FORMAT_FLAG_TEST_FUNC(FormatHasAlpha, AV_PIX_FMT_FLAG_ALPHA)
FORMAT_FLAG_TEST_FUNC(FormatIsFloat, AV_PIX_FMT_FLAG_FLOAT)

#undef FORMAT_FLAG_TEST_FUNC

int VideoColorInfo::GetBitsPerPixel() const
{
    return av_get_bits_per_pixel(format_desc_);
}

int VideoColorInfo::GetPlanesCount() const
{
    return av_pix_fmt_count_planes(format_);
}

VideoBufferInfo::VideoBufferInfo(int32_t width,
                                 int32_t height,
                                 const int32_t strides[VideoColorInfo::kMaxPlanes],
                                 FrameType frame_type,
                                 const VideoColorInfo &color_info)
    : width_(width)
    , height_(height)
    , strides_{}
    , frame_type_(frame_type)
    , color_info_(color_info)
{
    std::memset(strides_, 0, sizeof(strides_));
    for (int32_t i = 0; i < color_info_.GetPlanesCount(); i++)
        strides_[i] = strides[i];
}

UTAU_NAMESPACE_END
