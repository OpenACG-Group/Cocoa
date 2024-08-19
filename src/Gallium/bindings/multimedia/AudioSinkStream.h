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

#ifndef COCOA_GALLIUM_BINDINGS_MULTIMEDIA_AUDIOSINKSTREAM_H
#define COCOA_GALLIUM_BINDINGS_MULTIMEDIA_AUDIOSINKSTREAM_H

#include "Utau/AudioSinkStream.h"
#include "Gallium/bindings/multimedia/Types.h"
#include "Gallium/bindings/EventEmitter.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Class.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

class Frame;

//! @tsdocbegin
//! A stream that allows writing audio frames to the audio service.
//!
//! `AudioSinkStream` is an event emitter of the following events:
//!   @event volume-changed(volumes: f32[]): when volume has been changed by system;
//!                                          volumes are in the current channel order.
//!
//!   @event empty-queue(lastFrameId: bigint): when the last frame has been consumed and
//!                                            the queue becomes empty. `lastFrameId` is
//!                                            the return value of the last `enqueue()`.
//! @tsdocend
//! TSDecl: @class @nonconstructible @extends(@import(event) EventEmitterBase) AudioSinkStream
class AudioSinkStream : public EventEmitterBase
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    class ListenerImpl;

    explicit AudioSinkStream(std::shared_ptr<utau::AudioSinkStream> stream);
    ~AudioSinkStream() override = default;

    //! TSDecl: @method dispose(): void
    ffi::Ret<void> dispose();

    //! @tsdocbegin
    //! Pushes a frame into the presentation queue of the sink stream, and the frame
    //! will be played as soon as possible. Timestamp of the frame is ignored.
    //! Returns a frame ID on success, which can be used when handling `empty-queue` event.
    //!
    //! The frame must be an audio frame, and its format, sample rate, and channel
    //! layout must be strictly identical to what were used to create the sink stream.
    //! Otherwise, throws an exception on failure.
    //!
    //! Contents of frame will not be touched, and it is dispose-safe.
    //! @tsdocend
    //! TSDecl: @method enqueue(frame: Frame): bigint
    ffi::Ret<ffi::PreciseU64> enqueue(ffi::Class<Frame> frame);

    //! @tsdocbegin
    //! Returns the current delay of system audio, in microseconds.
    //! Useful for realtime audio to make sure the frame will be played at a correct time.
    //! @tsdocend
    //! TSDecl: @method getDelayInUs(): f64
    ffi::Ret<double> getDelayInUs();

    //! @tsdocbegin
    //! Set volumes for each channel. `volumes.length` must be identical to
    //! the number of channels in the current channel layout; otherwise it does nothing.
    //! A `volume-changed` event will be triggered when the volumes are updated.
    //! @tsdocend
    //! TSDecl: @method setVolumes(volumes: @array(f64)): void
    ffi::Ret<void> setVolumes(const std::vector<float>& volumes);

private:
    std::shared_ptr<ListenerImpl> listener_;
    std::shared_ptr<utau::AudioSinkStream> stream_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
#endif //COCOA_GALLIUM_BINDINGS_MULTIMEDIA_AUDIOSINKSTREAM_H
