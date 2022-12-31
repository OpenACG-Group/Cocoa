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

#ifndef COCOA_UTAU_PIPEWIREAUDIOSINK_H
#define COCOA_UTAU_PIPEWIREAUDIOSINK_H

#include <future>
#include <thread>

#include <pipewire/pipewire.h>

#include "Utau/Utau.h"
#include "Utau/AudioSink.h"
UTAU_NAMESPACE_BEGIN

class AudioBuffer;

class PipeWireAudioSink : public AudioSink
{
public:
    static std::unique_ptr<PipeWireAudioSink> Connect(uv_loop_t *loop);

    explicit PipeWireAudioSink(uv_loop_t *loop);
    ~PipeWireAudioSink() override = default;

    g_nodiscard SampleFormat GetRequiredSampleFormat() const override;
    g_nodiscard int GetRequiredSampleRate() const override;
    g_nodiscard AudioChannelMode GetRequiredChannelMode() const override;

    void NotifyNewBufferEnqueuedInWorkerThread();
    void NotifyDisposeInWorkerThread();

    static void on_stream_process(void *userdata);

private:
    static void worker_thread_routine(PipeWireAudioSink *self,
                                      std::promise<bool> *init_result_promise);
    bool initialize_audio_sink();

    void NotifyNewBufferEnqueued() override;
    void NotifyAndWaitWorkerThreadDispose() override;
    bool BufferCheckBeforeEnqueue(const std::shared_ptr<AudioBuffer>& buffer) override;

    std::thread          thread_;
    pw_main_loop        *pw_loop_;
    pw_stream           *pw_stream_;
    std::unique_ptr<uint8_t[]> pod_buffer_;
    BufferWithId         current_buffer_;
    size_t               current_buffer_offset_;
};

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_PIPEWIREAUDIOSINK_H
