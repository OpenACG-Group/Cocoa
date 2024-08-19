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

class PipeWireAudioSinkStream : public AudioSinkStream,
                                public std::enable_shared_from_this<PipeWireAudioSinkStream>
{
public:
    using time_point = std::chrono::steady_clock::time_point;

    static std::shared_ptr<PipeWireAudioSinkStream> MakeFromDevice(
        const std::shared_ptr<PipeWireAudioDevice>& device,
        const std::string& name,
        AVSampleFormat sample_format,
        int32_t sample_rate,
        const AVChannelLayout& ch_layout,
        bool realtime
    );

    PipeWireAudioSinkStream();
    ~PipeWireAudioSinkStream() override;

    struct BufferItem
    {
        uint64_t id = 0;
        AVFrame *frame = nullptr;
        int64_t offset = 0;
    };

    std::shared_ptr<AudioDevice> GetDevice() override;

    void Dispose() override;

    std::shared_ptr<Listener> GetListener() const override;
    void SetListener(std::shared_ptr<Listener> listener) override;

    uint64_t Enqueue(const AVFrame *frame) override;

    double GetDelayInUs() override;
    void SetVolume(const std::vector<float>& volume) override;

    static void Process(void *userdata);
    static void OnControlInfo(void *userdata, uint32_t id, const pw_stream_control *ctl);

private:
    bool ConnectToStream(AVSampleFormat sample_format,
                         int32_t sample_rate,
                         const AVChannelLayout& ch_layout,
                         bool realtime);
    void DisconnectStream();
    void DoDispose();

    bool HasExpiredBuffer();
    BufferItem& GetExpiredBuffer();
    void CurrentBufferConsumed();

    uint64_t                                    frame_id_cnt_;
    bool                                        disposed_;
    std::shared_ptr<Listener>                   listener_;
    std::shared_ptr<PipeWireAudioDevice>        device_;
    pw_stream                                  *pw_stream_;
    AVSampleFormat                              sample_format_;
    AVChannelLayout                             ch_layout_;
    int32_t                                     sample_rate_;
    std::mutex                                  queue_lock_;
    std::queue<BufferItem>                      queue_;
    BufferItem                                  current_buffer_;
    int64_t                                     current_queued_samples_;
    double                                      delay_in_us_;
};

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_PIPEWIRE_PIPEWIREAUDIOSINKSTREAM_H
