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

#ifndef COCOA_GALLIUM_BINDINGS_MULTIMEDIA_PACKET_H
#define COCOA_GALLIUM_BINDINGS_MULTIMEDIA_PACKET_H

#include "Gallium/bindings/multimedia/ffwrappers/libavcodec-packet.h"

#include "Gallium/bindings/multimedia/Types.h"
#include "Gallium/ffi/JSObject.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

//! TSDecl: @enum PacketFlags
enum class PacketFlags
{
    //! TSDecl: @enumitem Key
    kKey = AV_PKT_FLAG_KEY,

    //! TSDecl: @enumitem Corrupt
    kCorrupt = AV_PKT_FLAG_CORRUPT,

    //! @tsdocbegin
    //! Flag is used to discard packets which are required to maintain valid
    //! decoder state but are not required for output and should be dropped
    //! after decoding.
    //! @tsdocend
    //! TSDecl: @enumitem Discard
    kDiscard = AV_PKT_FLAG_DISCARD,

    //! @tsdocbegin
    //! Flag is used to indicate packets that contain frames that can
    //! be discarded by the decoder.  I.e. Non-reference frames.
    //! @tsdocend
    //! TSDecl: @enumitem Disposable
    kDisposable = AV_PKT_FLAG_DISPOSABLE
};
//! TSDecl: @end

//! @tsdocbegin
//! `Packet` is a reference of a underlying binary buffer, which stores compressed
//! media data. It is typically exported by demuxers and then passed as input to
//! decoders, or received as output from encoders and then passed to muxers.
//!
//! The underlying buffer is reference-counted, each instance of `Packet` should
//! be considered as a reference to a buffer. However, side data in the packet are
//! not shared among instances. They will be copied when a packet is cloned, and will
//! be freed when a packet is disposed.
//!
//! For video, it should typically contain one compressed frame. For audio it may
//! contain several compressed frames. Encoders are allowed to output empty packets,
//! with no compressed data, containing only side data. (e.g. to update some stream
//! parameters at the end of encoding).
//! @tsdocend
//! TSDecl: @class @nonconstructible Packet
class Packet : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit Packet(AVPacket *packet) : packet_(packet) {}
    ~Packet() override;

    g_nodiscard AVPacket *RefPacket() const {
        CHECK(packet_ && "phantom packet object");
        return av_packet_clone(packet_);
    }

    g_nodiscard AVPacket *GetPacket() const {
        CHECK(packet_ && "phantom packet object");
        return packet_;
    }

    g_nodiscard bool IsReusable() const {
        return packet_ && (GetDisposeState() == DisposeState::kDisposed);
    }

    // Only internally used to reuse the packet instance.
    void ResetDisposeState() {
        NotifyDisposeState(DisposeState::kNot);
    }

    //! @tsdocbegin
    //! Create a new instance that shares the same underlying buffer, and copies
    //! side data (if we have).
    //! This operation increases the refcount of the underlying buffer.
    //! @tsdocend
    //! TSDecl: @method clone(): Packet
    ffi::RetLocal<v8::Value> clone();

    //! @tsdocbegin
    //! Destroy the instance and reference (including side data).
    //! This operation decreases the refcount of the underlying buffer.
    //! @tsdocend
    //! TSDecl: @method dispose(): void
    ffi::Ret<void> dispose();

    //! @tsdocbegin
    //! Only deletes the reference to underlying buffer and frees side data.
    //! Like `dispose()`, the instance will become invalid, but the difference is
    //! that it can be reused by functions like `FormatDemuxer.readFrame(packet)`.
    //! @tsdocend
    //! TSDecl: @method disposeReusable(): void
    ffi::Ret<void> disposeReusable();

    //! @tsdocbegin
    //! Presentation timestamp in the stream's timebase units; the time at which
    //! the decompressed packet will be presented to the user.
    //! Can be `null` if it is not stored in the file.
    //! pts MUST be larger or equal to dts as presentation cannot happen before
    //! decompression, unless one wants to view hex dumps. Some formats misuse
    //! the terms dts and pts/cts to mean something different. Such timestamps
    //! must be converted to true pts/dts before they are stored in packet.
    //! @tsdocend
    //! TSDecl: @property @readonly pts: @union(null, i64)
    ffi::RetLocal<v8::Value> getPts();

    //! @tsdocbegin
    //! Decompression timestamp in the stream's timebase units; the time at which
    //! the packet is decompressed.
    //! Can be `null` if it is not stored in the file.
    //! @tsdocend
    //! TSDecl: @property @readonly dts: @union(null, i64)
    ffi::RetLocal<v8::Value> getDts();

    //! @tsdocbegin
    //! Duration of this packet in the stream's timebase units, `null` if unknown,
    //! never 0. Equals `next_pts - this_pts` in presentation order.
    //! @tsdocend
    //! TSDecl: @property @readonly duration: @union(null, i64)
    ffi::RetLocal<v8::Value> getDuration();
    
    //! TSDecl: @property @readonly streamIndex: i32
    ffi::Ret<int32_t> getStreamIndex();

    //! @tsdocbegin
    //! A combination of `PacketFlags.*` values.
    //! @tsdocend
    //! TSDecl: @property @readonly flags: i32
    ffi::Ret<int32_t> getFlags() {
        return packet_->flags;
    }

    // TODO(sora): support accessing side data

private:
    AVPacket *packet_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
#endif //COCOA_GALLIUM_BINDINGS_MULTIMEDIA_PACKET_H
