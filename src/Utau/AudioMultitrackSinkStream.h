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

#ifndef COCOA_UTAU_AUDIOMULTITRACKSINKSTREAM_H
#define COCOA_UTAU_AUDIOMULTITRACKSINKSTREAM_H

#include <queue>
#include <mutex>

#include "Core/Errors.h"
#include "Utau/Utau.h"
#include "Utau/AudioSinkStream.h"
UTAU_NAMESPACE_BEGIN

class AudioDevice;
class AudioSubtrackSinkStream;
class AVFilterDAG;

class AudioMultitrackSinkStream
{
public:
    using Subtrack = AudioSubtrackSinkStream;

    static std::unique_ptr<AudioMultitrackSinkStream> Make(const std::shared_ptr<AudioSinkStream>& sink,
                                                           const std::vector<SampleTriple>& subtrack_triples,
                                                           const SampleTriple& sink_triple);

    g_nodiscard g_inline std::shared_ptr<Subtrack> GetSubtrack(int32_t n) const {
        CHECK(n < subtracks_.size());
        return subtracks_[n];
    }

    void Update();

private:
    std::shared_ptr<AudioSinkStream> sink_;
    std::vector<std::shared_ptr<Subtrack>> subtracks_;
    std::unique_ptr<AVFilterDAG> composer_;
};

class AudioSubtrackSinkStream : public AudioSinkStream
{
    friend class AudioMultitrackSinkStream;

public:
    explicit AudioSubtrackSinkStream(const SampleTriple& triple)
        : sample_triple_(triple) {}

    bool OnConnect(SampleFormat sample_format, AudioChannelMode channel_mode,
                   int32_t sample_rate, bool realtime) override;
    bool OnDisconnect() override;
    void OnDispose() override;
    std::shared_ptr<AudioDevice> OnGetDevice() override;
    bool Enqueue(const AudioBuffer &buffer) override;
    void SetVolume(float volume) override;
    float GetVolume() override;
    double GetDelayInUs() override;

private:
    SampleTriple sample_triple_;
    std::mutex queue_lock_;
    std::queue<std::shared_ptr<AudioBuffer>> queue_;
};

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_AUDIOMULTITRACKSINKSTREAM_H
