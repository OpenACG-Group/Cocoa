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

#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>
#include <spa/support/loop.h>
#include <spa/param/props.h>

#include "Core/Journal.h"
#include "Utau/AudioBuffer.h"
#include "Utau/pipewire/PipeWireAudioSinkStream.h"
#include "Utau/pipewire/PipeWireAudioDevice.h"
UTAU_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Utau.pipewire.PipeWireAudioSinkStream)

namespace {

#define POD_BUFFER_SIZE 1024

const pw_stream_events g_stream_events = {
    .version = PW_VERSION_STREAM_EVENTS,
    .control_info = PipeWireAudioSinkStream::OnControlInfo,
    .process = PipeWireAudioSinkStream::Process
};

struct PWFormatsMapEntry
{
    SampleFormat format;
    spa_audio_format spa_format;
    int32_t stride;
    bool planar;
} const g_pw_formats_map[] = {
    { SampleFormat::kU8, SPA_AUDIO_FORMAT_U8, 1, false },
    { SampleFormat::kS16, SPA_AUDIO_FORMAT_S16, 2, false },
    { SampleFormat::kS32, SPA_AUDIO_FORMAT_S32, 4, false },
    { SampleFormat::kF32, SPA_AUDIO_FORMAT_F32, 4, false },
    { SampleFormat::kF64, SPA_AUDIO_FORMAT_F64, 8, false },
    { SampleFormat::kU8P, SPA_AUDIO_FORMAT_U8P, 1, true },
    { SampleFormat::kS16P, SPA_AUDIO_FORMAT_S16P, 2, true },
    { SampleFormat::kS32P, SPA_AUDIO_FORMAT_S32P, 4, true },
    { SampleFormat::kF32P, SPA_AUDIO_FORMAT_F32P, 4, true },
    { SampleFormat::kF64P, SPA_AUDIO_FORMAT_F64P, 8, true }
};

spa_audio_format get_spa_audio_format(SampleFormat format)
{
    for (const auto& entry : g_pw_formats_map)
    {
        if (entry.format == format)
            return entry.spa_format;
    }
    return SPA_AUDIO_FORMAT_UNKNOWN;
}

const PWFormatsMapEntry& get_sample_format_info(SampleFormat format)
{
    for (const auto& entry : g_pw_formats_map)
    {
        if (entry.format == format)
            return entry;
    }
    MARK_UNREACHABLE();
}

} // namespace anonymous

std::unique_ptr<PipeWireAudioSinkStream>
PipeWireAudioSinkStream::MakeFromDevice(const std::shared_ptr<PipeWireAudioDevice>& device,
                                        const std::string& name)
{
    if (name.empty() || !device)
        return nullptr;

    auto stream = std::make_unique<PipeWireAudioSinkStream>();

    pw_loop *loop = pw_thread_loop_get_loop(device->GetPipeWireLoop());
    CHECK(loop);

    PipeWireAudioDevice::ScopedThreadLoopLock lock(device.get());

    std::string nodename = fmt::format("Cocoa [{}]", name);

    pw_properties *props = pw_properties_new(PW_KEY_MEDIA_TYPE, "Audio",
                                             PW_KEY_MEDIA_CATEGORY, "Playback",
                                             PW_KEY_MEDIA_ROLE, "Music",
                                             PW_KEY_NODE_NAME, nodename.c_str(),
                                             PW_KEY_NODE_DESCRIPTION, "Cocoa Audio Sink",
                                             PW_KEY_APP_NAME, "Cocoa",
                                             PW_KEY_NODE_ALWAYS_PROCESS, "true",
                                             nullptr);

    stream->device_ = device;
    stream->pw_stream_ = pw_stream_new_simple(
            loop, name.c_str(), props, &g_stream_events, stream.get());

    if (!stream->pw_stream_)
    {
        pw_properties_free(props);
        return nullptr;
    }

    stream->disposed_ = false;

    return stream;
}

PipeWireAudioSinkStream::PipeWireAudioSinkStream()
    : disposed_(true)
    , pw_stream_(nullptr)
    , sample_format_(SampleFormat::kUnknown)
    , channel_mode_(AudioChannelMode::kUnknown)
    , sample_rate_(0)
    , current_queued_samples_(0)
    , delay_in_us_(0)
{
}

PipeWireAudioSinkStream::~PipeWireAudioSinkStream()
{
    CHECK(disposed_ && "Object must be disposed before destruction");
}

std::shared_ptr<AudioDevice> PipeWireAudioSinkStream::OnGetDevice()
{
    return device_;
}

void PipeWireAudioSinkStream::OnDispose()
{
    if (disposed_)
        return;

    if (this->IsConnected())
        this->Disconnect();

    {
        PipeWireAudioDevice::ScopedThreadLoopLock lock(device_.get());
        pw_stream_destroy(pw_stream_);
    }

    pw_stream_ = nullptr;
    device_.reset();
    disposed_ = true;
}

bool PipeWireAudioSinkStream::OnConnect(SampleFormat sample_format,
                                        AudioChannelMode channel_mode,
                                        int32_t sample_rate,
                                        bool realtime)
{
    PipeWireAudioDevice::ScopedThreadLoopLock lock(device_.get());

    spa_audio_format spa_fmt = get_spa_audio_format(sample_format);
    if (spa_fmt == SPA_AUDIO_FORMAT_UNKNOWN)
    {
        QLOG(LOG_ERROR, "Failed to connect stream: unsupported sample format");
        return false;
    }

    // Prepare POD buffer and connect to the stream
    auto pod_buffer = std::make_unique<uint8_t[]>(POD_BUFFER_SIZE);
    spa_pod_builder builder = SPA_POD_BUILDER_INIT(pod_buffer.get(), POD_BUFFER_SIZE);
    const spa_pod *params[1];

    uint32_t channels = (channel_mode == AudioChannelMode::kStereo ? 2 : 1);
    spa_audio_info_raw audio_info{
        .format = spa_fmt,
        .rate = static_cast<uint32_t>(sample_rate),
        .channels = channels
    };

    params[0] = spa_format_audio_raw_build(&builder,
                                           SPA_PARAM_EnumFormat,
                                           &audio_info);

    auto flags = static_cast<pw_stream_flags>(
            PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS |
            (realtime ? PW_STREAM_FLAG_RT_PROCESS : 0));

    int connect_result = pw_stream_connect(pw_stream_, PW_DIRECTION_OUTPUT,
                                           PW_ID_ANY, flags, params, 1);
    if (connect_result < 0)
    {
        QLOG(LOG_ERROR, "Failed to connect to PipeWire playback stream");
        return false;
    }

    sample_format_ = sample_format;
    channel_mode_ = channel_mode;
    sample_rate_ = sample_rate;

    return true;
}

bool PipeWireAudioSinkStream::OnDisconnect()
{
    {
        PipeWireAudioDevice::ScopedThreadLoopLock lock(device_.get());
        if (pw_stream_disconnect(pw_stream_) < 0)
            return false;
    }

    if (current_buffer_.frame)
    {
        av_frame_free(&current_buffer_.frame);
        current_buffer_.offset = 0;
    }

    // Mutex is not needed here as the streaming thread has been stopped
    while (!queue_.empty())
    {
        BufferItem buffer = queue_.front();
        queue_.pop();
        av_frame_free(&buffer.frame);
    }

    current_queued_samples_ = 0;

    return true;
}


bool PipeWireAudioSinkStream::Enqueue(const AudioBuffer& buffer)
{
    if (!this->IsConnected())
        return false;

    if (buffer.GetInfo().GetSampleFormat() != sample_format_ ||
        buffer.GetInfo().GetChannelMode() != channel_mode_ ||
        buffer.GetInfo().GetSampleRate() != sample_rate_)
    {
        return false;
    }

    AVFrame *frame_dup = av_frame_clone(buffer.CastUnderlyingPointer<AVFrame>());
    CHECK(frame_dup && "Failed to allocate memory");

    {
        std::scoped_lock<std::mutex> lock(queue_lock_);
        queue_.emplace(BufferItem{
            .frame = frame_dup,
            .offset = 0
        });
        current_queued_samples_ += frame_dup->nb_samples;
    }

    return true;
}

void PipeWireAudioSinkStream::OnControlInfo(void *userdata,
                                            uint32_t id,
                                            const pw_stream_control *ctl)
{
    auto *self = reinterpret_cast<PipeWireAudioSinkStream*>(userdata);
    CHECK(self);

    if (id == SPA_PROP_channelVolumes)
    {
        float avg_vol = 0;
        for (int i = 0; i < ctl->n_values; i++)
            avg_vol += ctl->values[i] / static_cast<float>(ctl->n_values);

        self->device_->InvokeFromMainThread([self, avg_vol] {
            self->volume_ = avg_vol;
            if (self->GetEventListener())
                self->GetEventListener()->OnVolumeChanged(avg_vol);
        });
    }
}

void PipeWireAudioSinkStream::Process(void *userdata)
{
    auto *self = reinterpret_cast<PipeWireAudioSinkStream*>(userdata);
    CHECK(self);

    if (!self->HasExpiredBuffer())
        return;

    pw_buffer *buffer = pw_stream_dequeue_buffer(self->pw_stream_);
    if (!buffer)
    {
        QLOG(LOG_WARNING, "PipeWire stream is out of buffer");
        return;
    }

    // Update delay
    pw_time stream_time{};
    pw_stream_get_time_n(self->pw_stream_, &stream_time, sizeof(pw_time));
    if (stream_time.rate.num == 0)
        stream_time.rate.denom = 1;
    if (stream_time.rate.denom == 0)
        stream_time.rate.denom = self->sample_rate_;

    self->delay_in_us_ = static_cast<double>(stream_time.delay) * SPA_USEC_PER_SEC
                         * stream_time.rate.num / stream_time.rate.denom;

    // Fill buffers
    spa_buffer *spabuf = buffer->buffer;

    const PWFormatsMapEntry& format_info = get_sample_format_info(self->sample_format_);
    int channels = (self->channel_mode_ == AudioChannelMode::kStereo ? 2 : 1);
    uint32_t stride = format_info.stride, nb_buffers = channels;
    if (!format_info.planar)
    {
        stride *= channels;
        nb_buffers = 1;
    }

    if (spabuf->n_datas < nb_buffers)
    {
        QLOG(LOG_ERROR, "PipeWire provide us with a invalid buffer");
        pw_stream_queue_buffer(self->pw_stream_, buffer);
        return;
    }

    uint32_t req_nb_samples = UINT32_MAX;
    for (int32_t i = 0; i < nb_buffers; i++)
    {
        uint32_t samples = spabuf->datas[i].maxsize / stride;
        if (req_nb_samples > samples)
            req_nb_samples = samples;
    }
    if (buffer->requested > 0)
        req_nb_samples = buffer->requested;

    uint32_t req_each_bufsize = req_nb_samples * stride;

    BufferItem& expired_buf = self->GetExpiredBuffer();
    uint32_t remaining_bufsize = expired_buf.frame->nb_samples * stride - expired_buf.offset;
    uint32_t write_each_bufsize = std::min(req_each_bufsize, remaining_bufsize);
    for (int32_t i = 0; i < nb_buffers; i++)
    {
        std::memcpy(spabuf->datas[i].data,
                    expired_buf.frame->data[i] + expired_buf.offset,
                    write_each_bufsize);
        spabuf->datas[i].chunk->offset = 0;
        spabuf->datas[i].chunk->stride = static_cast<int32_t>(stride);
        spabuf->datas[i].chunk->size = write_each_bufsize;
    }
    expired_buf.offset += write_each_bufsize;

    // Current buffer is used up
    if (remaining_bufsize == write_each_bufsize)
        self->CurrentBufferConsumed();

    pw_stream_queue_buffer(self->pw_stream_, buffer);
}

bool PipeWireAudioSinkStream::HasExpiredBuffer()
{
    // A buffer is currently playing
    if (current_buffer_.frame)
        return true;

    std::scoped_lock<std::mutex> lock(queue_lock_);
    return !queue_.empty();
}

PipeWireAudioSinkStream::BufferItem& PipeWireAudioSinkStream::GetExpiredBuffer()
{
    std::scoped_lock<std::mutex> lock(queue_lock_);
    if (!current_buffer_.frame)
    {
        current_buffer_ = queue_.front();
        queue_.pop();
        current_queued_samples_ -= current_buffer_.frame->nb_samples;
    }

    return current_buffer_;
}

void PipeWireAudioSinkStream::CurrentBufferConsumed()
{
    CHECK(current_buffer_.frame);
    av_frame_free(&current_buffer_.frame);
    current_buffer_.offset = 0;
}

double PipeWireAudioSinkStream::GetDelayInUs()
{
    queue_lock_.lock();
    double queue_delay = static_cast<double>(current_queued_samples_) / sample_rate_ * SPA_USEC_PER_SEC;
    queue_lock_.unlock();

    return delay_in_us_ + queue_delay;
}

float PipeWireAudioSinkStream::GetVolume()
{
    return volume_;
}

void PipeWireAudioSinkStream::SetVolume(float volume)
{
    PipeWireAudioDevice::ScopedThreadLoopLock lock(device_.get());
    int channels = (channel_mode_ == AudioChannelMode::kStereo ? 2 : 1);
    float values[2] = {volume, volume};
    pw_stream_set_control(pw_stream_, SPA_PROP_channelVolumes, channels, values);
}

UTAU_NAMESPACE_END
