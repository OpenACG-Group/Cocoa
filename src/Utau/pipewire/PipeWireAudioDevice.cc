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

#include "Core/Errors.h"
#include "Core/Exception.h"
#include "Core/Journal.h"
#include "Utau/pipewire/PipeWireAudioDevice.h"
#include "Utau/pipewire/PipeWireAudioSinkStream.h"
UTAU_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Utau.pipewire.PipeWireAudioDevice)

std::shared_ptr<AudioDevice> AudioDevice::MakePipeWire(uv_loop_t *loop)
{
    // Initialize & setup log interface for PipeWire client
    int pseudo_argc = 1;
    char *pseudo_argv[] = { program_invocation_name, nullptr };
    pw_init(&pseudo_argc, reinterpret_cast<char***>(&pseudo_argv));

    // Will be cancelled if no error occurred
    ScopeExitAutoInvoker scope_exit([] { pw_deinit(); });

    auto dev = std::make_shared<PipeWireAudioDevice>();

    dev->main_thread_loop_ = loop;

    // Connect to PipeWire daemon
    dev->pw_loop_ = pw_thread_loop_new("PipeWire", nullptr);
    if (!dev->pw_loop_)
    {
        QLOG(LOG_ERROR, "Failed to create PipeWire mainloop");
        return nullptr;
    }

    pw_thread_loop_lock(dev->pw_loop_);

    if (pw_thread_loop_start(dev->pw_loop_) < 0)
    {
        QLOG(LOG_ERROR, "Failed to start PipeWire thread loop");
        pw_thread_loop_unlock(dev->pw_loop_);
        return nullptr;
    }

    pw_context *context = pw_context_new(pw_thread_loop_get_loop(dev->pw_loop_), nullptr, 0);
    if (!context)
    {
        QLOG(LOG_ERROR, "Failed to create PipeWire context");
        pw_thread_loop_unlock(dev->pw_loop_);
        return nullptr;
    }

    dev->pw_core_ = pw_context_connect(context, nullptr, 0);
    if (!dev->pw_core_)
    {
        QLOG(LOG_ERROR, "Failed to connect to PipeWire context");
        pw_context_destroy(context);
        pw_thread_loop_unlock(dev->pw_loop_);
        return nullptr;
    }

    // TODO(sora): add core listener

    pw_thread_loop_unlock(dev->pw_loop_);

    dev->uv_async_ = static_cast<uv_async_t*>(malloc(sizeof(uv_async_t)));
    CHECK(dev->uv_async_ && "Failed to allocate memory");

    uv_async_init(loop, dev->uv_async_, PipeWireAudioDevice::AsyncHandler);
    dev->uv_async_->data = dev.get();

    scope_exit.cancel();
    return dev;
}

PipeWireAudioDevice::PipeWireAudioDevice()
    : AudioDevice(kPipeWire_Backend)
    , main_thread_loop_(nullptr)
    , uv_async_(nullptr)
    , pw_loop_(nullptr)
    , pw_core_(nullptr)
{
}

PipeWireAudioDevice::~PipeWireAudioDevice()
{
    pw_thread_loop_stop(pw_loop_);
    pw_context_destroy(pw_core_get_context(pw_core_));
    pw_thread_loop_destroy(pw_loop_);

    pw_deinit();

    uv_close((uv_handle_t*)uv_async_, [](uv_handle_t *ptr) {
        free(ptr);
    });

    QLOG(LOG_INFO, "PipeWire audio device {} was disposed", fmt::ptr(this));
}

void PipeWireAudioDevice::LockThreadLoop()
{
    pw_thread_loop_lock(pw_loop_);
}

void PipeWireAudioDevice::UnlockThreadLoop()
{
    pw_thread_loop_unlock(pw_loop_);
}

std::unique_ptr<AudioSinkStream>
PipeWireAudioDevice::CreateSinkStream(const std::string& name)
{
    return PipeWireAudioSinkStream::MakeFromDevice(shared_from_this(), name);
}

void PipeWireAudioDevice::InvokeFromMainThread(const std::function<void()>& proc)
{
    std::scoped_lock<std::mutex> lock(calls_queue_lock_);
    main_thread_calls_queue_.emplace(proc);
    uv_async_send(uv_async_);
}

void PipeWireAudioDevice::AsyncHandler(uv_async_t *handle)
{
    auto *self = reinterpret_cast<PipeWireAudioDevice*>(handle->data);
    CHECK(self);

    self->calls_queue_lock_.lock();
    while (!self->main_thread_calls_queue_.empty())
    {
        std::function<void()> proc = std::move(self->main_thread_calls_queue_.front());
        self->main_thread_calls_queue_.pop();
        self->calls_queue_lock_.unlock();

        proc();

        self->calls_queue_lock_.lock();
    }
    self->calls_queue_lock_.unlock();
}

UTAU_NAMESPACE_END
