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

#ifndef COCOA_UTAU_PIPEWIREAUDIOPLAYBACKSTREAM_H
#define COCOA_UTAU_PIPEWIREAUDIOPLAYBACKSTREAM_H

#include <pipewire/pipewire.h>

#include "Utau/Utau.h"
#include "Utau/AudioPlaybackStream.h"
UTAU_NAMESPACE_BEGIN

class PipewireAudioServiceProvider;

class PipewireAudioPlaybackStream : public AudioPlaybackStream
{
public:
    using AudioPlaybackStream::AudioPlaybackStream;
    ~PipewireAudioPlaybackStream() override;

    static std::shared_ptr<PipewireAudioPlaybackStream> Make(const std::shared_ptr<PipewireAudioServiceProvider>& provider,
                                                             const std::string& name,
                                                             MediaRole role,
                                                             int channels,
                                                             SampleFormat sample_format,
                                                             int sample_rate);

    void OnDispose() override;
    void OnInterruptCurrentBuffer() override;
    void OnBufferEnqueued() override;

private:
    pw_loop         *pipewire_loop_;
    pw_stream       *pipewire_stream_;
};

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_PIPEWIREAUDIOPLAYBACKSTREAM_H
