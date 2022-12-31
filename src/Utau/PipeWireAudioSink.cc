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

#include "Core/Errors.h"
#include "Core/Exception.h"
#include "Core/Journal.h"
#include "Utau/Utau.h"
#include "Utau/PipeWireAudioSink.h"
#include "Utau/AudioBuffer.h"
UTAU_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Utau.PipeWireAudioSink)

#define DEFAULT_RATE        44100
#define DEFAULT_CHANNELS    2

#define POD_BUFFER_SIZE     1024

namespace {

const pw_stream_events g_stream_events = {
    .version = PW_VERSION_STREAM_EVENTS,
    .process = PipeWireAudioSink::on_stream_process
};

} // namespace anonymous

std::unique_ptr<AudioSink> AudioSink::MakePipeWire(uv_loop_t *loop)
{
    return PipeWireAudioSink::Connect(loop);
}

std::unique_ptr<PipeWireAudioSink> PipeWireAudioSink::Connect(uv_loop_t *loop)
{
    CHECK(loop);

    auto sink = std::make_unique<PipeWireAudioSink>(loop);

    std::promise<bool> init_result_promise;

    sink->thread_ = std::thread(PipeWireAudioSink::worker_thread_routine,
                                sink.get(), &init_result_promise);

    if (!init_result_promise.get_future().get())
        return nullptr;

    return sink;
}

void PipeWireAudioSink::on_stream_process(void *userdata)
{
    CHECK(userdata);

    auto *sink = reinterpret_cast<PipeWireAudioSink*>(userdata);

    if (!sink->current_buffer_.buffer)
    {
        auto maybe = sink->TakeNextBuffer();
        if (!maybe)
            return;

        BufferEvent event(*maybe, BufferEvent::kPlaying);
        sink->SendBufferEventFromWorkerThread(event);

        sink->current_buffer_.buffer = maybe->buffer;
        sink->current_buffer_.id = maybe->id;
        sink->current_buffer_offset_ = 0;
    }

    pw_buffer *buffer = pw_stream_dequeue_buffer(sink->pw_stream_);
    if (!buffer)
    {
        QLOG(LOG_WARNING, "PipeWire stream is out of buffer");
        return;
    }

    static constexpr size_t kStride = sizeof(float) * DEFAULT_CHANNELS;

    spa_buffer *spa_buf = buffer->buffer;
    if (spa_buf->datas[0].data == nullptr)
        return;

    int32_t req_n_frames = SPA_MIN(buffer->requested, spa_buf->datas[0].maxsize / kStride);

    // Copy audio frames
    std::shared_ptr<AudioBuffer> src_buffer = sink->current_buffer_.buffer;
    const AudioBufferInfo& src_info = src_buffer->GetInfo();

    uint8_t *ptr = src_buffer->GetAddress(0) + sink->current_buffer_offset_;
    size_t read_bytes = std::min(src_info.ComputeTotalBufferSize() - sink->current_buffer_offset_,
                                 req_n_frames * kStride);

    std::memcpy(spa_buf->datas[0].data, ptr, read_bytes);
    sink->current_buffer_offset_ += read_bytes;

    if (sink->current_buffer_offset_ >= src_info.ComputeTotalBufferSize())
    {
        BufferEvent event(sink->current_buffer_, BufferEvent::kConsumed);
        sink->SendBufferEventFromWorkerThread(event);

        sink->current_buffer_.buffer = nullptr;
        sink->current_buffer_.id = 0;
        sink->current_buffer_offset_ = 0;
    }

    // Submit filled buffer
    spa_buf->datas[0].chunk->offset = 0;
    spa_buf->datas[0].chunk->stride = kStride;
    spa_buf->datas[0].chunk->size = read_bytes;

    pw_stream_queue_buffer(sink->pw_stream_, buffer);
}

void PipeWireAudioSink::worker_thread_routine(PipeWireAudioSink *self,
                                              std::promise<bool> *init_result_promise)
{
    QLOG(LOG_INFO, "Initialize PipeWire audio sink");

    if (!self->initialize_audio_sink())
    {
        init_result_promise->set_value(false);
        return;
    }
    init_result_promise->set_value(true);

    pthread_setname_np(self->thread_.native_handle(), "PipeWireSink");

    pw_main_loop_run(self->pw_loop_);
    QLOG(LOG_INFO, "PipeWire audio sink was exited");
}

bool PipeWireAudioSink::initialize_audio_sink()
{
    // Initialize & setup log interface for PipeWire client
    int pseudo_argc = 1;
    char *pseudo_argv[] = { program_invocation_name, nullptr };
    pw_init(&pseudo_argc, reinterpret_cast<char***>(&pseudo_argv));

    // Will be cancelled if no error occurred
    ScopeExitAutoInvoker scope_exit([] { pw_deinit(); });

    pod_buffer_ = std::make_unique<uint8_t[]>(POD_BUFFER_SIZE);

    // Connect to PipeWire daemon
    pw_loop_ = pw_main_loop_new(nullptr);
    if (!pw_loop_)
    {
        QLOG(LOG_ERROR, "Failed to create PipeWire mainloop");
        return false;
    }

    pw_properties *props = pw_properties_new(PW_KEY_MEDIA_TYPE, "Audio",
                                             PW_KEY_MEDIA_CATEGORY, "Playback",
                                             PW_KEY_MEDIA_ROLE, "Music",
                                             nullptr);

    // Create PipeWire audio playback stream
    pw_stream_ = pw_stream_new_simple(pw_main_loop_get_loop(pw_loop_),
                                      "Cocoa", props, &g_stream_events, this);
    if (!pw_stream_)
    {
        QLOG(LOG_ERROR, "Failed to create PipeWire playback stream");
        pw_main_loop_destroy(pw_loop_);
        return false;
    }

    // Prepare POD buffer and connect to the stream
    spa_pod_builder builder = SPA_POD_BUILDER_INIT(pod_buffer_.get(), POD_BUFFER_SIZE);
    const spa_pod *params[1];

    spa_audio_info_raw audio_info = SPA_AUDIO_INFO_RAW_INIT(
            .format = SPA_AUDIO_FORMAT_F32,
            .rate = DEFAULT_RATE,
            .channels = DEFAULT_CHANNELS
    );

    params[0] = spa_format_audio_raw_build(&builder,
                                           SPA_PARAM_EnumFormat,
                                           &audio_info);

    auto flags = static_cast<pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT |
                                              PW_STREAM_FLAG_MAP_BUFFERS |
                                              PW_STREAM_FLAG_RT_PROCESS);
    int connect_result = pw_stream_connect(pw_stream_, PW_DIRECTION_OUTPUT,
                                           PW_ID_ANY, flags, params, 1);
    if (connect_result < 0)
    {
        QLOG(LOG_ERROR, "Failed to connect to PipeWire playback stream");
        pw_stream_destroy(pw_stream_);
        pw_main_loop_destroy(pw_loop_);
        return false;
    }

    scope_exit.cancel();
    return true;
}

PipeWireAudioSink::PipeWireAudioSink(uv_loop_t *loop)
    : AudioSink(loop, BackDevice::kPipeWire_BackDevice)
    , pw_loop_(nullptr)
    , pw_stream_(nullptr)
    , current_buffer_(nullptr, 0)
    , current_buffer_offset_(0)
{
}

bool PipeWireAudioSink::BufferCheckBeforeEnqueue(const std::shared_ptr<AudioBuffer>& buffer)
{
    const AudioBufferInfo& info = buffer->GetInfo();

    if (info.GetChannels() != DEFAULT_CHANNELS)
        return false;

    if (info.GetSampleRate() != DEFAULT_RATE)
        return false;

    if (info.GetSampleFormat() != SampleFormat::kF32)
        return false;

    return true;
}

namespace {

int new_buffer_notify_trampoline(struct spa_loop *loop,
                                 g_maybe_unused bool async,
                                 g_maybe_unused uint32_t seq,
                                 g_maybe_unused const void *data,
                                 g_maybe_unused size_t size,
                                 void *user_data)
{
    CHECK(user_data);

    auto *sink = reinterpret_cast<PipeWireAudioSink*>(user_data);
    sink->NotifyNewBufferEnqueuedInWorkerThread();

    return 0;
}

int dispose_notify_trampoline(struct spa_loop *loop,
                              g_maybe_unused bool async,
                              g_maybe_unused uint32_t seq,
                              g_maybe_unused const void *data,
                              g_maybe_unused size_t size,
                              void *user_data)
{
    CHECK(user_data);

    auto *sink = reinterpret_cast<PipeWireAudioSink*>(user_data);
    sink->NotifyDisposeInWorkerThread();

    return 0;
}

} // namespace

void PipeWireAudioSink::NotifyNewBufferEnqueued()
{
    // Invoke `new_buffer_notify_trampoline` function in worker thread, and as a trampoline,
    // `new_buffer_notify_trampoline` will finally call `NotifyNewBufferEnqueuedInWorkerThread`.
    // The caller thread will be blocked until calling is done.
    pw_loop_invoke(pw_main_loop_get_loop(pw_loop_),
                   new_buffer_notify_trampoline, SPA_ID_INVALID, nullptr, 0, true, this);
}

void PipeWireAudioSink::NotifyNewBufferEnqueuedInWorkerThread()
{
    pw_stream_trigger_process(pw_stream_);
}

void PipeWireAudioSink::NotifyAndWaitWorkerThreadDispose()
{
    // Ask mainloop quit immediately, the worker thread should quit automatically
    pw_loop_invoke(pw_main_loop_get_loop(pw_loop_),
                   dispose_notify_trampoline, SPA_ID_INVALID, nullptr, 0, true, this);

    // Wait the worker thread
    if (thread_.joinable())
        thread_.join();

    // Free resources
    pw_stream_disconnect(pw_stream_);
    pw_stream_destroy(pw_stream_);
    pw_main_loop_destroy(pw_loop_);

    pw_deinit();
}

void PipeWireAudioSink::NotifyDisposeInWorkerThread()
{
    pw_main_loop_quit(pw_loop_);
}

AudioChannelMode PipeWireAudioSink::GetRequiredChannelMode() const
{
    return AudioChannelMode::kStereo;
}

SampleFormat PipeWireAudioSink::GetRequiredSampleFormat() const
{
    return SampleFormat::kF32;
}

int PipeWireAudioSink::GetRequiredSampleRate() const
{
    return DEFAULT_RATE;
}

UTAU_NAMESPACE_END
