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

#ifndef COCOA_UTAU_AUDIOSINKSTREAM_H
#define COCOA_UTAU_AUDIOSINKSTREAM_H

#include <functional>

#include "Core/Errors.h"
#include "Utau/Utau.h"
#include "Utau/ffwrappers/libavutil.h"
UTAU_NAMESPACE_BEGIN

class AudioDevice;
class AudioBuffer;

class AudioSinkStream
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void OnVolumeChanged(const std::vector<float>& volume) = 0;
        virtual void OnEmptyQueue(uint64_t last_frame_id) = 0;
    };

    virtual ~AudioSinkStream() = default;

    virtual std::shared_ptr<AudioDevice> GetDevice() = 0;

    virtual void Dispose() = 0;

    virtual std::shared_ptr<Listener> GetListener() const = 0;
    virtual void SetListener(std::shared_ptr<Listener> listener) = 0;

    virtual uint64_t Enqueue(const AVFrame *frame) = 0;

    virtual double GetDelayInUs() = 0;
    virtual void SetVolume(const std::vector<float>& volume) = 0;
};

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_AUDIOSINKSTREAM_H
