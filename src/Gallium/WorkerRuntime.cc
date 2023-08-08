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

#include <optional>
#include <string>

#include "uv.h"
#include "fmt/format.h"

#include "Core/Journal.h"
#include "Gallium/WorkerRuntime.h"
#include "Gallium/Platform.h"
#include "Gallium/Infrastructures.h"
GALLIUM_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.WorkerRuntimeThread)

WorkerRuntimeThread::WorkerRuntimeThread(ParentThreadDelegate *delegate)
    : delegate_(delegate)
    , thread_{}
    , thread_loop_{}
{
}

WorkerRuntimeThread::~WorkerRuntimeThread()
{
    uv_thread_join(&thread_);
}

struct WorkerArgs
{
    std::string eval_url;
    uv_sem_t semaphore{};
    std::shared_ptr<Platform> platform = {};
    WorkerRuntimeThread *thread_self = nullptr;
    std::optional<std::string> error = {};
    ParentThreadDelegate *parent_thread_delegate = nullptr;

    void Post() {
        uv_sem_post(&semaphore);
    }

    void PostError(const std::string& str) {
        error = str;
        uv_sem_post(&semaphore);
    }
};

WorkerRuntimeThread::CreateResult
WorkerRuntimeThread::Create(ParentThreadDelegate *parent_thread_delegate,
                            const std::string &url,
                            std::shared_ptr<Platform> platform,
                            const Options& options)
{
    auto thread = std::make_unique<WorkerRuntimeThread>(parent_thread_delegate);

    WorkerArgs worker_args{
        .eval_url = url,
        .platform = std::move(platform),
        .thread_self = thread.get(),
        .parent_thread_delegate = parent_thread_delegate
    };
    uv_sem_init(&worker_args.semaphore, 0);

    int ret = uv_thread_create(&thread->thread_,
                               WorkerRuntimeThread::WorkerEntrypoint,
                               &worker_args);
    if (ret < 0)
    {
        uv_sem_destroy(&worker_args.semaphore);
        QLOG(LOG_ERROR, "Failed to create JSWorker thread: {}", uv_strerror(ret));
        return {
            .worker = nullptr,
            .error = "Failed to create JSWorker thread"
        };
    }

    // Wait worker thread for getting ready
    uv_sem_wait(&worker_args.semaphore);
    uv_sem_destroy(&worker_args.semaphore);

    if (worker_args.error.has_value())
    {
        QLOG(LOG_INFO, "Worker:%fg<bl,hl>{}%reset error: {}",
             fmt::ptr(thread.get()), *worker_args.error);
        return {
            .worker = nullptr,
            .error = worker_args.error
        };
    }

    QLOG(LOG_INFO, "Worker:%fg<bl,hl>{}%reset reported it got ready to evaluate module",
         fmt::ptr(thread.get()));

    return { .worker = std::move(thread) };
}

void WorkerRuntimeThread::WorkerEntrypoint(void *args)
{
    pthread_setname_np(pthread_self(), "JSWorker");
    auto *worker_args = reinterpret_cast<WorkerArgs*>(args);
    WorkerRuntimeThread *thread_self = worker_args->thread_self;
    ParentThreadDelegate *parent_thread_delegate = worker_args->parent_thread_delegate;

    // Prepare thread context
    uv_loop_t *loop = &thread_self->thread_loop_;
    int ret = uv_loop_init(loop);
    if (ret < 0)
    {
        worker_args->PostError(uv_strerror(ret));
        return;
    }

    // Now we can create runtime
    WorkerRuntime runtime(pthread_self(),
                          loop,
                          std::move(worker_args->platform),
                          worker_args->parent_thread_delegate);

    runtime.Initialize();

    thread_self->message_async_.emplace(loop, [thread_self, &runtime] {
        // This callback will be fired when messages come
        // from main thread.
        std::vector<std::unique_ptr<WorkerMessage>> messages;
        thread_self->message_queue_lock_.lock();
        while (!thread_self->message_queue_.empty())
        {
            messages.emplace_back(std::move(thread_self->message_queue_.front()));
            thread_self->message_queue_.pop();
        }
        thread_self->message_queue_lock_.unlock();

        for (std::unique_ptr<WorkerMessage>& msg : messages)
        {
            if (msg->type == WorkerMessage::kTerminate_Type)
            {
                QLOG(LOG_INFO, "Termination message delivered from main thread");
                thread_self->message_async_.reset();
                // TODO(sora): Should we close the loop and force it exit?
                return;
            }
            runtime.ReceiveHostMessage(*msg);
        }
    });

    std::string url = worker_args->eval_url;
    // Since then, pointer `worker_args` becomes a dangling pointer,
    // and should not be used anymore.
    worker_args->Post();
    worker_args = nullptr;
    (void) worker_args;

    {
        v8::Isolate::Scope isolate_scope(runtime.GetIsolate());
        v8::HandleScope handle_scope(runtime.GetIsolate());
        v8::Context::Scope context_scope(runtime.GetContext());

        runtime.EvaluateModule(url);
        runtime.SpinRun();
    }

    runtime.Dispose();
    uv_loop_close(loop);

    parent_thread_delegate->PostMessageToMainThread(
            WorkerMessage::Terminate(thread_self));
}

void WorkerRuntimeThread::PostMessageToWorker(std::unique_ptr<WorkerMessage> message)
{
    CHECK(message_async_.has_value());
    std::scoped_lock<std::mutex> lock(message_queue_lock_);
    message_queue_.emplace(std::move(message));
    message_async_->Send();
}

WorkerRuntime::WorkerRuntime(uint32_t thread_id,
                             uv_loop_t *event_loop,
                             std::shared_ptr<Platform> platform,
                             ParentThreadDelegate *parent_thread_delegate)
    : RuntimeBase(event_loop, std::move(platform), fmt::format("Runtime@Worker#{}", thread_id))
    , parent_thread_delegate_(parent_thread_delegate)
{
}

void WorkerRuntime::OnInitialize(v8::Isolate *isolate, v8::Local<v8::Context> context)
{
    infra::InstallOnGlobalContext(isolate, context, true);
}

void WorkerRuntime::ReceiveHostMessage(WorkerMessage& message)
{
    // Termination message should not be handled by `WorkerRuntime`.
    CHECK(message.type == WorkerMessage::kTransfer_Type);

    // TODO(sora): implement this.
}

GALLIUM_NS_END
