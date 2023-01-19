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

#include <sstream>

#include "fmt/format.h"

#include "Core/Journal.h"
#include "Utau/AudioMultitrackSinkStream.h"
#include "Utau/AVFilterDAG.h"
UTAU_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Utau.AudioMultitrackSinkStream)

std::unique_ptr<AudioMultitrackSinkStream>
AudioMultitrackSinkStream::Make(const std::shared_ptr<AudioSinkStream>& sink,
                                const std::vector<SampleTriple>& subtrack_triples,
                                const SampleTriple& sink_triple)
{
    if (!sink || subtrack_triples.empty())
        return nullptr;

    auto stream = std::make_unique<AudioMultitrackSinkStream>();
    stream->sink_ = sink;

    std::vector<AVFilterDAG::InBufferParameters> inparams(subtrack_triples.size());
    for (int32_t i = 0; i < subtrack_triples.size(); i++)
    {
        inparams[i].name = fmt::format("in{}", i);
        inparams[i].media_type = MediaType::kAudio;
        inparams[i].channel_mode = subtrack_triples[i].channel_mode;
        inparams[i].sample_fmt = subtrack_triples[i].format;
        inparams[i].sample_rate = subtrack_triples[i].sample_rate;
        stream->subtracks_.push_back(std::make_shared<Subtrack>(subtrack_triples[i]));
    }

    std::vector<AVFilterDAG::OutBufferParameters> outparams(1);
    outparams[0].name = "out";
    outparams[0].media_type = MediaType::kAudio;
    outparams[0].channel_modes = { sink_triple.channel_mode };
    outparams[0].sample_fmts = { sink_triple.format };
    outparams[0].sample_rates = { sink_triple.sample_rate };

    std::ostringstream descriptor_builder;
    for (int32_t i = 0; i < subtrack_triples.size(); i++)
        descriptor_builder << fmt::format("[out{}]", i);

    descriptor_builder << fmt::format(" amix=inputs={}:duration=longest:normalize=1,", subtrack_triples.size());
    descriptor_builder << fmt::format(" aresample=och={}:osr={}:osf={} [out]",
                                      sink_triple.channel_mode == AudioChannelMode::kStereo ? "stereo" : "mono",
                                      sink_triple.sample_rate,
                                      static_cast<int32_t>(SampleFormatToLibavFormat(sink_triple.format)));

    auto filter = AVFilterDAG::MakeFromDSL(descriptor_builder.str(), inparams, outparams);
    if (!filter)
        return nullptr;

    stream->composer_ = std::move(filter);

    return stream;
}

void AudioMultitrackSinkStream::Update()
{
}

bool AudioSubtrackSinkStream::OnConnect(SampleFormat sample_format,
                                        AudioChannelMode channel_mode,
                                        int32_t sample_rate,
                                        bool realtime)
{
    return false;
}

bool AudioSubtrackSinkStream::OnDisconnect()
{
    return false;
}

void AudioSubtrackSinkStream::SetVolume(float volume)
{
}

float AudioSubtrackSinkStream::GetVolume()
{
    return 0;
}

double AudioSubtrackSinkStream::GetDelayInUs()
{
    return 0;
}

bool AudioSubtrackSinkStream::Enqueue(const AudioBuffer& buffer)
{
    return false;
}

void AudioSubtrackSinkStream::OnDispose()
{
}

std::shared_ptr<AudioDevice> AudioSubtrackSinkStream::OnGetDevice()
{
    return nullptr;
}

UTAU_NAMESPACE_END
