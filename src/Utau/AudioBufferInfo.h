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

#ifndef COCOA_UTAU_AUDIOBUFFERINFO_H
#define COCOA_UTAU_AUDIOBUFFERINFO_H

#include "Core/Errors.h"
#include "Utau/Utau.h"
UTAU_NAMESPACE_BEGIN

class AudioBufferInfo
{
public:
    AudioBufferInfo(AudioChannelMode channel_mode,
                    SampleFormat sample_format,
                    int sample_rate,
                    int samples_count)
        : channels_(channel_mode == AudioChannelMode::kStereo ? 2 : 1)
        , channel_mode_(channel_mode)
        , sample_format_(sample_format)
        , sample_rate_(sample_rate)
        , samples_count_(samples_count)
    {
    }

    ~AudioBufferInfo() = default;

    g_nodiscard g_inline int GetChannels() const {
        return channels_;
    }

    g_nodiscard g_inline AudioChannelMode GetChannelMode() const {
        return channel_mode_;
    }

    g_nodiscard g_inline SampleFormat GetSampleFormat() const {
        return sample_format_;
    }

    g_nodiscard g_inline int GetSampleRate() const {
        return sample_rate_;
    }

    g_nodiscard g_inline int GetSamplesCount() const {
        return samples_count_;
    }

    g_nodiscard g_inline size_t ComputeTotalBufferSize() const {
        int per_sample_size = GetPerSampleSize(sample_format_);
        return (per_sample_size * samples_count_ * channels_);
    }

    g_nodiscard g_inline size_t ComputePerPlanarBufferSize() const {
        CHECK(SampleFormatIsPlanar(sample_format_));
        int per_sample_size = GetPerSampleSize(sample_format_);
        return (per_sample_size * samples_count_);
    }

    g_nodiscard g_inline int GetPlanesCount() const {
        CHECK(SampleFormatIsPlanar(sample_format_));
        return channels_;
    }

    g_nodiscard g_inline bool IsPlanarFormat() const {
        return SampleFormatIsPlanar(sample_format_);
    }

    g_nodiscard g_inline int GetNeededBuffersCount() const {
        if (this->IsPlanarFormat())
            return this->GetChannels();
        return 1;
    }

private:
    int                     channels_;
    AudioChannelMode        channel_mode_;
    SampleFormat            sample_format_;
    int                     sample_rate_;
    int                     samples_count_;
};

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_AUDIOBUFFERINFO_H
