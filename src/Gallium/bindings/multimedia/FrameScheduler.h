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

#ifndef COCOA_GALLIUM_BINDINGS_MULTIMEDIA_FRAMESCHEDULER_H
#define COCOA_GALLIUM_BINDINGS_MULTIMEDIA_FRAMESCHEDULER_H

#include <queue>
#include <list>
#include <thread>

#include "Core/EventLoop.h"
#include "Gallium/bindings/multimedia/Types.h"
#include "Gallium/bindings/EventEmitter.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Class.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/Interface.h"

namespace cocoa::utau {
class AudioDevice;
class AudioSinkStream;
}

GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

class Frame;
class AudioStreamService;
class AChannelLayout;

//! TSDecl: @interface FrameSchedulerQueueOptions
struct FrameSchedulerQueueOptions
{
    //! @tsdocbegin
    //! Whether to trigger a `present` event when a frame in the queue is represented.
    //! Default is false.
    //! @tsdocend
    //! TSDecl: @property @optional emitsPresentEvent: boolean
    ffi::Opt<bool> emits_present_event;

    //! @tsdocbegin
    //! Whether to feedback a `Frame` instance in the `present` event. The provided
    //! instance clones the original instance passed to `enqueue()` or `enqueuePromise()`.
    //! Default is false.
    //! @tsdocend
    //! TSDecl: @property @optional requiresFrameFeedback: boolean
    ffi::Opt<bool> requires_frame_feedback;
};
//! TSDecl: @end

//! @tsdocbegin
//! Helper class to build a `FrameScheduler` instance. See `FrameScheduler` for more details.
//! @tsdocend
//! TSDecl: @class FrameSchedulerBuilder
class FrameSchedulerBuilder : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    //! TSDecl: @constructor()
    FrameSchedulerBuilder();
    ~FrameSchedulerBuilder() override;

    //! @tsdocbegin
    //! Add a queue into the scheduler, with specified media type, watermark, and timebase.
    //! Values in `options` controls some detailed behaviours of the scheduler, see
    //! `FrameSchedulerQueueOptions` for more details.
    //!
    //! For a certain media type, there is only one queue can be added. If the method
    //! is called more than once with the same media type, the newer parameters will be used.
    //! But for `options`, only present fields (fields which are not absent) will overwrite
    //! the older value.
    //!
    //! @param type         Media type of the queue, uniquely identifies a queue.
    //! @param watermark    The maximum number of frames that could be stored in the queue.
    //! @param timebase     Timebase of the queue, which all frames' pts are assumed to be in.
    //! @param options      Detailed options of the queue.
    //! @tsdocend
    //! TSDecl: @method addQueue(type: MediaType, watermark: i32, timebase: Rational,
    //! TSDecl:                  options: FrameSchedulerQueueOptions): FrameSchedulerBuilder
    ffi::RetLocal<v8::Value> addQueue(ffi::Enum<MediaType> type, int32_t watermark,
                                      const AVRationalAdapter& timebase,
                                      const ffi::IFace<FrameSchedulerQueueOptions>& options);

    //! @tsdocbegin
    //! Set an audio sink for the audio queue (if we have).
    //! If set, an audio sink stream will be created from the provided `service`, with specified
    //! `format`, `sampleRate`, and `channelLayout`. That audio sink stream will be used to
    //! receive expired audio frames. When the scheduler is disposed, the stream will be disposed
    //! together.
    //!
    //! Although the user can require a frame feedback (by queue option `requiresFrameFeedback`)
    //! for audio queue, and enqueue that frame into the user created `AudioSinkStream` instance
    //! to play it, setting an audio sink directly by `setAudioSinkCreationInfo()` can reduce audio
    //! delay and provide better performance.
    //! @tsdocend
    //! TSDecl: @method setAudioSinkCreationInfo(service: AudioStreamService,
    //! TSDecl:                                  format: SampleFormat,
    //! TSDecl:                                  sampleRate: i32,
    //! TSDecl:                                  channelLayout: AChannelLayout): FrameSchedulerBuilder
    ffi::RetLocal<v8::Value> setAudioSinkCreationInfo(ffi::Class<AudioStreamService> service,
                                                      ffi::Enum<SampleFormat> format,
                                                      int32_t sample_rate,
                                                      ffi::Class<AChannelLayout> ch_layout);

    //! @tsdocbegin
    //! Creates a scheduler instance using the specified configurations,
    //! then disposes the builder.
    //! Throws an exception for invalid configuration.
    //! @tsdocend
    //! TSDecl: @method build(): FrameScheduler
    ffi::RetLocal<v8::Value> build();

private:
    struct QueueCreationInfo
    {
        MediaType type;
        int32_t watermark;
        AVRational timebase;
        bool emit_present_event = false;
        bool requires_frame_feedback = false;
    };

    QueueCreationInfo& GetOrAddQueueInfo(MediaType type);

    std::map<MediaType, QueueCreationInfo> queue_creation_info_;

    std::shared_ptr<utau::AudioDevice> asink_device_;
    AVSampleFormat asink_format_;
    int32_t asink_sample_rate_;
    AVChannelLayout asink_ch_layout_;
};
//! TSDecl: @end

//! TSDecl: @enum FrameSchedulerStatus
enum class FrameSchedulerStatus
{
    //! TSDecl: @enumitem Success
    kSuccess,

    //! @tsdocbegin
    //! Number of frames in the queue exceeds the watermark, and the new frame
    //! cannot be enqueued until frames in the queue has been consumed.
    //! @tsdocend
    //! TSDecl: @enumitem Full
    kFull
};
//! TSDecl: @end

//! @tsdocbegin
//! Controls the behaviour of frame scheduler when `pause()` is called.
//! @tsdocend
//! TSDecl: @enum FrameSchedulerPausePolicy
enum class FrameSchedulerPausePolicy
{
    //! @tsdocbegin
    //! Keep the frames in the queue, and pause the internal timer.
    //! @tsdocend
    //! TSDecl: @enumitem Conserve
    kConserve,

    //! @tsdocbegin
    //! Discard all the frames in the queue, not including pending frames from `enqueuePromise()`,
    //! and pause the internal timer.
    //! @tsdocend
    //! TSDecl: @enumitem Discard
    kDiscard,

    //! @tsdocbegin
    //! Discard all the frames in the queue, including pending frames from `enqueuePromise()`,
    //! and pause the internal timer. The pending promises will be rejected.
    //! @tsdocend
    //! TSDecl: @enumitem DiscardAndReject
    kDiscardAndReject
};
//! TSDecl: @end

//! @tsdocbegin
//! `FrameScheduler` receives a sequence of frames that has timestamp, and will notify the
//! user by emitting events when a frame has expired. It guarantees that the time is accurate
//! enough for multimedia playing.
//!
//! `FrameScheduler` maintains a set of queues, which are identified by media type. A certain
//! queue only accepts frames of a certain media type. Each queue has a limitation of the maximum
//! number of stored frames, which is called "watermark". If the limitation is exceeded, the queue
//! will not accept any frames until the head frame in the queue expires and leaves the queue.
//!
//! `FrameScheduler` is an event emitter of the following events:
//!  @event empty-queue(type: MediaType):
//!    when the queue identified by media type `type` becomes empty (the last frame has been consumed).
//!
//!  @event present(type: MediaType, ptsInSeconds: i32, frame: Frame | null):
//!    when a frame `frame` with timestamp `ptsInSeconds` has expired. Only appears when the
//!    `emitsPresentEvent` option is set, and `frame` is non-null only when the `requiresFrameFeedback`
//!    option is set. If `frame` is non-null, it is a different instance from the frame that the user
//!    has enqueued, but refers to the same underlying buffer, just like a cloned instance.
//!
//! @tsdocend
//! TSDecl: @class @nonconstructible @extends(@import(event) EventEmitterBase) FrameScheduler
class FrameScheduler : public EventEmitterBase
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    using TimePoint = std::chrono::steady_clock::time_point;

    struct PromisifiedEnqueueInfo
    {
        PromisifiedEnqueueInfo(AVFrame *f, v8::Isolate *i, v8::Local<v8::Promise::Resolver> r)
                : frame(f), isolate(i), resolver(i, r) {}
        PromisifiedEnqueueInfo(PromisifiedEnqueueInfo&& rhs) noexcept
            : frame(rhs.frame), isolate(rhs.isolate), resolver(std::move(rhs.resolver)) {}

        PromisifiedEnqueueInfo(const PromisifiedEnqueueInfo&) {
            CHECK("never copy this object");
        }

        AVFrame *frame;
        v8::Isolate *isolate;
        v8::Global<v8::Promise::Resolver> resolver;
    };

    struct Queue
    {
        ~Queue();

        void DiscardQueueFrames();
        std::vector<PromisifiedEnqueueInfo> DiscardPrebufQueueFrames();

        MediaType media_type = MediaType::kUnknown;
        bool emit_present_event = false;
        bool requires_frame_feedback = false;
        bool is_main_sync = false;

        // Accepts frames submitted by `enqueuePromise()`, to store frames
        // before they are finally pushed into `queue`. This queue will be used
        // only by main thread, so mutex is not needed.
        std::queue<PromisifiedEnqueueInfo> prebuf_queue;

        std::mutex lock;
        std::queue<AVFrame*> queue;
        int32_t watermark = 0;
        AVRational timebase = {0, 1};
        std::function<bool(Queue *self, const AVFrame*)> verifier;

        // Statistics
        uint32_t dropped_frames = 0;

        // Extra fields that may be used by `verifier` function
        AVSampleFormat sample_format = AV_SAMPLE_FMT_NONE;
        int32_t sample_rate = 0;
        AVChannelLayout sample_ch_layout = {};
    };
    struct ThreadContext;

    FrameScheduler(uv_loop_t *event_loop,
                   std::map<MediaType, Queue> queues,
                   std::shared_ptr<utau::AudioSinkStream> asink_stream);
    ~FrameScheduler() override;

    //! @tsdocbegin
    //! Dispatch a frame to a certain queue according to `mediaType`.
    //! It clones the frame - `frame` is dispose-safe after calling the method.
    //!
    //! Note that the timestamp of frames served in the same queue MUST be monotonically increasing.
    //! If a frame has an older (smaller) timestamp than the previous frame, it will be dropped without
    //! any notification. Specifically, for the scheduler, the concept of "now" always follows the
    //! latest expired frame's timestamp, any frames older than "now" will be dropped.
    //! @tsdocend
    //! TSDecl: @method enqueue(mediaType: MediaType, frame: Frame): FrameSchedulerStatus
    ffi::Ret<int32_t> enqueue(ffi::Enum<MediaType> media_type, ffi::Class<Frame> frame);

    //! @tsdocbegin
    //! Like `enqueue()`, but never returns `FrameSchedulerStatus.Full`.
    //! Instead, if the queue is not full, the promise will be fulfilled immediately;
    //! otherwise, if the queue is full, the promise will be fulfilled when the enqueue
    //! has free space and the frame has been enqueued.
    //! @tsdocend
    //! TSDecl: @method enqueuePromise(mediaType: MediaType, frame: Frame): @promise(void)
    ffi::RetLocal<v8::Value> enqueuePromise(ffi::Enum<MediaType> media_type, ffi::Class<Frame> frame);

    //! @tsdocbegin
    //! Enters the pause state, in which the internal timer is stopped.
    //! `policy` controls how the frames remaining in the queue are handled.
    //! @tsdocend
    //! TSDecl: @method pause(policy: FrameSchedulerPausePolicy): void
    ffi::Ret<void> pause(ffi::Enum<FrameSchedulerPausePolicy> policy);

    //! @tsdocbegin
    //! Resume from the pause state.
    //! @tsdocend
    //! TSDecl: @method resume(): void
    ffi::Ret<void> resume();

    //! TSDecl: @method dispose(): void
    ffi::Ret<void> dispose();

private:
    // Entrypoint of the scheduler thread
    void SchedulerThread();
    void DispatchNextFrame();
    void SubmitMainThreadTask(std::function<void()> task);
    void EnterPauseState(FrameSchedulerPausePolicy policy);
    void ResumeFromPauseState();

    // Called on the main thread when a notification comes from the scheduler thread
    void OnMainThreadNotify();

    struct ControlCmd
    {
        enum class Op { kTerminate, kPause, kResume };
        Op op;
        int64_t args[4];
    };
    void EnqueueControlCmd(const ControlCmd& cmd);

    std::unique_ptr<ThreadContext> thread_ctx_;
    std::thread thread_;
    std::map<MediaType, Queue> queues_;
    std::shared_ptr<utau::AudioSinkStream> asink_stream_;

    uv::AsyncHandle main_thread_notifier_;
    std::queue<std::function<void()>> main_thread_task_queue_;
    std::mutex main_thread_task_lock_;

    EventEmitFuncT emit_empty_queue_;
    EventEmitFuncT emit_present_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
#endif //COCOA_GALLIUM_BINDINGS_MULTIMEDIA_FRAMESCHEDULER_H
