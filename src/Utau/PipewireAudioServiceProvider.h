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

#ifndef COCOA_UTAU_PIPEWIREAUDIOSERVICEPROVIDER_H
#define COCOA_UTAU_PIPEWIREAUDIOSERVICEPROVIDER_H

#include <pipewire/pipewire.h>

#include "Utau/Utau.h"
#include "Utau/AudioServiceProvider.h"
UTAU_NAMESPACE_BEGIN

class PipewireAudioServiceProvider : public AudioServiceProvider
{
public:
    explicit PipewireAudioServiceProvider(pw_thread_loop *loop);
    ~PipewireAudioServiceProvider() override;

    g_nodiscard g_inline pw_thread_loop *GetPipewireThreadLoop() {
        return thread_loop_;
    }

    g_nodiscard g_inline pw_loop *GetPipewireLoop() {
        return pw_thread_loop_get_loop(thread_loop_);
    }

    std::shared_ptr<AudioPlaybackStream> OnCreatePlaybackStream(const std::string &name,
                                                                MediaRole role,
                                                                int channels,
                                                                SampleFormat sample_format,
                                                                int sample_rate) override;

private:
    pw_thread_loop      *thread_loop_;
};

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_PIPEWIREAUDIOSERVICEPROVIDER_H
