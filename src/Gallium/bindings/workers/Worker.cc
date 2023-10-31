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

#include "fmt/format.h"

#include "Core/Journal.h"
#include "Core/Exception.h"

#include "Gallium/Platform.h"
#include "Gallium/RuntimeBase.h"
#include "Gallium/binder/Class.h"
#include "Gallium/bindings/workers/Exports.h"
#include "Gallium/bindings/workers/WorkerRuntime.h"
#include "Gallium/bindings/workers/MessagePort.h"
GALLIUM_BINDINGS_WORKERS_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.bindings.workers.Worker)

namespace {

struct WorkerParameters
{
    WorkerParameters(std::shared_ptr<Platform> platform_, std::string url_,
                     std::shared_ptr<MessagePort> port_)
        : platform(std::move(platform_))
        , url(std::move(url_))
        , message_port(std::move(port_))
        , is_running(std::make_shared<std::atomic_bool>(true)) {
        uv_sem_init(&ready_semaphore, 0);
    }

    ~WorkerParameters() {
        uv_sem_destroy(&ready_semaphore);
    }

    void Post() {
        uv_sem_post(&ready_semaphore);
    }

    void PostError(std::string error) {
        maybe_error = std::move(error);
        uv_sem_post(&ready_semaphore);
    }

    void WaitForPost() {
        uv_sem_wait(&ready_semaphore);
    }

    std::shared_ptr<Platform> platform;
    std::string url;
    uv_sem_t ready_semaphore{};
    std::optional<std::string> maybe_error;
    std::shared_ptr<MessagePort> message_port;
    std::shared_ptr<std::atomic_bool> is_running;
};

void *worker_entrypoint(void *arg)
{
    auto *params = reinterpret_cast<WorkerParameters*>(arg);
    CHECK(params);

    pthread_setname_np(pthread_self(), "JSWorker");

    EventLoop::New();
    EventLoop *event_loop = EventLoop::GetCurrent();

    std::shared_ptr<MessagePort> message_port = params->message_port;
    message_port->AttachToEventLoop(event_loop->handle());

    WorkerRuntime runtime(pthread_self(),
                          event_loop->handle(),
                          params->platform,
                          message_port);

    runtime.Initialize();

    std::string eval_url = params->url;
    ScopeExitAutoInvoker on_exit([flag = params->is_running] {
        flag->store(false);
    });

    // After `Post()` is called, the `params` will become a dangling pointer
    // sooner, so we set it `nullptr` to emphasize that it should not be used
    // anymore.
    params->Post();

    // Evaluate the specified module URL
    bool eval_status;
    {
        v8::Isolate::Scope isolate_scope(runtime.GetIsolate());
        v8::HandleScope handle_scope(runtime.GetIsolate());
        v8::Context::Scope context_scope(runtime.GetContext());

        v8::Local<v8::Value> eval_ret;
        eval_status = runtime.EvaluateModule(eval_url).ToLocal(&eval_ret);
    }

    if (!eval_status)
        QLOG(LOG_ERROR, "Failed to evaluate module `{}`", eval_url);

    runtime.SpinRun();
    runtime.Dispose();

    message_port->DetachFromEventLoop();
    EventLoop::Delete();
    return nullptr;
}

} // namespace anonymous

v8::Local<v8::Value> WorkerWrap::MakeFromURL(const std::string &url)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    RuntimeBase *current_runtime = RuntimeBase::FromIsolate(isolate);
    CHECK(current_runtime);

    auto message_ports = MessagePort::MakeConnectedPair(nullptr);
    message_ports.first->AttachToEventLoop(current_runtime->GetEventLoop());

    WorkerParameters params(current_runtime->GetPlatform(),
                            url,
                            std::move(message_ports.second));

    pthread_t thread;
    int ret = pthread_create(&thread, nullptr, worker_entrypoint, &params);
    if (ret < 0)
        g_throw(Error, fmt::format("Failed to create thread: {}", strerror(ret)));

    params.WaitForPost();
    if (params.maybe_error)
        g_throw(Error, fmt::format("{}", *params.maybe_error));

    // Following callbacks are designed to make sure the worker threads
    // have exited before the parent thread exiting.

    using CbType = RuntimeBase::ExternalCallbackType;
    using AfterCallBehaviour = RuntimeBase::ExternalCallbackAfterCall;

    uint64_t spin_exit_cb_id = current_runtime->AddExternalCallback(CbType::kBeforeSpinRunExit, [thread] {
        // When parent thread is going to exit, the worker thread is still running.
        // Just wait for it, we can do nothing, and it is hard to know what the worker
        // thread is doing (maybe it is executing some tasks and will exit later, or maybe
        // it gets into trouble).
        pthread_join(thread, nullptr);
        return AfterCallBehaviour::kRemove;
    });

    std::shared_ptr<std::atomic_bool> is_running_flag = params.is_running;
    current_runtime->AddExternalCallback(CbType::kAfterTasksCheckpoint,
                                         [is_running_flag, thread, spin_exit_cb_id, current_runtime] {
        // If the worker thread is still running, we keep this callback
        // so that we can check it again at the next checkpoint.
        if (is_running_flag->load())
            return AfterCallBehaviour::kOnceMore;

        // If the worker thread has stopped, wait for its termination.
        pthread_join(thread, nullptr);

        // Remove the callback, as the thread is terminated.
        current_runtime->RemoveExternalCallback(CbType::kBeforeSpinRunExit, spin_exit_cb_id);
        return AfterCallBehaviour::kRemove;
    });

    return binder::NewObject<WorkerWrap>(isolate, std::move(message_ports.first));
}

WorkerWrap::WorkerWrap(std::shared_ptr<MessagePort> port)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    message_port_.Reset(
        isolate,
        binder::NewObject<MessagePortWrap>(isolate, std::move(port))
    );
}

WorkerWrap::~WorkerWrap() = default;

GALLIUM_BINDINGS_WORKERS_NS_END
