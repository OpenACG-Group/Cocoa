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

#include <cstdio>

#include <pipewire/pipewire.h>

#include "Core/Journal.h"
#include "Core/Exception.h"
#include "Core/Errors.h"
#include "Utau/PipewireAudioServiceProvider.h"
#include "Utau/PipewireAudioPlaybackStream.h"
UTAU_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Utau.PipewireAudioServiceProvider)

std::shared_ptr<AudioServiceProvider> AudioServiceProvider::MakePipewire()
{
    // Initialize & setup log interface for pipewire client
    int fake_argc = 1;
    char *fake_argv[] = { program_invocation_name, nullptr };
    pw_init(&fake_argc, reinterpret_cast<char***>(&fake_argv));

    // Will be cancelled if no error occurred.
    ScopeExitAutoInvoker scopeExit([]() {
        pw_deinit();
    });

    // Connect to pipewire daemon
    pw_thread_loop *thread_loop = pw_thread_loop_new(COCOA_FREEDESKTOP_APPID, nullptr);
    if (thread_loop == nullptr)
    {
        QLOG(LOG_ERROR, "Failed to create pipewire thread loop");
        return nullptr;
    }

    // Finally, lock and start the pipewire loop
    pw_thread_loop_lock(thread_loop);
    pw_thread_loop_start(thread_loop);

    scopeExit.cancel();
    return std::make_shared<PipewireAudioServiceProvider>(thread_loop);
}

PipewireAudioServiceProvider::PipewireAudioServiceProvider(pw_thread_loop *loop)
    : thread_loop_(loop)
{
}

PipewireAudioServiceProvider::~PipewireAudioServiceProvider()
{
    // Force the running threaded loop to stop and deinitialize pipewire
    pw_thread_loop_unlock(thread_loop_);
    pw_thread_loop_stop(thread_loop_);
    pw_thread_loop_destroy(thread_loop_);
    pw_deinit();
}

std::shared_ptr<AudioPlaybackStream>
PipewireAudioServiceProvider::OnCreatePlaybackStream(const std::string& name,
                                                     MediaRole role,
                                                     int channels,
                                                     SampleFormat sample_format,
                                                     int sample_rate)
{
    auto self = std::dynamic_pointer_cast<PipewireAudioServiceProvider>(shared_from_this());
    CHECK(self);

    return PipewireAudioPlaybackStream::Make(self, name, role, channels,
                                             sample_format, sample_rate);
}

UTAU_NAMESPACE_END
