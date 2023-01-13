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

#include <future>
#include <condition_variable>

#include "fmt/format.h"

#include "Core/EventLoop.h"
#include "Gallium/bindings/utau/Exports.h"
#include "Gallium/binder/Class.h"
#include "Gallium/binder/Convert.h"
#include "Utau/ffwrappers/libavutil.h"
GALLIUM_BINDINGS_UTAU_NS_BEGIN

using DecodeResult = utau::AVStreamDecoder::AVGenericDecoded;

struct MediaFramePresentDispatcher::PresentThreadCmd
{
    enum Verb
    {
        kTerminate,
        kPause,
        kPlay,
        kSeekTo
    };

    Verb verb;
    int64_t param;
    std::promise<void> promise;
};

struct MediaFramePresentDispatcher::PresentThreadContext
{
    explicit PresentThreadContext(MediaFramePresentDispatcher *d)
        : dispatcher(d)
        , decoder(d->decoder_wrap_->GetDecoder())
        , asinkstream(d->asinkstream_wrap_->GetStream())
    {
        uv_loop_init(&loop);
        uv_async_init(&loop, &thread_notifier, PresentThreadCmdHandler);
        thread_notifier.data = this;

        uv_timer_init(&loop, &timer);
        timer.data = this;

        decoding_thread = std::thread(&PresentThreadContext::DecodingThreadRoutine, this);
    }

    void DecodingThreadRoutine();

    void TryCloseLoopHandles() {
        if (loop_handles_closed)
            return;

        uv_close(reinterpret_cast<uv_handle_t*>(&thread_notifier), nullptr);
        uv_close(reinterpret_cast<uv_handle_t*>(&timer), nullptr);
        loop_handles_closed = true;
    }

    MediaFramePresentDispatcher *dispatcher;
    uv_loop_t loop{};
    uv_async_t thread_notifier{};
    uv_timer_t timer{};
    PresentThreadCmd *cmd = nullptr;
    utau::AVStreamDecoder *decoder;
    utau::AudioSinkStream *asinkstream;
    std::thread decoding_thread;
    bool loop_handles_closed = false;

#define AUDIO_QUEUE_MAX_FRAMES 20
#define VIDEO_QUEUE_MAX_FRAMES 5

    template<typename T>
    struct QueueTimedBuffer
    {
        double pts = 0;
        int64_t serial = 0;
        std::unique_ptr<T> buffer;
    };

    bool decode_stop_flag = false;
    std::condition_variable queue_cond;
    std::mutex queue_lock;
    std::queue<QueueTimedBuffer<utau::AudioBuffer>> audio_queue;
    std::queue<QueueTimedBuffer<utau::VideoBuffer>> video_queue;

    double last_frame_pts = 0;

    DecodeResult last_frame{DecodeResult::kNull};

    int64_t last_required_intv_ms = 0;
    int64_t last_delay_compensated_intv_ms = 0;
};

void MediaFramePresentDispatcher::PresentThreadContext::DecodingThreadRoutine()
{
    pthread_setname_np(pthread_self(), "DecodeThread");

    int64_t serial_counter = 0;
    while (true)
    {
        {
            std::unique_lock<std::mutex> lock(queue_lock);
            queue_cond.wait(lock, [this] {
                return (audio_queue.size() < AUDIO_QUEUE_MAX_FRAMES ||
                        audio_queue.size() < VIDEO_QUEUE_MAX_FRAMES ||
                        decode_stop_flag);
            });

            if (decode_stop_flag)
                break;
        }

        DecodeResult result = decoder->DecodeNextFrame();

        std::scoped_lock<std::mutex> lock(queue_lock);
        if (result.type == DecodeResult::kEOF || result.type == DecodeResult::kNull)
        {
            // Both Error and EOF can cause this thread to exit
            decode_stop_flag = true;
            break;
        }

        if (result.type == DecodeResult::kAudio)
        {
            AVRational tb = av_make_q(dispatcher->audio_stinfo_.time_base.num,
                                      dispatcher->audio_stinfo_.time_base.denom);
            int64_t pts = result.audio->CastUnderlyingPointer<AVFrame>()->pts;
            audio_queue.emplace();
            audio_queue.back().serial = ++serial_counter;
            audio_queue.back().pts = av_q2d(tb) * static_cast<double>(pts);
            audio_queue.back().buffer = std::move(result.audio);
        }
        else if (result.type == DecodeResult::kVideo)
        {
            AVRational tb = av_make_q(dispatcher->video_stinfo_.time_base.num,
                                      dispatcher->video_stinfo_.time_base.denom);
            int64_t pts = result.video->CastUnderlyingPointer<AVFrame>()->pts;
            video_queue.emplace();
            video_queue.back().serial = ++serial_counter;
            video_queue.back().pts = av_q2d(tb) * static_cast<double>(pts);
            video_queue.back().buffer = std::move(result.video);
        }
    }
}

MediaFramePresentDispatcher::MediaFramePresentDispatcher(v8::Local<v8::Value> decoder,
                                                         v8::Local<v8::Value> audioSinkStream)
    : MaybeGCRootObject(v8::Isolate::GetCurrent())
    , asinkstream_wrap_(nullptr)
    , disposed_(false)
    , paused_(true)
    , mp_thread_()
    , host_notifier_(nullptr)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    decoder_wrap_ = binder::Class<AVStreamDecoderWrap>::unwrap_object(isolate, decoder);
    if (!decoder_wrap_)
        g_throw(TypeError, "Argument `decoder` must be an instance of `AVStreamDecoder`");
    decoder_js_obj_.Reset(isolate, v8::Local<v8::Object>::Cast(decoder));

    has_audio_ = decoder_wrap_->GetDecoder()->HasAudioStream();
    has_video_ = decoder_wrap_->GetDecoder()->HasVideoStream();

    if (has_audio_)
    {
        auto maybe = decoder_wrap_->GetDecoder()->GetStreamInfo(
                utau::AVStreamDecoder::kAudio_StreamType);
        if (!maybe)
            g_throw(Error, "Failed to get audio stream info");
        audio_stinfo_ = *maybe;

        asinkstream_wrap_ = binder::Class<AudioSinkStreamWrap>::unwrap_object(isolate, audioSinkStream);
        if (!asinkstream_wrap_)
            g_throw(TypeError, "Argument `audioSinkStream` must be an instance of `AudioSinkStream`");
        asinkstream_js_obj_.Reset(isolate, v8::Local<v8::Object>::Cast(audioSinkStream));
    }
    else
    {
        // TODO(sora): support pure-video media
        g_throw(Error, "Decoder cannot provide an audio stream (pure video media is not supported)");
    }

    if (has_video_)
    {
        auto maybe = decoder_wrap_->GetDecoder()->GetStreamInfo(
                utau::AVStreamDecoder::kVideo_StreamType);
        if (!maybe)
            g_throw(Error, "Failed to video stream info");
        video_stinfo_ = *maybe;
    }

    host_notifier_ = static_cast<uv_async_t*>(malloc(sizeof(uv_async_t)));
    CHECK(host_notifier_ && "Failed to allocate memory");
    uv_loop_t *main_thread_loop = EventLoop::Ref().handle();
    uv_async_init(main_thread_loop, host_notifier_, PresentRequestHandler);
    host_notifier_->data = this;

    thread_ctx_ = std::make_unique<PresentThreadContext>(this);
    mp_thread_ = std::thread(&MediaFramePresentDispatcher::ThreadRoutine, this);
}

MediaFramePresentDispatcher::~MediaFramePresentDispatcher()
{
    dispose();
}

void MediaFramePresentDispatcher::dispose()
{
    if (disposed_)
        return;

    if (!thread_ctx_->loop_handles_closed)
    {
        if (!paused_)
            pause();
        SendAndWaitForPresentThreadCmd(PresentThreadCmd::kTerminate, 0);
    }

    if (thread_ctx_->decoding_thread.joinable())
        thread_ctx_->decoding_thread.join();

    if (mp_thread_.joinable())
        mp_thread_.join();

    uv_close(reinterpret_cast<uv_handle_t*>(host_notifier_), [](uv_handle_t *ptr) {
        free(ptr);
    });

    thread_ctx_.reset();

    cb_present_video_buffer_.Reset();
    decoder_js_obj_.Reset();
    decoder_wrap_ = nullptr;
    asinkstream_js_obj_.Reset();
    asinkstream_js_obj_.Reset();
    asinkstream_wrap_ = nullptr;

    disposed_ = true;
}

void MediaFramePresentDispatcher::setOnPresentVideoBuffer(v8::Local<v8::Value> func)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!func->IsFunction())
        g_throw(TypeError, "Property `onPresentVideoBuffer` must be a function");
    cb_present_video_buffer_.Reset(isolate, v8::Local<v8::Function>::Cast(func));
}

v8::Local<v8::Value> MediaFramePresentDispatcher::getOnPresentVideoBuffer()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (cb_present_video_buffer_.IsEmpty())
        return v8::Null(isolate);
    return cb_present_video_buffer_.Get(isolate);
}

void MediaFramePresentDispatcher::setOnErrorOrEOF(v8::Local<v8::Value> func)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!func->IsFunction())
        g_throw(TypeError, "Property `onErrorOrEOF` must be a function");
    cb_error_or_eof_.Reset(isolate, v8::Local<v8::Function>::Cast(func));
}

v8::Local<v8::Value> MediaFramePresentDispatcher::getOnErrorOrEOF()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (cb_error_or_eof_.IsEmpty())
        return v8::Null(isolate);
    return cb_error_or_eof_.Get(isolate);
}

void MediaFramePresentDispatcher::ThreadRoutine()
{
    pthread_setname_np(pthread_self(), "MediaPresent");

    // FIXME(sora): Using `UV_RUN_DEFAULT` makes the audio cracking,
    //              as the event loop itself spends too much time relative to
    //              what we expected. Iterating the loop manually with a
    //              usleep() to decrease the CPU time is just a temporary
    //              solution, and maybe there are some better ways to
    //              solve that problem.
    while (uv_run(&thread_ctx_->loop, UV_RUN_NOWAIT) != 0)
        usleep(200);

    uv_loop_close(&thread_ctx_->loop);
}

void MediaFramePresentDispatcher::TimerCallback(uv_timer_t *timer)
{
    auto *thread_ctx = reinterpret_cast<PresentThreadContext*>(timer->data);
    CHECK(thread_ctx);

    if (thread_ctx->last_frame.type != DecodeResult::kNull)
    {
        thread_ctx->dispatcher->SendPresentRequest(std::move(thread_ctx->last_frame),
                                                   thread_ctx->last_frame_pts);
    }

    int64_t delay_comp = thread_ctx->last_required_intv_ms - thread_ctx->last_delay_compensated_intv_ms;
    if (delay_comp > 0)
    {
        thread_ctx->last_required_intv_ms = 0;
        thread_ctx->last_delay_compensated_intv_ms = 0;
        uv_timer_start(timer, TimerCallback, delay_comp, 0);
        return;
    }

    std::scoped_lock<std::mutex> queue_lock(thread_ctx->queue_lock);
    if (thread_ctx->decode_stop_flag)
    {
        // Decoding has stopped (by EOF or a decoding error)
        thread_ctx->dispatcher->SendErrorOrEOFRequest();
        // Close handles to quit main loop
        thread_ctx->TryCloseLoopHandles();
        return;
    }

    if (thread_ctx->audio_queue.empty())
    {
        // Do next loop iteration to wait for decoding thread to fill the queue
        uv_timer_start(timer, TimerCallback, 0, 0);
        return;
    }

    double head_audio_pts = thread_ctx->audio_queue.front().pts;
    double head_video_pts = -1;

    // Drop video frames
    while (!thread_ctx->video_queue.empty())
    {
        auto& head = thread_ctx->video_queue.front();
        if (head.pts < thread_ctx->last_frame_pts)
        {
            DecodeResult reqbuf(DecodeResult::kVideo);
            reqbuf.video = std::move(head.buffer);
            thread_ctx->dispatcher->SendPresentRequest(std::move(reqbuf), head.pts);
            thread_ctx->video_queue.pop();
        }
        else
        {
            head_video_pts = head.pts;
            break;
        }
    }

    double pts;
    if (head_video_pts >= 0 && head_video_pts < head_audio_pts)
    {
        thread_ctx->last_frame.type = DecodeResult::kVideo;
        thread_ctx->last_frame.video = std::move(thread_ctx->video_queue.front().buffer);
        pts = head_video_pts;
        thread_ctx->video_queue.pop();
    }
    else
    {
        thread_ctx->last_frame.type = DecodeResult::kAudio;
        thread_ctx->last_frame.audio = std::move(thread_ctx->audio_queue.front().buffer);
        pts = head_audio_pts;
        thread_ctx->audio_queue.pop();
    }

    thread_ctx->queue_cond.notify_one();

    auto interval = static_cast<int64_t>(std::round((pts - thread_ctx->last_frame_pts) * 1000));
    thread_ctx->last_required_intv_ms = interval;

    if (pts == head_audio_pts)
    {
        double stream_delay_us = thread_ctx->asinkstream->GetDelayInUs();
        interval -= static_cast<int64_t>(std::round(stream_delay_us / 1000));
    }

    interval = std::max(int64_t(0), interval);
    thread_ctx->last_delay_compensated_intv_ms = interval;

    uv_timer_start(timer, TimerCallback, interval, 0);
    thread_ctx->last_frame_pts = pts;
}

void MediaFramePresentDispatcher::SendPresentRequest(DecodeResult frame, double pts_seconds)
{
    if (frame.type == DecodeResult::kAudio)
    {
        asinkstream_wrap_->GetStream()->Enqueue(*frame.audio);
        return;
    }

    std::scoped_lock<std::mutex> lock(present_queue_lock_);
    present_queue_.emplace(PresentRequest{
        .error_or_eof = false,
        .send_timestamp = utau::GlobalContext::Ref().GetCurrentTimestampMs(),
        .frame_pts_seconds = pts_seconds,
        .vbuffer = std::move(frame.video)
    });
    uv_async_send(host_notifier_);
}

void MediaFramePresentDispatcher::SendErrorOrEOFRequest()
{
    std::scoped_lock<std::mutex> lock(present_queue_lock_);
    present_queue_.emplace(PresentRequest{
        .error_or_eof = true,
        .send_timestamp = utau::GlobalContext::Ref().GetCurrentTimestampMs(),
        .frame_pts_seconds = 0,
        .vbuffer = nullptr
    });
    uv_async_send(host_notifier_);
}

void MediaFramePresentDispatcher::PresentRequestHandler(uv_async_t *handle)
{
    // Requested by present thread; called from main thread
    auto *dispatcher = reinterpret_cast<MediaFramePresentDispatcher*>(handle->data);
    CHECK(dispatcher);

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::HandleScope handle_scope(isolate);

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> global = context->Global();

    v8::Local<v8::Function> cb_vp;
    bool has_vp = false;
    if (!dispatcher->cb_present_video_buffer_.IsEmpty())
    {
        cb_vp = dispatcher->cb_present_video_buffer_.Get(isolate);
        has_vp = !cb_vp->IsNullOrUndefined();
    }

    v8::Local<v8::Function> cb_eeof;
    bool has_eeof = false;
    if (!dispatcher->cb_error_or_eof_.IsEmpty())
    {
        cb_eeof = dispatcher->cb_error_or_eof_.Get(isolate);
        has_eeof = !cb_eeof->IsNullOrUndefined();
    }

    bool exited = false;
    dispatcher->present_queue_lock_.lock();
    while (!dispatcher->present_queue_.empty())
    {
        PresentRequest req = dispatcher->present_queue_.front();
        dispatcher->present_queue_.pop();
        dispatcher->present_queue_lock_.unlock();

        if (req.error_or_eof)
        {
            if (has_eeof)
                cb_eeof->Call(context, global, 0, nullptr).IsEmpty();

            exited = true;
            break;
        }
        else if (req.vbuffer && has_vp)
        {
            // TODO(sora): drop frames with too long latency
            v8::Local<v8::Value> obj = binder::Class<VideoBufferWrap>::create_object(
                    isolate, req.vbuffer);

            v8::Local<v8::Value> args[] = { obj, binder::to_v8(isolate, req.frame_pts_seconds) };
            cb_vp->Call(context, global, 2, args).IsEmpty();
        }

        dispatcher->present_queue_lock_.lock();
    }

    if (exited)
    {
        // Clear the queue to free buffers, lock is not needed
        // as other threads has exited
        while (!dispatcher->present_queue_.empty())
            dispatcher->present_queue_.pop();
    }
    else
    {
        dispatcher->present_queue_lock_.unlock();
    }
}

void MediaFramePresentDispatcher::PresentThreadCmdHandler(uv_async_t *handle)
{
    // Requested by main thread; called from present thread
    auto *thread_ctx = reinterpret_cast<PresentThreadContext*>(handle->data);
    CHECK(thread_ctx);

    if (!thread_ctx->cmd)
        return;

    PresentThreadCmd::Verb verb = thread_ctx->cmd->verb;
    if (verb == PresentThreadCmd::kTerminate)
    {
        thread_ctx->TryCloseLoopHandles();

        // Notify decoding thread to exit
        thread_ctx->queue_lock.lock();
        thread_ctx->decode_stop_flag = true;
        thread_ctx->queue_lock.unlock();
        thread_ctx->queue_cond.notify_one();
    }
    else if (verb == PresentThreadCmd::kPlay)
    {
        // `TimerCallback` will be called on the next loop iteration
        uv_timer_start(&thread_ctx->timer, TimerCallback, 0, 0);
    }
    else if (verb == PresentThreadCmd::kPause)
    {
        uv_timer_stop(&thread_ctx->timer);
    }
    else if (verb == PresentThreadCmd::kSeekTo)
    {
        // TODO(sora): implement this
    }

    thread_ctx->cmd->promise.set_value();
}

void MediaFramePresentDispatcher::SendAndWaitForPresentThreadCmd(int verb, int64_t param)
{
    PresentThreadCmd cmd{ static_cast<PresentThreadCmd::Verb>(verb), param };
    thread_ctx_->cmd = &cmd;
    uv_async_send(&thread_ctx_->thread_notifier);
    cmd.promise.get_future().wait();
    thread_ctx_->cmd = nullptr;
}

void MediaFramePresentDispatcher::play()
{
    if (disposed_)
        g_throw(Error, "Object has been disposed");

    if (!paused_)
        return;

    SendAndWaitForPresentThreadCmd(PresentThreadCmd::kPlay, 0);
    paused_ = false;
    MarkShouldEscapeGC(this);
}

void MediaFramePresentDispatcher::pause()
{
    if (disposed_)
        g_throw(Error, "Object has been disposed");

    if (paused_)
        return;

    SendAndWaitForPresentThreadCmd(PresentThreadCmd::kPause, 0);
    paused_ = true;
    MarkGCCollectable();
}

GALLIUM_BINDINGS_UTAU_NS_END
