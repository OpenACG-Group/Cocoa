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

#ifndef COCOA_GALLIUM_BINDINGS_MULTIMEDIA_CODECCONTEXT_H
#define COCOA_GALLIUM_BINDINGS_MULTIMEDIA_CODECCONTEXT_H

#include "Gallium/bindings/multimedia/ffwrappers/libavcodec.h"

#include "Gallium/bindings/multimedia/Types.h"
#include "Gallium/bindings/multimedia/HWFramesContext.h"
#include "Gallium/bindings/multimedia/HWDeviceContext.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Class.h"
#include "Gallium/ffi/Enum.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

class CodecParameters;
class Packet;
class Frame;

//! TSDecl: @enum CodecType
enum class CodecType
{
    //! TSDecl: @enumitem Encoder
    kEncoder,
    //! TSDecl: @enumitem Decoder
    kDecoder
};
//! TSDecl: @end

//! @tsdocbegin
//! Helps find a proper decoder/encoder, and create a `CodecContext` instance
//! from the given parameters. Cannot be reused.
//!
//! To build a codec context, you first should call `setXXX()` to provide a set of
//! parameters required by codec. Finally, call `detach()` to get the created context
//! and simultaneously dispose the builder.
//! @tsdocend
//! TSDecl: @class CodecContextBuilder
class CodecContextBuilder : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    //! TSDecl: @constructor(type: CodecType)
    explicit CodecContextBuilder(const ffi::Enum<CodecType>& type);
    ~CodecContextBuilder() override;

    //! TSDecl: @method setCodecParameters(params: CodecParameters): CodecContextBuilder
    ffi::RetLocal<v8::Value> setCodecParameters(const ffi::Class<CodecParameters>& params);

    //! TSDecl: @method setPacketTimebase(timebase: Rational): CodecContextBuilder
    ffi::RetLocal<v8::Value> setPacketTimebase(const AVRationalAdapter& timebase);

    //! TSDecl: @method setCodecOption(name: string, value: string): CodecContextBuilder
    ffi::RetLocal<v8::Value> setCodecOption(const std::string& name, const std::string& value);

    // TODO(sora): support other options

    //! TSDecl: @method setEncoderHWFramesContext(hwFramesCtx: HWFramesContext): CodecContextBuilder
    ffi::RetLocal<v8::Value> setEncoderHWFramesContext(ffi::Class<HWFramesContext> hw_frames_ctx);

    //! TSDecl: @method setDecoderHWDeviceContext(device: HWDeviceContext): CodecContextBuilder
    ffi::RetLocal<v8::Value> setDecoderHWDeviceContext(ffi::Class<HWDeviceContext> hw_device_ctx);

    //! TSDecl: @method detach(): CodecContext
    ffi::RetLocal<v8::Value> detach();

private:
    CodecType type_;
    AVCodecContext *codec_ctx_;
    AVDictionary *options_dict_;
    ffi::ClassInstance<HWFramesContext> encoder_hw_frames_ctx_;
    ffi::ClassInstance<HWDeviceContext> decoder_hw_device_ctx_;
};
//! TSDecl: @end

//! TSDecl: @enum CodecStatus
enum class CodecStatus : int32_t
{
    //! TSDecl: @enumitem Success
    kSuccess,

    //! @tsdocbegin
    //! Input is not accepted in the current state. User must read output first,
    //! and once all output is read, the input should be resent, and the call will
    //! not fail with this state.
    //! @tsdocend
    //! TSDecl: @enumitem NeedConsumeOutput
    kNeedConsumeOutput,

    //! @tsdocbegin
    //! Output is not available in this state. User must try to send new input.
    //! @tsdocend
    //! TSDecl: @enumitem NeedFeedInput
    kNeedFeedInput,

    //! @tsdocbegin
    //! The codec has been flushed, and neither new input can be sent to it, nor
    //! new output can be received from it.
    //! @tsdocend
    //! TSDecl: @enumitem EOF
    kEOF,

    //! @tsdocbegin
    //! Invalid use of codec (e.g. use a encoder as a decoder).
    //! See detailed information in the comment of related methods.
    //! @tsdocend
    //! TSDecl: @enumitem Invalid
    kInvalid,

    //! @tsdocbegin
    //! Failed to allocate memory; failed to add input to internal queue; or similar.
    //! @tsdocend
    //! TSDecl: @enumitem NoMemory
    kNoMemory,

    //! TSDecl: @enumitem Error
    kError
};
//! TSDecl: @end

//! TSDecl: @class @nonconstructible CodecContext
class CodecContext : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit CodecContext(AVCodecContext *ctx) : codec_ctx_(ctx) {}
    ~CodecContext() override;

    //! TSDecl: @method dispose(): void
    ffi::Ret<void> dispose();

    //! @tsdocbegin
    //! Supply raw packet data as input to a decoder, and then `receiveFrame()`
    //! should be used to receive the decoded pixel data.
    //!
    //! The provided packet is fully consumed, and if it contains multiple frames
    //! (e.g. some audio codecs), will require you to call `receiveFrame()` multiple
    //! times afterwards before you can send a new packet. It can be `null` or an
    //! empty packet; in this case, it is considered a flush packet, which signals the
    //! end of the stream. Sending the first flush packet will return success.
    //! Subsequent ones are unnecessary and will return EOF. If the decoder still
    //! has frames buffered, it will return them after sending a flush packet.
    //!
    //! Possible status returned:
    //!   Success
    //!   NeedConsumeOutput
    //!   EOF                   - the decoder has been flushed, and no new packets
    //!                           can be sent to it (also returned if more than 1 flush
    //!                           packet is sent)
    //!   Invalid               - it is an encoder, or requires flush
    //!   NoMemory
    //!   Error
    //!
    //! @tsdocend
    //! TSDecl: @method sendPacket(packet: @union(Packet, null)): CodecStatus
    ffi::Ret<int32_t> sendPacket(const ffi::Opt<ffi::Class<Packet>>& packet);

    //! @tsdocbegin
    //! Return decoded output data from a decoder or encoder
    //! (when the `CodecFlags.ReconFrame` flag is used).
    //!
    //! If `reuse` argument is null, a new frame instance will be created and returned;
    //! otherwise, we try to reuse the provided frame to store data, and returns the same
    //! instance. If the provided frame is not reusable, throws an exception.
    //!
    //! Possible status returned:
    //!   Success
    //!   NeedFeedInput
    //!   EOF                   - the codec has been fully flushed, and there will be
    //!                           no more output frames
    //!   Invalid               - it is an encoder without the `CodecFlags.ReconFrame`
    //!                           flag enabled
    //!   Error
    //! @tsdocend
    //! TSDecl: @method receiveFrame(reuse: @union(null, Frame)): @tuple(CodecStatus, Frame)
    ffi::RetLocal<v8::Value> receiveFrame(const ffi::Opt<ffi::Class<Frame>>& reuse);

    //! @tsdocbegin
    //! Supply a raw video or audio frame to the encoder. Use `receivePacket()` to retrieve
    //! buffered output packets.
    //!
    //! The provided frame can be `null`, in which case it is considered a flush packet.
    //! This signals the end of the stream. If the encoder still has packets buffered,
    //! it will return them after this call. Once flushing mode has been entered, additional
    //! flush packets are ignored, and sending frames will return EOF.
    //!
    //! For audio, if `CodecFlags.CapVariableFrameSize` is set, then each frame can have
    //! any number of samples. If it is not set, the number of samples must equal to
    //! `CodecContext.frameSize` for all frames except the last. The final frame may be
    //! smaller than we required.
    //!
    //! Possible status returned:
    //!   Success
    //!   NeedConsumeOutput
    //!   EOF                   - the encoder has been flushed, and no new frames can be
    //!                           sent to it
    //!   Invalid               - it is a decoder, or requires flush
    //!   NoMemory
    //!   Error
    //! @tsdocend
    //! TSDecl: @method sendFrame(frame: @union(Frame, null)): CodecStatus
    ffi::Ret<int32_t> sendFrame(const ffi::Opt<ffi::Class<Frame>>& frame);

    //! @tsdocbegin
    //! Read encoded data from the encoder.
    //!
    //! If `reuse` argument is null, a new packet instance will be created and returned;
    //! otherwise, we try to reuse the provided packet to store data, and returns the same
    //! instance. If the provided packet is not reusable, throws an exception.
    //!
    //! Possible status returned:
    //!   Success
    //!   NeedFeedInput
    //!   EOF                   - the encoder has been fully flushed, and there will be no
    //!                           more output packets
    //!   Invalid               - it is a decoder
    //!   Error
    //! @tsdocend
    //! TSDecl: @method receivePacket(reuse: @union(null, Packet)): @tuple(CodecStatus, Packet)
    ffi::RetLocal<v8::Value> receivePacket(const ffi::Opt<ffi::Class<Packet>>& reuse);

    // TODO(sora): support other operations

private:
    AVCodecContext *codec_ctx_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
#endif //COCOA_GALLIUM_BINDINGS_MULTIMEDIA_CODECCONTEXT_H
