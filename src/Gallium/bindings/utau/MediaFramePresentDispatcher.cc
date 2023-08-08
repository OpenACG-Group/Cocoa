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

#include "Core/EventLoop.h"
#include "Core/Journal.h"
#include "Core/TraceEvent.h"
#include "Gallium/bindings/utau/Exports.h"
#include "Gallium/bindings/utau/MediaFramePresentDispatcher.h"
#include "Gallium/binder/Class.h"
#include "Gallium/binder/Convert.h"
#include "Utau/ffwrappers/libavutil.h"
GALLIUM_BINDINGS_UTAU_NS_BEGIN

using DecodeResult = utau::AVStreamDecoder::AVGenericDecoded;

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.bindings.utau.MediaFramePresentDispatcher)

#define AUDIO_QUEUE_MAX_FRAMES 20
#define VIDEO_QUEUE_MAX_FRAMES 5
#define PRESENT_QUEUE_MAX_FRAMES 32

struct MediaFramePresentDispatcher::PresentThreadCmd
{
    enum Verb
    {
        kTerminate,
        kPause,
        kPlay,

        // param[0]: timestamp in milliseconds; param[1]: time tolerance in milliseconds
        kSeekTo
    };

    Verb verb;
    int64_t param[3];
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

        StartDecodingThread();
    }

    void DecodingThreadRoutine();

    void TryCloseLoopHandles()
    {
        if (loop_handles_closed)
            return;

        uv_close(reinterpret_cast<uv_handle_t*>(&thread_notifier), nullptr);
        uv_close(reinterpret_cast<uv_handle_t*>(&timer), nullptr);
        loop_handles_closed = true;
    }

    void ClearQueues()
    {
        while (!audio_queue.empty())
            audio_queue.pop();
        while (!video_queue.empty())
            video_queue.pop();
    }

    void ClearLastFrameStates()
    {
        last_frame.type = DecodeResult::kNull;
        last_frame.audio.reset();
        last_frame.video.reset();
        last_frame_pts = 0;
        last_required_intv_ms = 0;
        last_delay_compensated_intv_ms = 0;
    }

    void StopDecodingThread()
    {
        {
            std::scoped_lock<std::mutex> lock(queue_lock);
            decode_stop_flag = true;
        }
        queue_cond.notify_one();
        if (decoding_thread.joinable())
            decoding_thread.join();
    }

    void StartDecodingThread()
    {
        decode_stop_flag = false;
        decoding_thread = std::thread(&PresentThreadContext::DecodingThreadRoutine, this);
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

    bool seek_requested_ = false;
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
                return ((audio_queue.size() < AUDIO_QUEUE_MAX_FRAMES &&
                         video_queue.size() < VIDEO_QUEUE_MAX_FRAMES) ||
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
    , present_queue_full_(false)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    decoder_wrap_ = binder::UnwrapObject<AVStreamDecoderWrap>(isolate, decoder);
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

        asinkstream_wrap_ = binder::UnwrapObject<AudioSinkStreamWrap>(isolate, audioSinkStream);
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
    uv_loop_t *main_thread_loop = EventLoop::GetCurrent()->handle();
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
        SendAndWaitForPresentThreadCmd(PresentThreadCmd::kTerminate, nullptr);
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

void MediaFramePresentDispatcher::setOnAudioPresentNotify(v8::Local<v8::Value> func)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!func->IsFunction())
        g_throw(TypeError, "Property `onAudioPresentNotify` must be a function");
    cb_audio_present_notify_.Reset(isolate, v8::Local<v8::Function>::Cast(func));
}

v8::Local<v8::Value> MediaFramePresentDispatcher::getOnAudioPresentNotify()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (cb_audio_present_notify_.IsEmpty())
        return v8::Null(isolate);
    return cb_audio_present_notify_.Get(isolate);
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
    TRACE_EVENT("multimedia", "MediaFramePresentDispatcher::TimerCallback");

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

    double head_audio_pts = -1;
    double head_video_pts = -1;

    // Drop old frames
    while (!thread_ctx->video_queue.empty())
    {
        auto& head = thread_ctx->video_queue.front();
        if (head.pts < thread_ctx->last_frame_pts)
        {
            thread_ctx->video_queue.pop();
        }
        else
        {
            head_video_pts = head.pts;
            break;
        }
    }

    if (!thread_ctx->audio_queue.empty())
    {
        head_audio_pts = thread_ctx->audio_queue.front().pts;
    }

    double pts;
    if (head_video_pts >= 0 && (head_video_pts < head_audio_pts || head_audio_pts < 0))
    {
        thread_ctx->last_frame.type = DecodeResult::kVideo;
        thread_ctx->last_frame.video = std::move(thread_ctx->video_queue.front().buffer);
        pts = head_video_pts;
        thread_ctx->video_queue.pop();
    }
    else if (head_audio_pts >= 0)
    {
        thread_ctx->last_frame.type = DecodeResult::kAudio;
        thread_ctx->last_frame.audio = std::move(thread_ctx->audio_queue.front().buffer);
        pts = head_audio_pts;
        thread_ctx->audio_queue.pop();
    }
    else
    {
        // No frames available
        thread_ctx->queue_cond.notify_one();
        uv_timer_start(timer, TimerCallback, 0, 0);
        return;
    }

    thread_ctx->queue_cond.notify_one();

    if (thread_ctx->seek_requested_)
    {
        thread_ctx->last_frame_pts = pts;
        thread_ctx->seek_requested_ = false;
        uv_timer_start(timer, TimerCallback, 0, 0);
    }
    else
    {
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
}

void MediaFramePresentDispatcher::SendPresentRequest(DecodeResult frame, double pts_seconds)
{
    //
    PresentQueueFullPresentThreadCheckpoint();

    if (frame.type == DecodeResult::kAudio)
        asinkstream_wrap_->GetStream()->Enqueue(*frame.audio);

    std::scoped_lock<std::mutex> lock(present_queue_lock_);
    present_queue_.emplace(PresentRequest{
        .error_or_eof = false,
        .send_timestamp = utau::GlobalContext::Ref().GetCurrentTimestampMs(),
        .frame_pts_seconds = pts_seconds,
        .vbuffer = std::move(frame.video),
        .abuffer = std::move(frame.audio)
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

    v8::Local<v8::Function> cb_audiopts;
    bool has_audiopts = false;
    if (!dispatcher->cb_audio_present_notify_.IsEmpty())
    {
        cb_audiopts = dispatcher->cb_audio_present_notify_.Get(isolate);
        has_audiopts = !cb_audiopts->IsNullOrUndefined();
    }

    std::vector<PresentRequest> reqs;
    dispatcher->present_queue_lock_.lock();
    while (!dispatcher->present_queue_.empty())
    {
        PresentRequest req = dispatcher->present_queue_.front();
        dispatcher->present_queue_.pop();
        dispatcher->PresentQueueFullHostCheckpoint();

        if (req.error_or_eof)
        {
            while (!dispatcher->present_queue_.empty())
                dispatcher->present_queue_.pop();
        }

        reqs.push_back(req);
    }
    dispatcher->present_queue_lock_.unlock();

    for (const auto& req : reqs)
    {
        if (req.error_or_eof)
        {
            if (has_eeof)
                cb_eeof->Call(context, global, 0, nullptr).IsEmpty();
        }
        else if (req.abuffer && has_audiopts)
        {
            v8::Local<v8::Value> obj = binder::NewObject<AudioBufferWrap>(
                    isolate, req.abuffer);

            v8::Local<v8::Value> args[] = { obj, v8::Number::New(isolate, req.frame_pts_seconds) };
            cb_audiopts->Call(context, global, 2, args).IsEmpty();

            binder::UnwrapObject<AudioBufferWrap>(isolate, obj)->dispose();
        }
        else if (req.vbuffer && has_vp)
        {
            // TODO(sora): drop frames with too long latency
            v8::Local<v8::Value> obj = binder::NewObject<VideoBufferWrap>(
                    isolate, req.vbuffer);

            v8::Local<v8::Value> args[] = { obj, v8::Number::New(isolate, req.frame_pts_seconds) };
            cb_vp->Call(context, global, 2, args).IsEmpty();

            binder::UnwrapObject<VideoBufferWrap>(isolate, obj)->dispose();
        }
    }
}

void MediaFramePresentDispatcher::PresentQueueFullHostCheckpoint()
{
    if (present_queue_full_ && present_queue_.size() <= PRESENT_QUEUE_MAX_FRAMES)
    {
        present_queue_full_ = false;
        present_queue_full_cond_.notify_one();
    }
}

void MediaFramePresentDispatcher::PresentQueueFullPresentThreadCheckpoint()
{
    std::unique_lock<std::mutex> lock(present_queue_lock_);
    if (present_queue_.size() <= PRESENT_QUEUE_MAX_FRAMES)
        return;

    present_queue_full_ = true;
    present_queue_full_cond_.wait(lock, [=]() {
        return !present_queue_full_;
    });
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
        thread_ctx->StopDecodingThread();
        thread_ctx->ClearQueues();
        thread_ctx->ClearLastFrameStates();

        double ts_sec = (double)thread_ctx->cmd->param[0] / 1000;

        if (thread_ctx->dispatcher->has_audio_)
        {
            AVRational tb = av_make_q(thread_ctx->dispatcher->audio_stinfo_.time_base.num,
                                      thread_ctx->dispatcher->audio_stinfo_.time_base.denom);

            bool res = thread_ctx->decoder->SeekStreamTo(utau::AVStreamDecoder::kAudio_StreamType,
                                                         static_cast<int64_t>(ts_sec / av_q2d(tb)));
            if (!res)
            {
                QLOG(LOG_ERROR, "Failed to seek audio stream");
                thread_ctx->dispatcher->SendErrorOrEOFRequest();
                thread_ctx->cmd->promise.set_value();
                return;
            }

            thread_ctx->decoder->FlushDecoderBuffers(utau::AVStreamDecoder::kAudio_StreamType);
        }

        if (thread_ctx->dispatcher->has_video_)
        {
            AVRational tb = av_make_q(thread_ctx->dispatcher->video_stinfo_.time_base.num,
                                      thread_ctx->dispatcher->video_stinfo_.time_base.denom);

            bool res = thread_ctx->decoder->SeekStreamTo(utau::AVStreamDecoder::kVideo_StreamType,
                                                         static_cast<int64_t>(ts_sec / av_q2d(tb)));
            if (!res)
            {
                QLOG(LOG_ERROR, "Failed to seek video stream");
                thread_ctx->dispatcher->SendErrorOrEOFRequest();
                thread_ctx->cmd->promise.set_value();
                return;
            }

            thread_ctx->decoder->FlushDecoderBuffers(utau::AVStreamDecoder::kVideo_StreamType);
        }

        // Start decoding again
        thread_ctx->StartDecodingThread();
        thread_ctx->seek_requested_ = true;
    }

    thread_ctx->cmd->promise.set_value();
}

void MediaFramePresentDispatcher::SendAndWaitForPresentThreadCmd(int verb, const int64_t param[3])
{
    static const int64_t default_params[3] = {0, 0, 0};
    if (!param)
        param = default_params;

    PresentThreadCmd cmd{
        static_cast<PresentThreadCmd::Verb>(verb),
        { param[0], param[1], param[2] }
    };
    thread_ctx_->cmd = &cmd;
    uv_async_send(&thread_ctx_->thread_notifier);
    cmd.promise.get_future().wait();
    thread_ctx_->cmd = nullptr;
}

void MediaFramePresentDispatcher::seekTo(double tsSeconds)
{
    if (disposed_)
        g_throw(Error, "Object has been disposed");

    if (tsSeconds < 0)
        g_throw(RangeError, "Argument `tsSeconds` must be a positive number");

    int64_t params[3] = { static_cast<int64_t>(tsSeconds * 1000), 0, 0 };
    SendAndWaitForPresentThreadCmd(PresentThreadCmd::kSeekTo, params);
}

void MediaFramePresentDispatcher::play()
{
    if (disposed_)
        g_throw(Error, "Object has been disposed");

    if (!paused_)
        return;

    SendAndWaitForPresentThreadCmd(PresentThreadCmd::kPlay, nullptr);
    paused_ = false;
    MarkShouldEscapeGC(this);
}

void MediaFramePresentDispatcher::pause()
{
    if (disposed_)
        g_throw(Error, "Object has been disposed");

    if (paused_)
        return;

    SendAndWaitForPresentThreadCmd(PresentThreadCmd::kPause, nullptr);
    paused_ = true;
    MarkGCCollectable();
}

GALLIUM_BINDINGS_UTAU_NS_END
