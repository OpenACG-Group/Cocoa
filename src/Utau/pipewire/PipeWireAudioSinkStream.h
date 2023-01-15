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

#ifndef COCOA_UTAU_PIPEWIRE_PIPEWIREAUDIOSINKSTREAM_H
#define COCOA_UTAU_PIPEWIRE_PIPEWIREAUDIOSINKSTREAM_H

#include <chrono>
#include <queue>
#include <mutex>

#include <pipewire/pipewire.h>

#include "Utau/AudioSinkStream.h"
#include "Utau/ffwrappers/libavutil.h"
UTAU_NAMESPACE_BEGIN

class PipeWireAudioDevice;

class PipeWireAudioSinkStream : public AudioSinkStream
{
public:
    using time_point = std::chrono::steady_clock::time_point;

    static std::unique_ptr<PipeWireAudioSinkStream> MakeFromDevice(
            const std::shared_ptr<PipeWireAudioDevice>& device,
            const std::string& name);

    PipeWireAudioSinkStream();
    ~PipeWireAudioSinkStream() override;

    struct BufferItem
    {
        AVFrame *frame = nullptr;
        int64_t offset = 0;
    };

    std::shared_ptr<AudioDevice> OnGetDevice() override;
    void OnDispose() override;
    bool OnConnect(SampleFormat sample_format, AudioChannelMode channel_mode,
                   int32_t sample_rate, bool realtime) override;
    bool OnDisconnect() override;

    bool Enqueue(const AudioBuffer &buffer) override;

    double GetDelayInUs() override;
    float GetVolume() override;
    void SetVolume(float volume) override;

    static void Process(void *userdata);
    static void OnControlInfo(void *userdata, uint32_t id, const pw_stream_control *ctl);

private:
    bool HasExpiredBuffer();
    BufferItem& GetExpiredBuffer();
    void CurrentBufferConsumed();

    bool                                        disposed_;
    std::shared_ptr<PipeWireAudioDevice>        device_;
    pw_stream                                  *pw_stream_;
    SampleFormat                                sample_format_;
    AudioChannelMode                            channel_mode_;
    int32_t                                     sample_rate_;
    std::mutex                                  queue_lock_;
    std::queue<BufferItem>                      queue_;
    BufferItem                                  current_buffer_;
    int64_t                                     current_queued_samples_;
    double                                      delay_in_us_;
    float                                       volume_;
};

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_PIPEWIRE_PIPEWIREAUDIOSINKSTREAM_H
