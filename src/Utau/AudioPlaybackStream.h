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

#ifndef COCOA_UTAU_AUDIOPLAYBACKSTREAM_H
#define COCOA_UTAU_AUDIOPLAYBACKSTREAM_H

#include <future>
#include <queue>
#include <mutex>

#include "Utau/Utau.h"
UTAU_NAMESPACE_BEGIN

class AudioBuffer;
class AudioServiceProvider;

enum class BufferFinalState
{
    // Buffer was dequeued as all its contents had been consumed.
    kConsumed,

    // Buffer was interrupted while playing.
    kInterrupted,

    // Buffer was not played
    kRejected
};

class AudioPlaybackStream
{
public:
    struct QueuedBuffer
    {
        explicit QueuedBuffer(std::shared_ptr<AudioBuffer> _buffer)
            : buffer(std::move(_buffer)) {}
        std::promise<BufferFinalState> final_state;
        std::shared_ptr<AudioBuffer> buffer;
    };

    struct StreamInfo
    {
        int channels;
        int sample_rate;
        SampleFormat sample_format;
    };

    AudioPlaybackStream(std::weak_ptr<AudioServiceProvider> provider,
                        std::string name,
                        const StreamInfo& info);
    virtual ~AudioPlaybackStream() = default;

    g_nodiscard g_inline const std::string& GetStreamName() const {
        return stream_name_;
    }

    g_nodiscard g_inline std::shared_ptr<AudioServiceProvider> GetAudioServiceProvider() const {
        return audio_service_provider_.lock();
    }

    g_nodiscard g_inline const StreamInfo& GetStreamInfo() const {
        return stream_info_;
    }

    void Dispose();
    void ClearQueue();
    std::future<BufferFinalState> EnqueueBuffer(const std::shared_ptr<AudioBuffer>& buffer);

protected:
    QueuedBuffer& GetBufferFromQueue();
    void PopHeadQueueBuffer(BufferFinalState final_state);

    virtual void OnBufferEnqueued() = 0;
    virtual void OnInterruptCurrentBuffer() = 0;
    virtual void OnDispose() = 0;

private:
    std::weak_ptr<AudioServiceProvider> audio_service_provider_;
    const std::string stream_name_;
    const StreamInfo  stream_info_;
    std::queue<QueuedBuffer> buffers_queue_;
    std::mutex queue_lock_;
    bool is_disposed_;
};

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_AUDIOPLAYBACKSTREAM_H
