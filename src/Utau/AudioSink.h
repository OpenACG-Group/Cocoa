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

#ifndef COCOA_UTAU_AUDIOSINK_H
#define COCOA_UTAU_AUDIOSINK_H

#include <queue>
#include <mutex>
#include <utility>
#include <list>
#include <optional>

#include "uv.h"

#include "Utau/Utau.h"
UTAU_NAMESPACE_BEGIN

class SoundBuffer;

class ThreadedAudioSink
{
public:
    enum BackDevice
    {
        kPipeWire_BackDevice
    };

    struct BufferWithId
    {
        static BufferWithId Generate(std::shared_ptr<SoundBuffer> buffer);

        BufferWithId(std::shared_ptr<SoundBuffer> _buffer, int32_t _id)
            : buffer(std::move(_buffer)), id(_id) {}

        BufferWithId(BufferWithId&& rhs) noexcept
            : buffer(std::move(rhs.buffer)), id(rhs.id) {}

        BufferWithId(const BufferWithId& lhs) = default;

        std::shared_ptr<SoundBuffer> buffer;
        int32_t id;
    };

    class BufferEventListener
    {
    public:
        virtual void OnConsumed(const BufferWithId& buf) = 0;
        virtual void OnPlaying(const BufferWithId& buf) = 0;
        virtual void OnCancelled(const BufferWithId& buf) = 0;
    };

    struct BufferEvent
    {
        enum EventType
        {
            kConsumed,
            kPlaying,
            kCancelled
        };

        BufferEvent(BufferWithId buffer_with_id, EventType event)
            : buffer(std::move(buffer_with_id)), event_type(event) {}

        BufferWithId buffer;
        EventType    event_type;
    };

    ThreadedAudioSink(uv_loop_t *main_loop, BackDevice device);
    virtual ~ThreadedAudioSink();

    g_nodiscard g_inline BackDevice GetBackDevice() const {
        return back_device_;
    }

    void AppendBufferEventListener(BufferEventListener *listener);
    void RemoveBufferEventListener(BufferEventListener *listener);

    int32_t EnqueueBuffer(const std::shared_ptr<SoundBuffer>& sound_buffer);
    void CancelBuffer(int32_t buffer);

    void Dispose(bool call_from_listener);

protected:
    // Call them on worker thread
    void SendBufferEventFromWorkerThread(const BufferEvent& event);
    std::optional<BufferWithId> TakeNextBuffer();

    // Call it on main thread
    void BroadcastBufferEvent(const BufferEvent& event);

    // They will be called on main thread
    virtual void NotifyAndWaitWorkerThreadDispose() = 0;
    virtual void NotifyNewBufferEnqueued() = 0;
    virtual bool BufferCheckBeforeEnqueue(const std::shared_ptr<SoundBuffer>& buffer) = 0;

private:
    static void on_buffer_event_notify(uv_async_t *handle);

    BackDevice                  back_device_;
    bool                        disposed_;
    uv_async_t                 *buffer_event_notifier_;
    std::list<BufferEventListener*> buffer_event_listeners_;
    std::queue<BufferEvent>     buffer_event_queue_;
    std::mutex                  buffer_event_queue_lock_;

    std::queue<BufferWithId>    buffer_queue_;
    std::mutex                  buffer_queue_lock_;
};

UTAU_NAMESPACE_END
#endif //COCOA_UTAU_AUDIOSINK_H
