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

#include <semaphore>
#include <mutex>

#include "Utau/AudioSinkStream.h"
#include "Gallium/bindings/multimedia/FrameScheduler.h"
#include "Gallium/bindings/multimedia/AudioStreamService.h"
#include "Gallium/bindings/multimedia/AChannelLayout.h"
#include "Gallium/bindings/multimedia/Frame.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

class AVFrameAutoFree
{
public:
    explicit AVFrameAutoFree(AVFrame *frame) : frame_(frame) {}
    AVFrameAutoFree(const AVFrameAutoFree& lhs) : frame_(nullptr) {
        if (lhs.frame_)
            frame_ = av_frame_clone(lhs.frame_);
    }
    AVFrameAutoFree(AVFrameAutoFree&& rhs) noexcept : frame_(rhs.frame_) {
        rhs.frame_ = nullptr;
    }
    ~AVFrameAutoFree() {
        if (frame_)
            av_frame_free(&frame_);
    }

    AVFrameAutoFree& operator=(const AVFrameAutoFree& lhs) {
        if (frame_)
            av_frame_free(&frame_);
        if (lhs.frame_)
            frame_ = av_frame_clone(lhs.frame_);
        return *this;
    }

    AVFrameAutoFree& operator=(AVFrameAutoFree&& rhs) {
        if (frame_)
            av_frame_free(&frame_);
        frame_ = rhs.frame_;
        rhs.frame_ = nullptr;
        return *this;
    }

    AVFrame *operator*() const {
        return frame_;
    }

    AVFrame *operator->() const {
        return frame_;
    }

    AVFrame *Discard() {
        AVFrame *value = frame_;
        frame_ = nullptr;
        return value;
    }

private:
    AVFrame *frame_;
};

FrameScheduler::Queue::~Queue()
{
    av_channel_layout_uninit(&sample_ch_layout);
    DiscardQueueFrames();
    DiscardPrebufQueueFrames();
}

void FrameScheduler::Queue::DiscardQueueFrames()
{
    while (!queue.empty())
    {
        AVFrame *frame = queue.front();
        queue.pop();
        av_frame_free(&frame);
    }
}

std::vector<FrameScheduler::PromisifiedEnqueueInfo>
FrameScheduler::Queue::DiscardPrebufQueueFrames()
{
    std::vector<PromisifiedEnqueueInfo> vec;
    while (!prebuf_queue.empty())
    {
        PromisifiedEnqueueInfo& info = prebuf_queue.front();
        av_frame_free(&info.frame);
        vec.emplace_back(std::move(info));
        prebuf_queue.pop();
    }

    return vec;
}

FrameSchedulerBuilder::FrameSchedulerBuilder()
    : asink_device_(nullptr)
    , asink_format_(AV_SAMPLE_FMT_NONE)
    , asink_sample_rate_(0)
    , asink_ch_layout_{}
{
}

FrameSchedulerBuilder::~FrameSchedulerBuilder()
{
    av_channel_layout_uninit(&asink_ch_layout_);
}

FrameSchedulerBuilder::QueueCreationInfo&
FrameSchedulerBuilder::GetOrAddQueueInfo(MediaType type)
{
    auto itr = queue_creation_info_.find(type);
    if (itr != queue_creation_info_.end())
        return itr->second;
    QueueCreationInfo& info = queue_creation_info_[type];
    info.type = type;
    info.watermark = 0;
    info.timebase = {0, 1};
    return info;
}

ffi::RetLocal<v8::Value>
FrameSchedulerBuilder::addQueue(ffi::Enum<MediaType> type, int32_t watermark,
                                const AVRationalAdapter& timebase,
                                const ffi::IFace<FrameSchedulerQueueOptions>& options)
{
    if (watermark < 0)
        return ffi::Fail(ffi::kErr, "invalid value for queue watermark");
    AVRational tb_q = *timebase;
    if (tb_q.num * tb_q.den <= 0)
        return ffi::Fail(ffi::kErr, "invalid value for timebase");

    auto& info = GetOrAddQueueInfo(*type);
    info.watermark = watermark;
    info.timebase = tb_q;
    if (options->emits_present_event)
        info.emit_present_event = *options->emits_present_event;
    if (options->requires_frame_feedback)
        info.requires_frame_feedback = *options->requires_frame_feedback;

    return GetThisHandle(v8::Isolate::GetCurrent());
}

ffi::RetLocal<v8::Value>
FrameSchedulerBuilder::setAudioSinkCreationInfo(ffi::Class<AudioStreamService> service,
                                                ffi::Enum<SampleFormat> format,
                                                int32_t sample_rate,
                                                ffi::Class<AChannelLayout> ch_layout)
{
    if (service->GetDisposeState() != DisposeState::kNot)
        return ffi::Fail(ffi::kErr, "provided AudioStreamService has been disposed");
    if (sample_rate <= 0)
        return ffi::Fail(ffi::kRangeErr, "invalid sample rate");
    asink_device_ = service->GetDevice();
    asink_format_ = static_cast<AVSampleFormat>(*format);
    asink_sample_rate_ = sample_rate;
    ch_layout->CopyLayoutTo(asink_ch_layout_);
    return GetThisHandle(v8::Isolate::GetCurrent());
}

ffi::RetLocal<v8::Value> FrameSchedulerBuilder::build()
{
    if (queue_creation_info_.empty())
        return ffi::Fail(ffi::kErr, "no queue has been added");
    if (asink_device_ && !queue_creation_info_.contains(MediaType::kAudio))
        return ffi::Fail(ffi::kErr, "an audio sink is present but no audio queue has been added");

    std::map<MediaType, FrameScheduler::Queue> queues;
    for (const auto& [type, queue_info] : queue_creation_info_)
    {
        const char *type_name = av_get_media_type_string(static_cast<AVMediaType>(type));
        if (queue_info.watermark <= 0)
            return ffi::Fail(ffi::kErr, fmt::format("watermark has not been set for {} queue", type_name));
        if (queue_info.timebase.num == 0)
            return ffi::Fail(ffi::kErr, fmt::format("timebase has not been set for {} queue", type_name));

        FrameScheduler::Queue& q = queues[type];
        q.media_type = type;
        q.watermark = queue_info.watermark;
        q.timebase = queue_info.timebase;
        q.emit_present_event = queue_info.emit_present_event;
        q.requires_frame_feedback = queue_info.requires_frame_feedback;
        if (type == MediaType::kAudio && asink_device_)
        {
            q.sample_format = asink_format_;
            q.sample_rate = asink_sample_rate_;
            av_channel_layout_copy(&q.sample_ch_layout, &asink_ch_layout_);
            q.verifier = [](FrameScheduler::Queue *self, const AVFrame *f) {
                return (self->sample_format == f->format) &&
                       (self->sample_rate == f->sample_rate) &&
                       av_channel_layout_compare(&self->sample_ch_layout, &f->ch_layout) == 0;
            };
        }
    }

    // Select a main sync queue
    if (queues.contains(MediaType::kAudio))
        queues[MediaType::kAudio].is_main_sync = true;
    else if (queues.contains(MediaType::kVideo))
        queues[MediaType::kVideo].is_main_sync = true;

    std::shared_ptr<utau::AudioSinkStream> asink_stream;
    if (asink_device_)
    {
        asink_stream = asink_device_->CreateSinkStream(
                "playback", asink_format_, asink_sample_rate_, asink_ch_layout_, true);
        if (!asink_stream)
            return ffi::Fail(ffi::kErr, "failed to create an audio sink stream");
    }

    queue_creation_info_.clear();
    asink_device_.reset();
    asink_format_ = AV_SAMPLE_FMT_NONE;
    asink_sample_rate_ = 0;
    av_channel_layout_uninit(&asink_ch_layout_);
    NotifyDisposeState(DisposeState::kDisposed);

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    uv_loop_t *event_loop = EventLoop::GetCurrent()->handle();
    return ffi::JSObject::New<FrameScheduler>(
            isolate, event_loop, std::move(queues), std::move(asink_stream));
}

struct FrameScheduler::ThreadContext
{
    uv_loop_t event_loop{};
    std::unique_ptr<uv::AsyncHandle> notifier;
    std::unique_ptr<uv::TimerHandle> timer;

    std::binary_semaphore thread_init_sem_{0};
    std::queue<ControlCmd> control_cmd_queue_;
    std::mutex control_cmd_queue_lock_;

    bool has_pending_dispatch = false;
    bool first_frame = true;
    bool is_paused = false;

    struct TimePointPair
    {
        TimePoint sys_clock;
        double frame_pts_sec;
    };
    TimePointPair ref_timepoint;
};

FrameScheduler::FrameScheduler(uv_loop_t *event_loop,
                               std::map<MediaType, Queue> queues,
                               std::shared_ptr<utau::AudioSinkStream> asink_stream)
    : thread_ctx_(std::make_unique<ThreadContext>())
    , thread_(&FrameScheduler::SchedulerThread, this)
    , queues_(std::move(queues))
    , asink_stream_(std::move(asink_stream))
    , main_thread_notifier_(event_loop, [this]() { OnMainThreadNotify(); })
{
    // unref the handle so that it does not prevent the main loop from exiting
    // when there are no event listeners.
    main_thread_notifier_.UnrefCounted();

    EmitterDefineEvent("empty-queue", [this]() {
        emit_empty_queue_ = EmitterWrapAsCallable("empty-queue");
        main_thread_notifier_.RefCounted();
        return 0;
    }, [this](uint64_t) {
        emit_empty_queue_ = {};
        main_thread_notifier_.UnrefCounted();
    });

    EmitterDefineEvent("present", [this]() {
        emit_present_ = EmitterWrapAsCallable("present");
        main_thread_notifier_.RefCounted();
        return 0;
    }, [this](uint64_t) {
        emit_present_ = {};
        main_thread_notifier_.UnrefCounted();
    });

    // wait until the scheduler thread finishes initialization
    thread_ctx_->thread_init_sem_.acquire();
}

FrameScheduler::~FrameScheduler()
{
    if (GetDisposeState() == DisposeState::kNot)
        dispose();
}

void FrameScheduler::EnqueueControlCmd(const ControlCmd& cmd)
{
    std::scoped_lock<std::mutex> lock(thread_ctx_->control_cmd_queue_lock_);
    thread_ctx_->control_cmd_queue_.push(cmd);
    thread_ctx_->notifier->Send();
}

ffi::Ret<void> FrameScheduler::dispose()
{
    // Notify and wait for the scheduler exiting
    EnqueueControlCmd({ControlCmd::Op::kTerminate});
    thread_.join();

    if (emit_empty_queue_)
        emit_empty_queue_ = {};

    if (asink_stream_)
    {
        asink_stream_->Dispose();
        asink_stream_.reset();
    }

    main_thread_notifier_.Close();
    // remaining AVFrames are freed by `Queue::~Queue()`
    queues_.clear();

    NotifyDisposeState(DisposeState::kDisposed);
    return {};
}

ffi::Ret<int32_t> FrameScheduler::enqueue(ffi::Enum<MediaType> media_type,
                                          ffi::Class<Frame> frame)
{
    auto itr = queues_.find(*media_type);
    if (itr == queues_.end())
    {
        return ffi::Fail(ffi::kErr, fmt::format(
                "the scheduler does not have a(n) {} queue",
                av_get_media_type_string(static_cast<AVMediaType>(*media_type))));
    }
    if (frame->GetDisposeState() != DisposeState::kNot)
        return ffi::Fail(ffi::kErr, "provided `Frame` instance has been disposed");
    const AVFrame *avframe = frame->GetAVFrame();

    Queue& queue = itr->second;
    if (queue.verifier && !queue.verifier(&queue, avframe))
        return ffi::Fail(ffi::kErr, "the frame does not satisfy all the constraints");

    std::scoped_lock<std::mutex> lock(queue.lock);
    if (queue.queue.size() >= queue.watermark)
        return static_cast<int32_t>(FrameSchedulerStatus::kFull);

    queue.queue.push(av_frame_clone(avframe));
    thread_ctx_->notifier->Send();
    return static_cast<int32_t>(FrameSchedulerStatus::kSuccess);
}

ffi::RetLocal<v8::Value> FrameScheduler::enqueuePromise(ffi::Enum<MediaType> media_type,
                                                        ffi::Class<Frame> frame)
{
    auto itr = queues_.find(*media_type);
    if (itr == queues_.end())
    {
        return ffi::Fail(ffi::kErr, fmt::format(
                "the scheduler does not have a(n) {} queue",
                av_get_media_type_string(static_cast<AVMediaType>(*media_type))));
    }
    if (frame->GetDisposeState() != DisposeState::kNot)
        return ffi::Fail(ffi::kErr, "provided `Frame` instance has been disposed");
    const AVFrame *avframe = frame->GetAVFrame();

    Queue& queue = itr->second;
    if (queue.verifier && !queue.verifier(&queue, avframe))
        return ffi::Fail(ffi::kErr, "the frame does not satisfy all the constraints");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> jsctx = isolate->GetCurrentContext();
    auto resolver = v8::Promise::Resolver::New(jsctx).ToLocalChecked();

    queue.lock.lock();
    if (queue.queue.size() < queue.watermark)
    {
        // the queue has not been full yet, so we enqueue the frame and fulfill
        // the promise immediately.
        queue.queue.push(av_frame_clone(avframe));
        thread_ctx_->notifier->Send();
        queue.lock.unlock();

        resolver->Resolve(jsctx, v8::Null(isolate)).Check();
        return resolver->GetPromise();
    }
    queue.lock.unlock();

    // The queue has been full, and we cannot enqueue the frame now.
    // Store the frame into another queue and enqueue it in the future.
    queue.prebuf_queue.emplace(av_frame_clone(avframe), isolate, resolver);
    main_thread_notifier_.RefCounted();

    return resolver->GetPromise();
}

ffi::Ret<void> FrameScheduler::pause(ffi::Enum<FrameSchedulerPausePolicy> policy)
{
    EnqueueControlCmd({ .op = ControlCmd::Op::kPause, .args = { policy.GetInteger() } });
    return {};
}

ffi::Ret<void> FrameScheduler::resume()
{
    EnqueueControlCmd({ .op = ControlCmd::Op::kResume });
    return {};
}

void FrameScheduler::SchedulerThread()
{
    pthread_setname_np(pthread_self(), "MediaFrameSched");

    std::unique_ptr<ThreadContext>& ctx = thread_ctx_;
    uv_loop_t *event_loop = &ctx->event_loop;
    CHECK(uv_loop_init(event_loop) >= 0);

    ctx->notifier = std::make_unique<uv::AsyncHandle>(event_loop, [&]() {
        // First, we should check the control messages
        ctx->control_cmd_queue_lock_.lock();
        while (!ctx->control_cmd_queue_.empty())
        {
            ControlCmd cmd = ctx->control_cmd_queue_.front();
            ctx->control_cmd_queue_.pop();
            if (cmd.op == ControlCmd::Op::kTerminate)
            {
                ctx->control_cmd_queue_lock_.unlock();
                ctx->timer->Close();
                ctx->notifier->Close();
                return;
            }
            else if (cmd.op == ControlCmd::Op::kPause)
            {
                EnterPauseState(static_cast<FrameSchedulerPausePolicy>(cmd.args[0]));
            }
            else if (cmd.op == ControlCmd::Op::kResume)
            {
                ResumeFromPauseState();
            }
        }
        ctx->control_cmd_queue_lock_.unlock();

        if (!ctx->is_paused)
            DispatchNextFrame();
    });

    ctx->timer = std::make_unique<uv::TimerHandle>(event_loop);

    ctx->thread_init_sem_.release();
    uv_run(event_loop, UV_RUN_DEFAULT);

    uv_loop_close(event_loop);
}

void FrameScheduler::EnterPauseState(FrameSchedulerPausePolicy policy)
{
    if (thread_ctx_->is_paused)
        return;

    thread_ctx_->timer->Stop();
    thread_ctx_->has_pending_dispatch = false;
    // Reset the first_frame flag to force the scheduler to update the reftime
    // based on the first frame after pausing.
    thread_ctx_->first_frame = true;
    thread_ctx_->is_paused = true;

    switch (policy)
    {
    case FrameSchedulerPausePolicy::kDiscardAndReject:
    {
        for (auto& [type, queue] : queues_)
        {
            auto infos = queue.DiscardPrebufQueueFrames();
            SubmitMainThreadTask([infos = std::move(infos)]() mutable {
                v8::Isolate *isolate = v8::Isolate::GetCurrent();
                v8::HandleScope handle_scope(isolate);
                v8::Local<v8::Context> jsctx = isolate->GetCurrentContext();
                auto msg = v8::String::NewFromUtf8Literal(isolate, "pause with DiscardAndReject policy");
                for (PromisifiedEnqueueInfo& info : infos) {
                    info.resolver.Get(isolate)->Reject(jsctx, msg).Check();
                }
            });
        }
    }
    [[fallthrough]];

    case FrameSchedulerPausePolicy::kDiscard:
        for (auto& [type, queue] : queues_)
            queue.DiscardQueueFrames();
        break;

    default:
        break;
    }
}

void FrameScheduler::ResumeFromPauseState()
{
    if (!thread_ctx_->is_paused)
        return;

    // Just reset the paused flag, and `DispatchNextFrame()` will be called
    // automatically by the caller of `ResumeFromPauseState()`.
    thread_ctx_->is_paused = false;
}

void FrameScheduler::DispatchNextFrame()
{
    if (queues_.empty() || thread_ctx_->has_pending_dispatch)
        return;

    double min_pts = std::numeric_limits<double>::max();
    AVFrame *min_pts_frame = nullptr;
    Queue *min_pts_queue = nullptr;
    for (auto& [type, queue] : queues_)
    {
        std::scoped_lock<std::mutex> lock(queue.lock);
        if (queue.queue.empty())
            continue;

        AVFrame *frame = queue.queue.front();
        double pts = (static_cast<double>(frame->pts) / queue.timebase.den) * queue.timebase.num;
        if (pts <= min_pts)
        {
            min_pts = pts;
            min_pts_frame = frame;
            min_pts_queue = &queue;
        }
    }

    if (!min_pts_queue)
    {
        thread_ctx_->has_pending_dispatch = false;
        return;
    }

    min_pts_queue->lock.lock();
    min_pts_queue->queue.pop();
    if (min_pts_queue->queue.empty())
    {
        // Emit event: empty-queue
        SubmitMainThreadTask([media_type = min_pts_queue->media_type, this] {
            if (!emit_empty_queue_)
                return;
            v8::Isolate *isolate = v8::Isolate::GetCurrent();
            v8::HandleScope handle_scope(isolate);
            emit_empty_queue_({ v8::Uint32::New(isolate, static_cast<int32_t>(media_type)) });
        });
    }
    min_pts_queue->lock.unlock();

    // Now we can notify the main thread to transfer some pending frames (if there are)
    // into the queue, since a frame has just been pop, and there are some spaces in the queue.
    SubmitMainThreadTask([q = min_pts_queue, this] {
        if (q->prebuf_queue.empty())
            return;

        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Context> jsctx = isolate->GetCurrentContext();
        while (!q->prebuf_queue.empty())
        {
            std::scoped_lock<std::mutex> lock(q->lock);
            if (q->queue.size() >= q->watermark)
                break;

            PromisifiedEnqueueInfo& info = q->prebuf_queue.front();
            q->queue.push(info.frame);
            thread_ctx_->notifier->Send();

            info.resolver.Get(isolate)->Resolve(jsctx, v8::Null(isolate)).Check();
            main_thread_notifier_.UnrefCounted();

            q->prebuf_queue.pop();
        }
    });

    // For first frame, initialize the reftime
    ThreadContext::TimePointPair& reftime = thread_ctx_->ref_timepoint;
    if (thread_ctx_->first_frame)
    {
        reftime.frame_pts_sec = min_pts;
        reftime.sys_clock = std::chrono::steady_clock::now();
        thread_ctx_->first_frame = false;
    }

    // Compute time based on the system clock
    int64_t pts_from_reftime_us = static_cast<int64_t>(
            std::round((min_pts - reftime.frame_pts_sec) * 1e6));
    TimePoint sysclk_now = std::chrono::steady_clock::now();
    int64_t delta_us = std::chrono::duration_cast<std::chrono::microseconds>(
            reftime.sys_clock + std::chrono::microseconds(pts_from_reftime_us) - sysclk_now).count();

    if (delta_us < 0)
    {
        delta_us = 0;

        // If this queue is main sync, set `first_time` flag to force the reftime
        // to be reset in the next dispatch round.
        if (min_pts_queue->is_main_sync)
            thread_ctx_->first_frame = true;
        else
        {
            // drop expired frames
            av_frame_free(&min_pts_frame);
        }
    }

    // TODO(sora): consider the audio stream delay (asink_stream_->GetDelayInUs())

    thread_ctx_->timer->Start(delta_us / 1000, 0,
    [this, pts = min_pts, q = min_pts_queue, frame = AVFrameAutoFree(min_pts_frame)]() mutable {
        thread_ctx_->has_pending_dispatch = false;
        DispatchNextFrame();

        // the frame was dropped
        if (*frame == nullptr)
        {
            q->dropped_frames++;
            return;
        }

        if (q->media_type == MediaType::kAudio)
        {
            if (asink_stream_)
                asink_stream_->Enqueue(*frame);
        }

        AVFrameAutoFree feedback_frame(nullptr);
        if (q->emit_present_event)
        {
            if (q->requires_frame_feedback)
                feedback_frame = std::move(frame);

            SubmitMainThreadTask([q, pts, cbframe = std::move(feedback_frame), this]() mutable {
                if (!emit_present_)
                    return;
                v8::Isolate *isolate = v8::Isolate::GetCurrent();
                v8::HandleScope handle_scope(isolate);
                emit_present_({
                    v8::Uint32::New(isolate, static_cast<int32_t>(q->media_type)),
                    v8::Number::New(isolate, pts),
                    *cbframe ? ffi::JSObject::New<Frame>(isolate, cbframe.Discard()).As<v8::Value>()
                             : v8::Null(isolate).As<v8::Value>()
                });
            });
        }
    });

    thread_ctx_->has_pending_dispatch = true;
}

void FrameScheduler::SubmitMainThreadTask(std::function<void()> task)
{
    std::scoped_lock<std::mutex> lock(main_thread_task_lock_);
    main_thread_task_queue_.emplace(std::move(task));
    main_thread_notifier_.Send();
}

void FrameScheduler::OnMainThreadNotify()
{
    main_thread_task_lock_.lock();
    while (!main_thread_task_queue_.empty())
    {
        std::function<void()> task = std::move(main_thread_task_queue_.front());
        main_thread_task_queue_.pop();
        main_thread_task_lock_.unlock();

        task();
    }
    main_thread_task_lock_.unlock();
}

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
