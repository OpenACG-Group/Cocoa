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

#ifndef COCOA_UTAU_VIDEOBUFFER_H
#define COCOA_UTAU_VIDEOBUFFER_H

#include "Utau/AVGenericBuffer.h"
UTAU_NAMESPACE_BEGIN

class VideoBuffer : public AVGenericBuffer
{
public:
    /**
     * Make an `VideoBuffer` instance from an `AVFrame` object obtained
     * from FFmpeg components (e.g. decoders). The `frame` must be reference counted,
     * and it will be cloned, which means it does NOT take the ownership of
     * the original `frame` object that caller passes in. It is safe
     * to free or unref `frame` object during the lifetime of `AudioBuffer`.
     */
    static std::unique_ptr<VideoBuffer> MakeFromAVFrame(UnderlyingPtr frame);

    explicit VideoBuffer(UnderlyingPtr ptr);
    ~VideoBuffer();

    g_nodiscard int64_t GetFramePTS();
};

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_VIDEOBUFFER_H
