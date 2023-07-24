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

#include <cstdint>

#include "Core/Errors.h"
#include "Utau/ffwrappers/libavutil.h"
#include "Utau/VideoBuffer.h"
UTAU_NAMESPACE_BEGIN

std::unique_ptr<VideoBuffer> VideoBuffer::MakeFromAVFrame(UnderlyingPtr opaque)
{
    CHECK(opaque);

    auto *frame = reinterpret_cast<AVFrame*>(opaque);
    CHECK(frame->width > 0 && frame->height > 0);

    VideoColorInfo color_info(static_cast<AVPixelFormat>(frame->format),
                              frame->color_primaries,
                              frame->color_trc,
                              frame->colorspace,
                              frame->color_range,
                              frame->chroma_location);

    static_assert(AV_NUM_DATA_POINTERS >= VideoColorInfo::kMaxPlanes);
    VideoBufferInfo info(frame->width,
                         frame->height,
                         frame->linesize,
                         static_cast<VideoBufferInfo::FrameType>(frame->pict_type),
                         color_info);

    return std::make_unique<VideoBuffer>(opaque, info);
}

VideoBuffer::VideoBuffer(UnderlyingPtr ptr,
                         const VideoBufferInfo& info)
    : AVGenericBuffer(ptr)
    , info_(info)
{
}

VideoBuffer::~VideoBuffer() = default;

uint8_t *VideoBuffer::GetAddress(int plane)
{
    int32_t num_planes = info_.GetColorInfo().GetPlanesCount();
    CHECK(plane >= 0 && plane < num_planes);

    uint8_t *ptr = CastUnderlyingPointer<AVFrame>()->data[plane];
    CHECK(ptr && "Unexpected null-pointer, ffmpeg error");

    return ptr;
}

UTAU_NAMESPACE_END
