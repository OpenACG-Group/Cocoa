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
#include "Core/TraceEvent.h"

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
    main_thread_queue_->Enqueue(std::move(message));

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


struct PresentThread::ThreadPriv
{
    explicit ThreadPriv(std::shared_ptr<PresentThread::Queue> _main_thread_queue)
        : main_thread_queue(std::move(_main_thread_queue))
        , collector(std::make_shared<RemoteDestroyablesCollector>())
        , thread_ready_semaphore{}
    {
        uv_sem_init(&thread_ready_semaphore, 0);
    }

    void Post() {
        uv_sem_post(&thread_ready_semaphore);
    }

    void WaitForPost() {
        uv_sem_wait(&thread_ready_semaphore);
    }

    // Filled by main thread
    std::shared_ptr<PresentThread::Queue> main_thread_queue;
    std::shared_ptr<RemoteDestroyablesCollector> collector;
    uv_sem_t thread_ready_semaphore;

    // Filled by present thread
    std::unique_ptr<PresentThread::Queue> present_thread_queue;
};

void present_thread_entrypoint(const std::shared_ptr<PresentThread::ThreadPriv>& thread_args)
{
    pthread_setname_np(pthread_self(), "PresentThread");

    QLOG(LOG_INFO, "Present thread has been started, tid={}", gettid());

    // Create a thread-local global event loop.
    EventLoop::New();
    EventLoop *event_loop = EventLoop::GetCurrent();

    auto main_thread_queue = thread_args->main_thread_queue;
    auto remote_collector = thread_args->collector;

    // Create present thread message queue.
    thread_args->present_thread_queue = std::make_unique<PresentThread::Queue>(
            event_loop->handle(), PresentThread::Queue::HandlerF{});

    // `Message` is `std::unique_ptr<PresentMessage>`
    using Message = PresentThread::Queue::Message;
    using Queue = PresentThread::Queue;
    thread_args->present_thread_queue->SetMessageHandler(
    [main_thread_queue, thread_args](Message message, Queue*) {

        if (message == nullptr)
        {
            // A null message requests the present thread to exit.
            // The event loop will exit if there are no any other pending handles.
            thread_args->present_thread_queue->SetNonBlocking(true);
            thread_args->present_thread_queue->SetMessageHandler({});
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

        TRACE_EVENT("present.request", "RemoteCallHandler", perfetto::Flow::FromPointer(message.get()));

        receiver->DoRemoteCall(remote_call->GetClientCallInfo());
        main_thread_queue->Enqueue(std::move(message));
    });

    // Now we can notify the main thread, which is waiting for
    // the present thread to prepare, that we have initiated all
    // the thread-local contexts, and will enter the event loop.
    thread_args->Post();

    PresentThread::LocalContext::New(
            event_loop->handle(), thread_args->main_thread_queue, thread_args->collector);

    event_loop->run();
    QLOG(LOG_INFO, "Present thread has exited");

    PresentThread::LocalContext::Delete();
    EventLoop::Delete();
}

std::unique_ptr<PresentThread> PresentThread::Start(uv_loop_t *loop)
{
    auto main_thread_queue = std::make_shared<Queue>(loop, Queue::HandlerF());
    auto thread_args = std::make_shared<ThreadPriv>(main_thread_queue);

    std::thread thread(present_thread_entrypoint, thread_args);
    // Wait until the thread has created its own event loop.
    thread_args->WaitForPost();

    return std::make_unique<PresentThread>(std::move(main_thread_queue),
                                           std::move(thread),
                                           std::move(thread_args),
                                           thread_args->collector);
}

PresentThread::PresentThread(std::shared_ptr<Queue> main_thread_queue,
                             std::thread thread,
                             std::shared_ptr<ThreadPriv> thread_priv,
                             std::shared_ptr<RemoteDestroyablesCollector> collector)
    : main_thread_queue_(std::move(main_thread_queue))
    , present_thread_(std::move(thread))
    , thread_priv_(std::move(thread_priv))
    , task_runner_(std::make_shared<PresentThreadTaskRunner>())
    , remote_destroyables_collector_(std::move(collector))
{
    main_thread_queue_->SetMessageHandler([](Queue::Message message, Queue*) {
        if (message->IsRemoteCall())
        {
            TRACE_EVENT("present.request", "RemoteCallResponse",
                        perfetto::TerminatingFlow::FromPointer(message.get()));

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
    });
}

void PresentThread::EnqueueRemoteCall(std::shared_ptr<PresentRemoteHandle> receiver,
                                      PresentRemoteCall call_info,
                                      PresentRemoteCallResultCallback result_callback)
{
    CHECK(thread_priv_->present_thread_queue);

    PresentRemoteCall::OpCode opcode = call_info.GetOpCode();
    auto message = std::make_unique<PresentRemoteCallMessage>(
            receiver, std::move(call_info), std::move(result_callback));

    thread_priv_->present_thread_queue->Enqueue(std::move(message), [&](const Queue::Message& msg) {
        TRACE_EVENT("present.request", nullptr, [&](perfetto::EventContext& ctx) {
            auto type_name = gl::PresentRemoteHandle::GetTypeName(receiver->GetRealType());
            ctx.event()->set_name(fmt::format("request:{}.opcode#{}", type_name, opcode));
        }, perfetto::Flow::FromPointer(msg.get()));
    });
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
    // Collect all the remote destroyable objects.
    // If there actually are collectable living objects, they will be
    // collected. Registered callbacks will be called immediately, and
    // several asynchronous tasks, which perform the destruction of the
    // collected objects, will be submitted to the present thread.
    remote_destroyables_collector_->Collect();

    // This should be the last message in present thread queue.
    // The thread will prepare to exit once it received this message.
    thread_priv_->present_thread_queue->Enqueue(nullptr);

    if (present_thread_.joinable())
        present_thread_.join();

    main_thread_queue_->SetNonBlocking(true);

    thread_priv_ = nullptr;
}

GLAMOR_NAMESPACE_END
