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

#ifndef COCOA_UTAU_PIPEWIRE_PIPEWIREAUDIODEVICE_H
#define COCOA_UTAU_PIPEWIRE_PIPEWIREAUDIODEVICE_H

#include <pipewire/pipewire.h>

#include <queue>
#include <thread>

#include "Utau/Utau.h"
#include "Utau/AudioDevice.h"
UTAU_NAMESPACE_BEGIN

class PipeWireAudioDevice : public AudioDevice,
                            public std::enable_shared_from_this<PipeWireAudioDevice>
{
    friend std::shared_ptr<AudioDevice> AudioDevice::MakePipeWire(uv_loop_t*);

public:
    PipeWireAudioDevice();
    ~PipeWireAudioDevice() override;

    class ScopedThreadLoopLock
    {
    public:
        explicit ScopedThreadLoopLock(PipeWireAudioDevice *device, bool lock = true)
            : dev_(device)
        {
            CHECK(device);
            if (lock)
                dev_->LockThreadLoop();
        }

        ~ScopedThreadLoopLock() {
            dev_->UnlockThreadLoop();
        }

    private:
        PipeWireAudioDevice *dev_;
    };

    std::unique_ptr<AudioSinkStream> CreateSinkStream(const std::string &name) override;

    g_private_api g_nodiscard g_inline pw_thread_loop *GetPipeWireLoop() const {
        return pw_loop_;
    }

    g_private_api void InvokeFromMainThread(const std::function<void()>& proc);

    g_private_api void LockThreadLoop();
    g_private_api void UnlockThreadLoop();

private:
    static void AsyncHandler(uv_async_t *handle);

    uv_loop_t                  *main_thread_loop_;
    uv_async_t                 *uv_async_;
    pw_thread_loop             *pw_loop_;
    pw_core                    *pw_core_;
    std::mutex                  calls_queue_lock_;
    std::queue<std::function<void()>> main_thread_calls_queue_;
};

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_PIPEWIRE_PIPEWIREAUDIODEVICE_H
