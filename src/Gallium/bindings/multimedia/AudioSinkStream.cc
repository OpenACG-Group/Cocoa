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

#include <vector>
#include <functional>

#include "Gallium/bindings/multimedia/AudioSinkStream.h"
#include "Gallium/bindings/multimedia/Frame.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

class AudioSinkStream::ListenerImpl : public utau::AudioSinkStream::Listener
{
public:
    using EmitterF = std::function<void(const EventEmitterBase::ListenerArgsT&)>;

    ~ListenerImpl() override = default;

    void OnVolumeChanged(const std::vector<float>& volume) override {
        if (!emit_vol_changed_)
            return;
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        v8::HandleScope handle_scope(isolate);
        emit_vol_changed_({
            ffi::Cast<std::vector<float>>::ToChecked(isolate, volume)
        });
    }

    void OnEmptyQueue(uint64_t last_frame_id) override {
        if (!emit_empty_queue_)
            return;
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        v8::HandleScope handle_scope(isolate);
        emit_empty_queue_({
            v8::BigInt::NewFromUnsigned(isolate, last_frame_id)
        });
    }

    bool IsAllEmpty() const {
        return !emit_vol_changed_ && !emit_empty_queue_;
    }

    EmitterF emit_vol_changed_;
    EmitterF emit_empty_queue_;
};

AudioSinkStream::AudioSinkStream(std::shared_ptr<utau::AudioSinkStream> stream)
    : listener_(std::make_shared<ListenerImpl>())
    , stream_(std::move(stream))
{
    EmitterDefineEvent("volume-changed", [this] {
        listener_->emit_vol_changed_ = EmitterWrapAsCallable("volume-changed");
        stream_->SetListener(listener_);
        return 0;
    }, [this](uint64_t) {
        listener_->emit_vol_changed_ = {};
        if (listener_->IsAllEmpty())
            stream_->SetListener(nullptr);
    });

    EmitterDefineEvent("empty-queue", [this] {
        listener_->emit_empty_queue_ = EmitterWrapAsCallable("empty-queue");
        stream_->SetListener(listener_);
        return 0;
    }, [this](uint64_t) {
        listener_->emit_empty_queue_ = {};
        if (listener_->IsAllEmpty())
            stream_->SetListener(nullptr);
    });
}

ffi::Ret<void> AudioSinkStream::dispose()
{
    EmitterDispose();
    stream_->Dispose();
    stream_.reset();
    NotifyDisposeState(DisposeState::kDisposed);
    return {};
}

ffi::Ret<ffi::PreciseU64> AudioSinkStream::enqueue(ffi::Class<Frame> frame)
{
    if (frame->GetDisposeState() != DisposeState::kNot)
        return ffi::Fail(ffi::kErr, "provided frame instance has been disposed");
    uint64_t id = stream_->Enqueue(frame->GetAVFrame());
    if (id == 0)
        return ffi::Fail(ffi::kErr, "provided frame cannot be accepted by stream");
    return id;
}

ffi::Ret<double> AudioSinkStream::getDelayInUs()
{
    return stream_->GetDelayInUs();
}

ffi::Ret<void> AudioSinkStream::setVolumes(const std::vector<float> &volumes)
{
    stream_->SetVolume(volumes);
    return {};
}

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
