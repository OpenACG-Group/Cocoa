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

#include "Core/Journal.h"
#include "Utau/Utau.h"
#include "Utau/AudioBuffer.h"
#include "Utau/ffwrappers/libavutil.h"
UTAU_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Utau.AudioBuffer)

std::unique_ptr<AudioBuffer> AudioBuffer::MakeFromAVFrame(UnderlyingPtr ptr)
{
    if (!ptr)
        return nullptr;

    auto *frame = reinterpret_cast<AVFrame*>(ptr);

    SampleFormat format = LibavFormatToSampleFormat(
            static_cast<AVSampleFormat>(frame->format));
    if (format == SampleFormat::kUnknown)
    {
        QLOG(LOG_ERROR, "Failed to create AudioBuffer: Unknown sample format or not an audio frame");
        return nullptr;
    }

    AudioChannelMode channel_mode;
    int channels = frame->ch_layout.nb_channels;
    if (channels == 1)
        channel_mode = AudioChannelMode::kMono;
    else if (channels == 2)
        channel_mode = AudioChannelMode::kStereo;
    else
    {
        QLOG(LOG_ERROR, "Failed to create AudioBuffer: Unsupported channel layout");
        return nullptr;
    }

    AudioBufferInfo info(channel_mode, format, frame->sample_rate, frame->nb_samples);
    return std::make_unique<AudioBuffer>(ptr, info);
}

AudioBuffer::AudioBuffer(UnderlyingPtr ptr, const AudioBufferInfo& info)
    : AVGenericBuffer(ptr)
    , info_(info)
{
}

AudioBuffer::~AudioBuffer() = default;

uint8_t *AudioBuffer::GetAddress(int plane)
{
    auto *frame = CastUnderlyingPointer<AVFrame>();
    CHECK(frame->data[plane] && "Invalid plane index for current format");
    return frame->data[plane];
}

UTAU_NAMESPACE_END
