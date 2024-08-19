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

#include <unordered_map>

#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>
#include <spa/support/loop.h>
#include <spa/param/props.h>

#include "Core/Journal.h"
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
    AVSampleFormat format;
    spa_audio_format spa_format;
    int32_t stride;
    bool planar;
} const g_pw_formats_map[] = {
    { AV_SAMPLE_FMT_U8, SPA_AUDIO_FORMAT_U8, 1, false },
    { AV_SAMPLE_FMT_S16, SPA_AUDIO_FORMAT_S16, 2, false },
    { AV_SAMPLE_FMT_S32, SPA_AUDIO_FORMAT_S32, 4, false },
    { AV_SAMPLE_FMT_FLT, SPA_AUDIO_FORMAT_F32, 4, false },
    { AV_SAMPLE_FMT_DBL, SPA_AUDIO_FORMAT_F64, 8, false },
    { AV_SAMPLE_FMT_U8P, SPA_AUDIO_FORMAT_U8P, 1, true },
    { AV_SAMPLE_FMT_S16P, SPA_AUDIO_FORMAT_S16P, 2, true },
    { AV_SAMPLE_FMT_S32P, SPA_AUDIO_FORMAT_S32P, 4, true },
    { AV_SAMPLE_FMT_FLTP, SPA_AUDIO_FORMAT_F32P, 4, true },
    { AV_SAMPLE_FMT_DBLP, SPA_AUDIO_FORMAT_F64P, 8, true }
};

spa_audio_format get_spa_audio_format(AVSampleFormat format)
{
    for (const auto& entry : g_pw_formats_map)
    {
        if (entry.format == format)
            return entry.spa_format;
    }
    return SPA_AUDIO_FORMAT_UNKNOWN;
}

const PWFormatsMapEntry& get_sample_format_info(AVSampleFormat format)
{
    for (const auto& entry : g_pw_formats_map)
    {
        if (entry.format == format)
            return entry;
    }
    MARK_UNREACHABLE();
}

const std::unordered_map<AVChannel, spa_audio_channel> g_channel_map{
    { AV_CHAN_FRONT_LEFT, SPA_AUDIO_CHANNEL_FL },
    { AV_CHAN_FRONT_RIGHT, SPA_AUDIO_CHANNEL_FR },
    { AV_CHAN_FRONT_CENTER, SPA_AUDIO_CHANNEL_FC },
    { AV_CHAN_LOW_FREQUENCY_2, SPA_AUDIO_CHANNEL_LFE },
    { AV_CHAN_SIDE_LEFT, SPA_AUDIO_CHANNEL_SL },
    { AV_CHAN_SIDE_RIGHT, SPA_AUDIO_CHANNEL_SR },
    { AV_CHAN_FRONT_LEFT_OF_CENTER, SPA_AUDIO_CHANNEL_FLC },
    { AV_CHAN_FRONT_RIGHT_OF_CENTER, SPA_AUDIO_CHANNEL_FRC },
    { AV_CHAN_BACK_CENTER, SPA_AUDIO_CHANNEL_RC },
    { AV_CHAN_BACK_LEFT, SPA_AUDIO_CHANNEL_RL },
    { AV_CHAN_BACK_RIGHT, SPA_AUDIO_CHANNEL_RR },
    { AV_CHAN_TOP_CENTER, SPA_AUDIO_CHANNEL_TC },
    { AV_CHAN_TOP_FRONT_LEFT, SPA_AUDIO_CHANNEL_TFL },
    { AV_CHAN_TOP_FRONT_CENTER, SPA_AUDIO_CHANNEL_TFC },
    { AV_CHAN_TOP_FRONT_RIGHT, SPA_AUDIO_CHANNEL_TFR },
    { AV_CHAN_TOP_BACK_LEFT, SPA_AUDIO_CHANNEL_TRL },
    { AV_CHAN_TOP_BACK_CENTER, SPA_AUDIO_CHANNEL_TRC },
    { AV_CHAN_TOP_BACK_RIGHT, SPA_AUDIO_CHANNEL_TRR },
    { AV_CHAN_WIDE_RIGHT, SPA_AUDIO_CHANNEL_FRW },
    { AV_CHAN_WIDE_LEFT, SPA_AUDIO_CHANNEL_FLW },
    { AV_CHAN_LOW_FREQUENCY_2, SPA_AUDIO_CHANNEL_LFE2 },
    { AV_CHAN_TOP_SIDE_LEFT, SPA_AUDIO_CHANNEL_TSL },
    { AV_CHAN_TOP_SIDE_RIGHT, SPA_AUDIO_CHANNEL_TSR },
    { AV_CHAN_BOTTOM_FRONT_CENTER, SPA_AUDIO_CHANNEL_BC },
    // FIXME(sora): is the following two map correct?
    { AV_CHAN_BOTTOM_FRONT_LEFT, SPA_AUDIO_CHANNEL_BLC },
    { AV_CHAN_BOTTOM_FRONT_RIGHT, SPA_AUDIO_CHANNEL_BRC }
};

bool fill_spa_audio_channel_info(spa_audio_info_raw& dst, const AVChannelLayout& ch_layout)
{
    // For unspecified channel order, we do not know how to
    // handle them.
    if (ch_layout.order == AV_CHANNEL_ORDER_UNSPEC ||
        ch_layout.nb_channels > SPA_AUDIO_MAX_CHANNELS)
    {
        return false;
    }

    dst.channels = ch_layout.nb_channels;
    for (int32_t i = 0; i < dst.channels; i++)
    {
        AVChannel ch = av_channel_layout_channel_from_index(&ch_layout, i);
        if (!g_channel_map.contains(ch))
            return false;
        dst.position[i] = g_channel_map.at(ch);
    }
    return true;
}

} // namespace anonymous

std::shared_ptr<PipeWireAudioSinkStream>
PipeWireAudioSinkStream::MakeFromDevice(const std::shared_ptr<PipeWireAudioDevice>& device,
                                        const std::string& name,
                                        AVSampleFormat sample_format,
                                        int32_t sample_rate,
                                        const AVChannelLayout& ch_layout,
                                        bool realtime)
{
    if (name.empty() || !device)
        return nullptr;

    auto stream = std::make_shared<PipeWireAudioSinkStream>();

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
    if (!stream->ConnectToStream(sample_format, sample_rate, ch_layout, realtime))
    {
        pw_properties_free(props);
        return nullptr;
    }

    return stream;
}

PipeWireAudioSinkStream::PipeWireAudioSinkStream()
    : frame_id_cnt_(0)
    , disposed_(true)
    , pw_stream_(nullptr)
    , sample_format_(AV_SAMPLE_FMT_NONE)
    , ch_layout_{}
    , sample_rate_(0)
    , current_queued_samples_(0)
    , delay_in_us_(0)
{
}

PipeWireAudioSinkStream::~PipeWireAudioSinkStream()
{
    DoDispose();
}

std::shared_ptr<AudioDevice> PipeWireAudioSinkStream::GetDevice()
{
    return device_;
}

void PipeWireAudioSinkStream::Dispose()
{
    DoDispose();
}

void PipeWireAudioSinkStream::DoDispose()
{
    if (disposed_)
        return;

    DisconnectStream();
    {
        PipeWireAudioDevice::ScopedThreadLoopLock lock(device_.get());
        pw_stream_destroy(pw_stream_);
    }

    pw_stream_ = nullptr;
    device_.reset();
    disposed_ = true;
    av_channel_layout_uninit(&ch_layout_);
}

auto PipeWireAudioSinkStream::GetListener() const -> std::shared_ptr<Listener>
{
    return listener_;
}

void PipeWireAudioSinkStream::SetListener(std::shared_ptr<Listener> listener)
{
    if (listener_ && !listener)
        device_->DecreaseEventListenerCount();
    else if (!listener_ && listener)
        device_->IncreaseEventListenerCount();

    listener_ = std::move(listener);
}

bool PipeWireAudioSinkStream::ConnectToStream(AVSampleFormat sample_format,
                                              int32_t sample_rate,
                                              const AVChannelLayout& ch_layout,
                                              bool realtime)
{
    PipeWireAudioDevice::ScopedThreadLoopLock lock(device_.get());

    spa_audio_format spa_fmt = get_spa_audio_format(sample_format);
    if (spa_fmt == SPA_AUDIO_FORMAT_UNKNOWN)
    {
        QLOG(LOG_ERROR, "Failed to connect stream: unsupported sample format");
        return false;
    }

    spa_audio_info_raw audio_info{
        .format = spa_fmt,
        .rate = static_cast<uint32_t>(sample_rate)
    };
    if (!fill_spa_audio_channel_info(audio_info, ch_layout))
    {
        QLOG(LOG_ERROR, "The channel layout is not supported by PipeWire");
        return false;
    }

    // Prepare POD buffer and connect to the stream
    auto pod_buffer = std::make_unique<uint8_t[]>(POD_BUFFER_SIZE);
    spa_pod_builder builder = SPA_POD_BUILDER_INIT(pod_buffer.get(), POD_BUFFER_SIZE);
    const spa_pod *params[1];

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
    av_channel_layout_copy(&ch_layout_, &ch_layout);
    sample_rate_ = sample_rate;

    return true;
}

void PipeWireAudioSinkStream::DisconnectStream()
{
    {
        PipeWireAudioDevice::ScopedThreadLoopLock lock(device_.get());
        if (pw_stream_disconnect(pw_stream_) < 0)
            return;
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
}

uint64_t PipeWireAudioSinkStream::Enqueue(const AVFrame *frame)
{
    // Verify sample characteristics
    if (frame->format != sample_format_ ||
        av_channel_layout_compare(&frame->ch_layout, &ch_layout_) != 0 ||
        frame->sample_rate != sample_rate_)
    {
        return 0;
    }

    AVFrame *frame_dup = av_frame_clone(frame);
    CHECK(frame_dup && "allocation failed");

    std::scoped_lock<std::mutex> lock(queue_lock_);
    frame_id_cnt_++;
    queue_.emplace(BufferItem{ .id = frame_id_cnt_, .frame = frame_dup, .offset = 0 });
    current_queued_samples_ += frame_dup->nb_samples;

    return frame_id_cnt_;
}

void PipeWireAudioSinkStream::OnControlInfo(void *userdata,
                                            uint32_t id,
                                            const pw_stream_control *ctl)
{
    auto *self_bare = reinterpret_cast<PipeWireAudioSinkStream*>(userdata);
    CHECK(self_bare);
    auto self = self_bare->shared_from_this();
    CHECK(self);

    if (id == SPA_PROP_channelVolumes)
    {
        std::vector<float> volumes(ctl->values, ctl->values + ctl->n_values);
        self->device_->SendTaskToMainThread([self, volumes] {
            if (self->GetListener())
                self->GetListener()->OnVolumeChanged(volumes);
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
    int channels = self->ch_layout_.nb_channels;
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

        if (queue_.empty())
        {
            device_->SendTaskToMainThread([self = shared_from_this(), id = current_buffer_.id]{
                if (self->GetListener())
                    self->GetListener()->OnEmptyQueue(id);
            });
        }
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

void PipeWireAudioSinkStream::SetVolume(const std::vector<float>& volume)
{
    if (volume.size() != ch_layout_.nb_channels)
        return;

    PipeWireAudioDevice::ScopedThreadLoopLock lock(device_.get());
    pw_stream_set_control(pw_stream_, SPA_PROP_channelVolumes, volume.size(),
                          const_cast<float*>(volume.data()));
}

UTAU_NAMESPACE_END
