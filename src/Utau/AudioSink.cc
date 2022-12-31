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

#include <algorithm>
#include <functional>

#include "Core/Errors.h"
#include "Utau/ThreadedAudioSink.h"
UTAU_NAMESPACE_BEGIN

ThreadedAudioSink::BufferWithId
ThreadedAudioSink::BufferWithId::Generate(std::shared_ptr<SoundBuffer> buffer)
{
    static int32_t g_id_counter = 0;

    CHECK(buffer);

    return {std::move(buffer), ++g_id_counter};
}

ThreadedAudioSink::ThreadedAudioSink(uv_loop_t *main_loop, BackDevice device)
    : back_device_(device)
    , disposed_(false)
    , buffer_event_notifier_(nullptr)
{
    CHECK(main_loop);

    buffer_event_notifier_ = reinterpret_cast<uv_async_t*>(
            malloc(sizeof(uv_async_t)));
    CHECK(buffer_event_notifier_ && "Failed to allocate memory");

    uv_async_init(main_loop, buffer_event_notifier_, &on_buffer_event_notify);
    uv_handle_set_data(reinterpret_cast<uv_handle_t*>(buffer_event_notifier_), this);
}

ThreadedAudioSink::~ThreadedAudioSink()
{
    CHECK(disposed_ && "ThreadAudioSink must be disposed before destructing");
}

void ThreadedAudioSink::Dispose(bool call_from_listener)
{
    if (disposed_)
        return;

    NotifyAndWaitWorkerThreadDispose();
    // Now, the worker thread has been closed and mutex is not needed anymore

    if (!call_from_listener)
    {
        // Processing remaining buffer events.
        while (!buffer_event_queue_.empty())
        {
            BroadcastBufferEvent(buffer_event_queue_.front());
            buffer_event_queue_.pop();
        }
    }

    // Cancel remaining buffers
    while (!buffer_queue_.empty())
    {
        BufferEvent event(std::move(buffer_queue_.front()), BufferEvent::kCancelled);
        BroadcastBufferEvent(event);
        buffer_queue_.pop();
    }

    // Free resources
    uv_close(reinterpret_cast<uv_handle_t*>(buffer_event_notifier_),
             [](uv_handle_t *handle) {
        CHECK(handle);
        free(handle);
    });

    buffer_event_notifier_ = nullptr;
    disposed_ = true;
}

void ThreadedAudioSink::AppendBufferEventListener(BufferEventListener *listener)
{
    auto& l = buffer_event_listeners_;
    auto itr = std::find(l.begin(), l.end(), listener);
    if (itr != l.end())
        return;
    l.push_back(listener);
}

void ThreadedAudioSink::RemoveBufferEventListener(BufferEventListener *listener)
{
    buffer_event_listeners_.remove(listener);
}

void ThreadedAudioSink::BroadcastBufferEvent(const BufferEvent& event)
{
    for (BufferEventListener *listener : buffer_event_listeners_)
    {
        CHECK(listener);
        if (event.event_type == BufferEvent::kConsumed)
            listener->OnConsumed(event.buffer);
        else if (event.event_type == BufferEvent::kPlaying)
            listener->OnPlaying(event.buffer);
        else if (event.event_type == BufferEvent::kCancelled)
            listener->OnCancelled(event.buffer);
        else
            MARK_UNREACHABLE();
    }
}

void ThreadedAudioSink::SendBufferEventFromWorkerThread(const BufferEvent& event)
{
    {
        std::scoped_lock<std::mutex> lock(buffer_event_queue_lock_);
        buffer_event_queue_.push(event);
    }
    uv_async_send(buffer_event_notifier_);
}

void ThreadedAudioSink::on_buffer_event_notify(uv_async_t *handle)
{
    CHECK(handle && handle->data);
    auto *sink = reinterpret_cast<ThreadedAudioSink*>(handle->data);

    std::scoped_lock<std::mutex> lock(sink->buffer_event_queue_lock_);
    while (!sink->buffer_event_queue_.empty())
    {
        sink->BroadcastBufferEvent(sink->buffer_event_queue_.front());
        sink->buffer_event_queue_.pop();
    }
}

std::optional<ThreadedAudioSink::BufferWithId> ThreadedAudioSink::TakeNextBuffer()
{
    std::scoped_lock<std::mutex> lock(buffer_queue_lock_);
    if (buffer_queue_.empty())
        return {};

    BufferWithId buffer(std::move(buffer_queue_.front()));
    buffer_queue_.pop();

    return std::move(buffer);
}

int32_t ThreadedAudioSink::EnqueueBuffer(const std::shared_ptr<SoundBuffer>& sound_buffer)
{
    if (!BufferCheckBeforeEnqueue(sound_buffer))
        return -1;

    int32_t id;
    {
        std::scoped_lock<std::mutex> lock(buffer_queue_lock_);
        buffer_queue_.emplace(BufferWithId::Generate(sound_buffer));
        id = buffer_queue_.back().id;
    }
    NotifyNewBufferEnqueued();

    return id;
}

UTAU_NAMESPACE_END
