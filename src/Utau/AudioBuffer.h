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

#ifndef COCOA_UTAU_AUDIOBUFFER_H
#define COCOA_UTAU_AUDIOBUFFER_H

#include "Utau/Utau.h"
#include "Utau/AudioBufferInfo.h"
#include "Utau/AVGenericBuffer.h"
UTAU_NAMESPACE_BEGIN

class AudioBuffer : public AVGenericBuffer
{
public:
    constexpr static int kMaxPlanesCount = 8;

    /**
     * Make an `AudioBuffer` instance from an `AVFrame` object obtained
     * from FFmpeg components (e.g. decoders). The `frame` must be reference counted,
     * and it will be cloned, which means it does NOT take the ownership of
     * the original `frame` object that caller passes in. It is safe
     * to free or unref `frame` object during the lifetime of `AudioBuffer`.
     */
    static std::unique_ptr<AudioBuffer> MakeFromAVFrame(UnderlyingPtr frame);

    AudioBuffer(UnderlyingPtr ptr, const AudioBufferInfo& info);
    ~AudioBuffer();

    g_nodiscard g_inline const AudioBufferInfo& GetInfo() const {
        return info_;
    }

    g_nodiscard uint8_t *GetAddress(int plane);

private:
    AudioBufferInfo     info_;
};

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_AUDIOBUFFER_H
