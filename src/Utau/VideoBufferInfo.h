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

#ifndef COCOA_UTAU_VIDEOBUFFERINFO_H
#define COCOA_UTAU_VIDEOBUFFERINFO_H

#include "Utau/Utau.h"
#include "Utau/ffwrappers/libavutil.h"
#include "Utau/AVGenericBuffer.h"
UTAU_NAMESPACE_BEGIN

class VideoColorInfo
{
public:
    static constexpr int32_t kMaxPlanes = 8;

    VideoColorInfo(AVPixelFormat format,
                   AVColorPrimaries color_primaries,
                   AVColorTransferCharacteristic color_transfer_chara,
                   AVColorSpace color_space,
                   AVColorRange color_range,
                   AVChromaLocation chroma_location);

    g_nodiscard g_inline AVPixelFormat GetFormat() const {
        return format_;
    }

    g_nodiscard g_inline AVColorTransferCharacteristic GetColorTransferCharacteristic() const {
        return color_transfer_chara_;
    }

    g_nodiscard g_inline AVColorRange GetColorRange() const {
        return color_range_;
    }

    g_nodiscard g_inline AVColorSpace GetColorSpace() const {
        return color_space_;
    }

    g_nodiscard g_inline AVChromaLocation GetChromaLocation() const {
        return chroma_location_;
    }

    g_nodiscard g_inline AVColorPrimaries GetColorPrimaries() const {
        return color_primaries_;
    }

    g_nodiscard const char *GetFormatName() const;
    g_nodiscard int GetFormatComponents() const;
    g_nodiscard bool FormatHasPalette() const;
    g_nodiscard bool FormatIsHWAccel() const;
    g_nodiscard bool FormatIsPlanar() const;
    g_nodiscard bool FormatIsRGBLike() const;
    g_nodiscard bool FormatHasAlpha() const;
    g_nodiscard bool FormatIsBayer() const;
    g_nodiscard bool FormatIsFloat() const;

    g_nodiscard int GetBitsPerPixel() const;

    g_nodiscard int GetPlanesCount() const;

private:
    // Geometry and format info
    AVPixelFormat   format_;
    const AVPixFmtDescriptor *format_desc_;

    // Colorspace info
    AVColorPrimaries    color_primaries_;
    AVColorTransferCharacteristic color_transfer_chara_;
    AVColorSpace        color_space_;
    AVColorRange        color_range_;
    AVChromaLocation    chroma_location_;
};

class VideoBufferInfo
{
public:
    enum class FrameType
    {
        kNone = AV_PICTURE_TYPE_NONE,
        kI = AV_PICTURE_TYPE_I,
        kP = AV_PICTURE_TYPE_P,
        kB = AV_PICTURE_TYPE_B,
        kS = AV_PICTURE_TYPE_S,
        kSI = AV_PICTURE_TYPE_SI,
        kSP = AV_PICTURE_TYPE_SP,
        kBI = AV_PICTURE_TYPE_BI
    };

    VideoBufferInfo(int32_t width,
                    int32_t height,
                    const int32_t strides[VideoColorInfo::kMaxPlanes],
                    FrameType frame_type,
                    const VideoColorInfo& color_info);
    ~VideoBufferInfo() = default;

    g_nodiscard g_inline int32_t GetWidth() const {
        return width_;
    }

    g_nodiscard g_inline int32_t GetHeight() const {
        return height_;
    }

    g_nodiscard g_inline FrameType GetFrameType() const {
        return frame_type_;
    }

    g_nodiscard g_inline const VideoColorInfo& GetColorInfo() const {
        return color_info_;
    }

    g_nodiscard g_inline int32_t GetStride(int plane) const {
        CHECK(plane >= 0 && plane < color_info_.GetPlanesCount());
        return strides_[plane];
    }

private:
    int32_t             width_;
    int32_t             height_;
    int32_t             strides_[VideoColorInfo::kMaxPlanes];
    FrameType           frame_type_;
    VideoColorInfo      color_info_;
};

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_VIDEOBUFFERINFO_H
