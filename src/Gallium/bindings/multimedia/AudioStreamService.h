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

#ifndef COCOA_GALLIUM_BINDINGS_MULTIMEDIA_AUDIOSTREAMSERVICE_H
#define COCOA_GALLIUM_BINDINGS_MULTIMEDIA_AUDIOSTREAMSERVICE_H

#include "Utau/AudioDevice.h"
#include "Gallium/bindings/multimedia/Types.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/Class.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

class AChannelLayout;

//! @tsdocbegin
//! An active connection to the system's audio service backend (e.g. PipeWire on Linux).
//! Once a connection is established via `Connect()` method.
//!
//! The connection itself is reference counted - when all the users of the connection
//! (they can be audio streams created by the connection) are disposed.
//! @tsdocend
//! TSDecl: @class @nonconstructible AudioStreamService
class AudioStreamService : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit AudioStreamService(std::shared_ptr<utau::AudioDevice> device)
        : device_(std::move(device)) {}
    ~AudioStreamService() override = default;

    g_nodiscard std::shared_ptr<utau::AudioDevice> GetDevice() const {
        CHECK(device_ && "phantom AudioStreamService instance");
        return device_;
    }

    //! @tsdocbegin
    //! Establishes a new connection to the system's audio service. On success,
    //! it prevents the event loop from exiting.
    //! Throws an exception on failure.
    //! @tsdocend
    //! TSDecl: @method @static Connect(): AudioStreamService
    static ffi::RetLocal<v8::Value> Connect();

    //! @tsdocbegin
    //! Disposes this instance. It does NOT mean to release the connection, but just
    //! decreases the refcount of the connection and invalidates this instance.
    //! Other users (streams created by the connection) may still need the connection,
    //! when all of them are disposed, the connection will be released.
    //! @tsdocend
    //! TSDecl: @method dispose(): void
    ffi::Ret<void> dispose();

    //! @tsdocbegin
    //! Create a sink (output) audio stream. The new stream increases the refcount of
    //! the connection.
    //! Throws an exception on failure.
    //!
    //! @param name     Can be any string describing the stream. May be showed by the
    //!                 desktop environment to indicate your application. Will not be
    //!                 parsed by any program.
    //! @param format   Sample format, the created stream only accepts this format.
    //! @param sampleRate Sample rate in Hz, the created stream only accepts this sample rate.
    //! @param channelLayout Channel layout, the created stream only accepts this layout.
    //! @param realtime Whether the stream is a realtime.
    //! @tsdocend
    //! TSDecl: @method createSinkStream(name: string, format: SampleFormat,
    //! TSDecl:                          sampleRate: i32, channelLayout: AChannelLayout,
    //! TSDecl:                          realtime: boolean): AudioSinkStream
    ffi::RetLocal<v8::Value> createSinkStream(const std::string& name,
                                              ffi::Enum<SampleFormat> format,
                                              int32_t sample_rate,
                                              ffi::Class<AChannelLayout> channel_layout,
                                              bool realtime);

private:
    std::shared_ptr<utau::AudioDevice> device_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
#endif //COCOA_GALLIUM_BINDINGS_MULTIMEDIA_AUDIOSTREAMSERVICE_H
