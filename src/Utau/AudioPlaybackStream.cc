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
#include "Utau/AudioPlaybackStream.h"

#include <utility>
UTAU_NAMESPACE_BEGIN

AudioPlaybackStream::AudioPlaybackStream(std::weak_ptr<AudioServiceProvider> provider,
                                         std::string name,
                                         const StreamInfo& info)
    : audio_service_provider_(std::move(provider))
    , stream_name_(std::move(name))
    , stream_info_(info)
    , is_disposed_(false)
{
}

std::future<BufferFinalState> AudioPlaybackStream::EnqueueBuffer(const std::shared_ptr<AudioBuffer>& buffer)
{
    QueuedBuffer *queued_buffer;
    {
        std::scoped_lock<std::mutex> lock(queue_lock_);
        buffers_queue_.emplace(buffer);
        queued_buffer = &buffers_queue_.front();
    }

    this->OnBufferEnqueued();
    return queued_buffer->final_state.get_future();
}

void AudioPlaybackStream::ClearQueue()
{
    // This method should interrupt the playing process of current buffer,
    // and pop that buffer out of queue.
    this->OnInterruptCurrentBuffer();

    {
        std::scoped_lock<std::mutex> lock(queue_lock_);
        while (!buffers_queue_.empty())
        {
            // Currently playing buffer (usually the first buffer in queue) has
            // final state `Interrupted`, which was set by `OnInterruptCurrentBuffer`,
            // and other buffers in queue have final state `Rejected`.
            buffers_queue_.front().final_state.set_value(BufferFinalState::kRejected);
            buffers_queue_.pop();
        }
    }
}

AudioPlaybackStream::QueuedBuffer& AudioPlaybackStream::GetBufferFromQueue()
{
    std::scoped_lock<std::mutex> lock(queue_lock_);
    CHECK(!buffers_queue_.empty());
    return buffers_queue_.front();
}

void AudioPlaybackStream::PopHeadQueueBuffer(BufferFinalState final_state)
{
    std::scoped_lock<std::mutex> lock(queue_lock_);
    CHECK(!buffers_queue_.empty());
    buffers_queue_.front().final_state.set_value(final_state);
    buffers_queue_.pop();
}

void AudioPlaybackStream::Dispose()
{
    if (!is_disposed_)
    {
        ClearQueue();
        this->OnDispose();
        is_disposed_ = true;
    }
}

UTAU_NAMESPACE_END
