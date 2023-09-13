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

#include <unistd.h>

#include "Core/Journal.h"
#include "Core/EventLoop.h"
#include "Glamor/PresentThread.h"
#include "Glamor/PresentRemoteCallMessage.h"
#include "Glamor/PresentSignalMessage.h"
#include "Glamor/PresentRemoteHandle.h"
#include "Glamor/GraphicsResourcesTrackable.h"
#include "Glamor/MaybeGpuObject.h"
#include "Glamor/Display.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.PresentThread)

PresentThread::LocalContext::LocalContext(
        uv_loop_t *event_loop,
        std::shared_ptr<Queue> main_thread_queue,
        std::shared_ptr<RemoteDestroyablesCollector> collector)
    : event_loop_(event_loop)
    , main_thread_queue_(std::move(main_thread_queue))
    , idle_handle_(event_loop)
    , remote_destroyables_collector_(std::move(collector))
{
}

void PresentThread::LocalContext::AddActiveDisplay(std::shared_ptr<Display> display)
{
    auto itr = std::find(active_displays_.begin(),
                         active_displays_.end(), display);
    if (itr != active_displays_.end())
        return;
    active_displays_.emplace_back(std::move(display));
}

void PresentThread::LocalContext::RemoveActiveDisplay(const std::shared_ptr<Display>& display)
{
    active_displays_.remove(display);
}

std::string PresentThread::LocalContext::TraceResourcesJSON()
{
    GraphicsResourcesTrackable::Tracer tracer;
    uint32_t idx = 0;
    for (const std::shared_ptr<Display>& d : active_displays_)
    {
        tracer.TraceRootObject(fmt::format("Display#{}", idx), d.get());
        idx++;
    }
    tracer.TraceRootObject("RemoteDestroyablesCollector",
                           remote_destroyables_collector_.get());
    return tracer.ToJsonString();
}

void
PresentThread::LocalContext::EnqueueSignal(const std::shared_ptr<PresentRemoteHandle>& emitter,
                                           PresentRemoteHandle::SignalCode signal_code,
                                           PresentSignal signal_info,
                                           bool has_local_listeners)
{
    auto shared_signal_info = std::make_shared<PresentSignal>(std::move(signal_info));
    auto message = std::make_unique<PresentSignalMessage>(
            shared_signal_info, emitter, signal_code);
    main_thread_queue_->Enqueue(std::move(message), [](const Queue::Message& msg) {
        msg->MarkProfileMilestone(PresentMessageMilestone::kClientEmitted);
    });

    // Schedule local signals. If the signal is being listened by
    // listeners on this thread, they should be called in the next
    // event loop iteration.
    if (!has_local_listeners)
        return;

    // Empty queue means that the idle handle has not been started yet.
    if (local_signal_queue_.empty())
    {
        idle_handle_.Start([this] {
            std::vector<PresentSignalMessage> messages;
            while (!local_signal_queue_.empty())
            {
                messages.emplace_back(local_signal_queue_.front());
                local_signal_queue_.pop();
            }
            for (const PresentSignalMessage& message : messages)
            {
                CHECK(message.GetEmitter());
                message.GetEmitter()->DoEmitSignal(
                        message.GetSignalCode(), *message.GetSignalInfo(), true);
            }

            // Only run this callback once
            idle_handle_.Stop();
        });
    }
    local_signal_queue_.emplace(shared_signal_info, emitter, signal_code);
}

namespace {

struct ThreadArgs
{
    explicit ThreadArgs(std::shared_ptr<PresentThread::Queue> _main_thread_queue)
        : main_thread_queue(std::move(_main_thread_queue))
        , collector(std::make_shared<RemoteDestroyablesCollector>())
        , thread_ready_semaphore{}
    {
        uv_sem_init(&thread_ready_semaphore, 0);
    }

    void Post(std::weak_ptr<PresentThread::Queue> queue) {
        present_thread_queue = std::move(queue);
        uv_sem_post(&thread_ready_semaphore);
    }

    std::weak_ptr<PresentThread::Queue> WaitForPost() {
        uv_sem_wait(&thread_ready_semaphore);
        return std::move(present_thread_queue);
    }

    // Filled by main thread
    std::shared_ptr<PresentThread::Queue> main_thread_queue;
    std::shared_ptr<RemoteDestroyablesCollector> collector;
    uv_sem_t thread_ready_semaphore;

    // Filled by present thread
    std::weak_ptr<PresentThread::Queue> present_thread_queue;
};

void *present_thread_entrypoint(void *args)
{
    pthread_setname_np(pthread_self(), "PresentThread");
    auto *thread_args = static_cast<ThreadArgs*>(args);

    QLOG(LOG_INFO, "Present thread has been started, tid={}", gettid());

    // Create a thread-local global event loop.
    EventLoop::New();
    EventLoop *event_loop = EventLoop::GetCurrent();

    std::shared_ptr<PresentThread::Queue> main_thread_queue =
            thread_args->main_thread_queue;

    auto remote_collector = thread_args->collector;

    // Create present thread message queue.
    auto present_thread_queue = std::make_shared<PresentThread::Queue>(
            event_loop->handle(), PresentThread::Queue::HandlerF{});

    // `Message` is `std::unique_ptr<PresentMessage>`
    using Message = PresentThread::Queue::Message;
    using Queue = PresentThread::Queue;
    present_thread_queue->SetMessageHandler([main_thread_queue, &present_thread_queue]
                                            (Message message, Queue*) {

        if (message == nullptr)
        {
            // A null message requests the present thread to exit.
            // Destroying the event queue removes its corresponding
            // handle from event loop, then the event loop will exit
            // if there are no any other pending handles.
            present_thread_queue.reset();
            return;
        }

        if (!message->IsRemoteCall())
        {
            QLOG(LOG_ERROR, "Coming message is not a remote call");
            return;
        }
        // NOLINTNEXTLINE
        auto *remote_call = static_cast<PresentRemoteCallMessage*>(message.get());
        auto receiver = remote_call->GetReceiver();

        remote_call->MarkProfileMilestone(PresentMessageMilestone::kClientReceived);
        receiver->DoRemoteCall(remote_call->GetClientCallInfo());
        remote_call->MarkProfileMilestone(PresentMessageMilestone::kClientProcessed);
        main_thread_queue->Enqueue(std::move(message), [](const Queue::Message& msg) {
            msg->MarkProfileMilestone(PresentMessageMilestone::kClientFeedback);
        });
    });

    // Now we can notify the main thread, which is waiting for
    // the present thread to prepare, that we have initiated all
    // the thread-local contexts, and will enter the event loop.
    thread_args->Post(present_thread_queue);

    // Now the two pointers become dangling pointers, so they should
    // be reset to `nullptr` to avoid illegal dereference.
    thread_args = nullptr;
    args = nullptr;

    PresentThread::LocalContext::New(event_loop->handle(),
                                     main_thread_queue,
                                     remote_collector);

    event_loop->run();
    QLOG(LOG_INFO, "Present thread has exited");

    // Send a null message to indicate that the present
    // thread has exited.
    main_thread_queue->Enqueue(nullptr, {});

    PresentThread::LocalContext::Delete();
    EventLoop::Delete();
    return nullptr;
}

} // namespace anonymous

std::unique_ptr<PresentThread> PresentThread::Start(uv_loop_t *loop)
{
    auto main_thread_queue = std::make_shared<Queue>(loop, Queue::HandlerF());
    ThreadArgs thread_args(main_thread_queue);

    pthread_t thread;
    int err = pthread_create(
            &thread, nullptr, present_thread_entrypoint, &thread_args);
    if (err < 0)
    {
        QLOG(LOG_ERROR, "Failed to create present thread: {}", strerror(err));
        return nullptr;
    }

    // Wait until the thread has created its own event loop
    // and message queue. The message queue will be used to
    // send messages to the thread.
    std::weak_ptr<Queue> present_thread_queue = thread_args.WaitForPost();

    return std::make_unique<PresentThread>(std::move(present_thread_queue),
                                           std::move(main_thread_queue),
                                           thread,
                                           std::move(thread_args.collector));
}

PresentThread::PresentThread(std::weak_ptr<Queue> present_thread_queue,
                             std::shared_ptr<Queue> main_thread_queue,
                             pthread_t present_thread,
                             std::shared_ptr<RemoteDestroyablesCollector> collector)
    : present_thread_queue_(std::move(present_thread_queue))
    , main_thread_queue_(std::move(main_thread_queue))
    , present_thread_(present_thread)
    , thread_has_exited_(false)
    , task_runner_(std::make_shared<PresentThreadTaskRunner>())
    , remote_destroyables_collector_(std::move(collector))
{
    main_thread_queue_->SetMessageHandler([this](Queue::Message message, Queue*) {
        OnMainThreadMessage(std::move(message));
    });
}

void PresentThread::EnqueueRemoteCall(std::shared_ptr<PresentRemoteHandle> receiver,
                                      PresentRemoteCall call_info,
                                      PresentRemoteCallResultCallback result_callback)
{
    std::shared_ptr<Queue> queue = present_thread_queue_.lock();
    if (!queue)
    {
        QLOG(LOG_ERROR, "Failed to enqueue remote call: queue is not available");
        return;
    }
    auto message = std::make_unique<PresentRemoteCallMessage>(
            std::move(receiver), std::move(call_info), std::move(result_callback));
    message->MarkProfileMilestone(PresentMessageMilestone::kHostConstruction);
    queue->Enqueue(std::move(message), [](const Queue::Message& msg) {
        msg->MarkProfileMilestone(PresentMessageMilestone::kHostEnqueued);
    });
}

void PresentThread::OnMainThreadMessage(Queue::Message message)
{
    if (thread_has_exited_)
        return;

    if (message == nullptr)
    {
        // A null message means that the present thread has exited.
        pthread_join(present_thread_, nullptr);
        thread_has_exited_ = true;
        // Allow the main thread event loop to exit.
        main_thread_queue_->SetNonBlocking(true);
        return;
    }

    message->MarkProfileMilestone(PresentMessageMilestone::kHostReceived);
    if (message->IsRemoteCall())
    {
        // NOLINTNEXTLINE
        auto *remote_call = static_cast<PresentRemoteCallMessage*>(message.get());
        PresentRemoteCallReturn call_return(remote_call);
        remote_call->GetHostCallback()(call_return);
    }
    else if (message->IsSignalEmit())
    {
        // NOLINTNEXTLINE
        auto *signal = static_cast<PresentSignalMessage*>(message.get());
        signal->GetEmitter()->DoEmitSignal(
                signal->GetSignalCode(), *signal->GetSignalInfo(), false);
    }

    // TODO(sora): collect messaging samples for profiling
}

void PresentThread::SubmitTaskNoRet(std::function<void()> task_func,
                                    std::function<void()> result_callback,
                                    std::function<void(std::string)> caught_callback)
{
    PresentThreadTaskRunner::Task task = [func = std::move(task_func)]() -> std::any {
        func(); return {};
    };
    task_runner_->Invoke(GLOP_TASKRUNNER_RUN, nullptr,
        [func = std::move(result_callback), caught = std::move(caught_callback)]
        (PresentRemoteCallReturn &ret) {
            if (ret.GetReturnStatus() == PresentRemoteCall::Status::kCaught)
                caught(ret.GetCaughtException());
            else if (func)
                func();
        },
        task
    );
}

void PresentThread::Dispose()
{
    std::shared_ptr<Queue> queue = present_thread_queue_.lock();
    if (!queue)
        return;

    // Collect all the remote destroyable objects.
    // If there actually are collectable living objects, they will be
    // collected. Registered callbacks will be called immediately, and
    // several asynchronous tasks, which perform the destruction of the
    // collected objects, will be submitted to the present thread.
    remote_destroyables_collector_->Collect();

    // This should be the last message in present thread queue.
    // The thread will prepare to exit once it received this message.
    queue->Enqueue(nullptr);
}

GLAMOR_NAMESPACE_END
