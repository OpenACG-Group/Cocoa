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

#include "Utau/Utau.h"

#include "Gallium/bindings/multimedia/CodecContext.h"

#include <Utau/HWDeviceContext.h>

#include "Gallium/bindings/multimedia/CodecParameters.h"
#include "Gallium/bindings/multimedia/Packet.h"
#include "Gallium/bindings/multimedia/Frame.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

namespace {

struct CodecContextUserdata
{
    ~CodecContextUserdata() {
        if (hw_frames_ctx)
            av_buffer_unref(&hw_frames_ctx);
    }

    AVBufferRef *hw_frames_ctx = nullptr;
};

void avcodec_free_context_helper(AVCodecContext **ctx_ptr)
{
    AVCodecContext *ctx = *ctx_ptr;
    if (ctx->opaque)
        delete static_cast<CodecContextUserdata*>(ctx->opaque);
    avcodec_free_context(ctx_ptr);
}

} // namespace anonymous

CodecContextBuilder::CodecContextBuilder(const ffi::Enum<CodecType>& type)
    : type_(*type)
    , codec_ctx_(avcodec_alloc_context3(nullptr))
    , options_dict_(nullptr)
{
    CHECK(codec_ctx_ && "allocation failed");
}

CodecContextBuilder::~CodecContextBuilder()
{
    if (codec_ctx_)
        avcodec_free_context_helper(&codec_ctx_);
}

ffi::RetLocal<v8::Value>
CodecContextBuilder::setCodecParameters(const ffi::Class<CodecParameters>& params)
{
    int res = avcodec_parameters_to_context(codec_ctx_, params->GetAVCodecParameters());
    if (res < 0)
        return ffi::Fail(ffi::kErr, fmt::format("failed to set parameters: {}", av_err2str(res)));
    return GetThisHandle(v8::Isolate::GetCurrent());
}

ffi::RetLocal<v8::Value>
CodecContextBuilder::setPacketTimebase(const AVRationalAdapter& timebase)
{
    codec_ctx_->pkt_timebase = *timebase;
    return GetThisHandle(v8::Isolate::GetCurrent());
}

ffi::RetLocal<v8::Value>
CodecContextBuilder::setCodecOption(const std::string& name, const std::string& value)
{
    int res = av_dict_set(&options_dict_, name.c_str(), value.c_str(), 0);
    if (res < 0)
        return ffi::Fail(ffi::kErr, fmt::format("failed to set option: {}", av_err2str(res)));
    return GetThisHandle(v8::Isolate::GetCurrent());
}

ffi::RetLocal<v8::Value>
CodecContextBuilder::setEncoderHWFramesContext(ffi::Class<HWFramesContext> hw_frames_ctx)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    encoder_hw_frames_ctx_.Reset(isolate, hw_frames_ctx->GetThisHandle(isolate));
    return GetThisHandle(isolate);
}

ffi::RetLocal<v8::Value>
CodecContextBuilder::setDecoderHWDeviceContext(ffi::Class<HWDeviceContext> hw_device_ctx)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    decoder_hw_device_ctx_.Reset(isolate, hw_device_ctx->GetThisHandle(isolate));
    return GetThisHandle(isolate);
}

ffi::RetLocal<v8::Value> CodecContextBuilder::detach()
{
    const AVCodec *codec;
    if (type_ == CodecType::kEncoder)
        codec = avcodec_find_encoder(codec_ctx_->codec_id);
    else if (type_ == CodecType::kDecoder)
        codec = avcodec_find_decoder(codec_ctx_->codec_id);
    else
        MARK_UNREACHABLE();

    if (!codec)
        return ffi::Fail(ffi::kErr, "failed to find a proper codec, not embedded or missing valid codec ID");

    CodecContextUserdata *userdata = new CodecContextUserdata;
    codec_ctx_->opaque = userdata;

    if (type_ == CodecType::kEncoder && !encoder_hw_frames_ctx_.IsEmpty())
    {
        if (codec_ctx_->codec_type != AVMEDIA_TYPE_VIDEO)
            return ffi::Fail(ffi::kErr, "only video codecs support hardware acceleration");
        if (encoder_hw_frames_ctx_->GetDisposeState() != DisposeState::kNot)
            return ffi::Fail(ffi::kErr, "given `HWFramesContext` has been disposed");

        AVBufferRef *hw_frames_ctx = encoder_hw_frames_ctx_->GetAVBufferRef();
        AVHWFramesContext *hwctx = reinterpret_cast<AVHWFramesContext*>(hw_frames_ctx->data);
        codec_ctx_->pix_fmt = hwctx->format;
        codec_ctx_->hw_frames_ctx = av_buffer_ref(hw_frames_ctx);
    }
    encoder_hw_frames_ctx_.Reset();

    if (type_ == CodecType::kDecoder && !decoder_hw_device_ctx_.IsEmpty())
    {
        if (codec_ctx_->codec_type != AVMEDIA_TYPE_VIDEO)
            return ffi::Fail(ffi::kErr, "only video codecs support hardware acceleration");
        if (decoder_hw_device_ctx_->GetDisposeState() != DisposeState::kNot)
            return ffi::Fail(ffi::kErr, "given `HWDeviceContext` has been disposed");

        // Set the device context to let libavcodec handles all the remaining things automatically
        codec_ctx_->hw_device_ctx = av_buffer_ref(decoder_hw_device_ctx_->GetContext());
    }
    decoder_hw_device_ctx_.Reset();

    int res = avcodec_open2(codec_ctx_, codec, &options_dict_);
    av_dict_free(&options_dict_);
    if (res < 0)
        return ffi::Fail(ffi::kErr, fmt::format("failed to open context: {}", av_err2str(res)));

    AVCodecContext *context = codec_ctx_;
    codec_ctx_ = nullptr;
    NotifyDisposeState(DisposeState::kDisposed);

    return ffi::JSObject::New<CodecContext>(v8::Isolate::GetCurrent(), context);
}

CodecContext::~CodecContext()
{
    if (codec_ctx_)
        avcodec_free_context_helper(&codec_ctx_);
}

ffi::Ret<void> CodecContext::dispose()
{
    avcodec_free_context_helper(&codec_ctx_);
    NotifyDisposeState(DisposeState::kDisposed);
    return {};
}

namespace {

int32_t to_codec_state(int res, bool is_input)
{
    if (res >= 0)
        return static_cast<int32_t>(CodecStatus::kSuccess);
    if (res == AVERROR(EAGAIN))
    {
        return static_cast<int32_t>(is_input ?
            CodecStatus::kNeedConsumeOutput : CodecStatus::kNeedFeedInput);
    }
    if (res == AVERROR_EOF)
        return static_cast<int32_t>(CodecStatus::kEOF);
    if (res == AVERROR(EINVAL))
        return static_cast<int32_t>(CodecStatus::kInvalid);
    if (res == AVERROR(ENOMEM))
        return static_cast<int32_t>(CodecStatus::kNoMemory);
    return static_cast<int32_t>(CodecStatus::kError);
}

v8::Local<v8::Value> to_status_tuple(int res, v8::Local<v8::Value> v)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (v.IsEmpty())
        v = v8::Null(isolate);
    using Tuple = std::tuple<int32_t, v8::Local<v8::Value>>;
    return ffi::Cast<Tuple>::ToChecked(isolate, {to_codec_state(res, false), v});
}

} // namespace anonymous

ffi::Ret<int32_t> CodecContext::sendPacket(const ffi::Opt<ffi::Class<Packet>>& packet)
{
    AVPacket *avpacket = nullptr;
    if (packet)
    {
        if ((*packet)->GetDisposeState() != DisposeState::kNot)
            return ffi::Fail(ffi::kErr, "the packet has been disposed");
        avpacket = (*packet)->GetPacket();
    }
    return to_codec_state(avcodec_send_packet(codec_ctx_, avpacket), true);
}

ffi::RetLocal<v8::Value> CodecContext::receiveFrame(const ffi::Opt<ffi::Class<Frame>>& reuse)
{
    AVFrame *frame;
    if (reuse)
    {
        if (!(*reuse)->IsReusable())
            return ffi::Fail(ffi::kErr, "provided frame instance is not reusable");
        frame = (*reuse)->GetAVFrame();
    }
    else
    {
        frame = av_frame_alloc();
        CHECK(frame && "allocation failed");
    }

    int res = avcodec_receive_frame(codec_ctx_, frame);
    if (res < 0)
    {
        if (!reuse)
            av_frame_free(&frame);
        return to_status_tuple(res, {});
    }

    if (reuse)
        (*reuse)->ResetDisposeState();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return to_status_tuple(res,
        reuse
        ? (*reuse)->GetThisHandle(isolate)
        : ffi::JSObject::New<Frame>(isolate, frame)
    );
}

ffi::Ret<int32_t> CodecContext::sendFrame(const ffi::Opt<ffi::Class<Frame>>& frame)
{
    AVFrame *avframe = nullptr;
    if (frame)
    {
        if ((*frame)->GetDisposeState() != DisposeState::kNot)
            return ffi::Fail(ffi::kErr, "the frame has been disposed");
        avframe = (*frame)->GetAVFrame();
    }
    return to_codec_state(avcodec_send_frame(codec_ctx_, avframe), true);
}

ffi::RetLocal<v8::Value> CodecContext::receivePacket(const ffi::Opt<ffi::Class<Packet>>& reuse)
{
    AVPacket *packet;
    if (reuse)
    {
        if (!(*reuse)->IsReusable())
            return ffi::Fail(ffi::kErr, "provided packet instance is not reusable");
        packet = (*reuse)->GetPacket();
    }
    else
    {
        packet = av_packet_alloc();
        CHECK(packet && "allocation failed");
    }

    int res = avcodec_receive_packet(codec_ctx_, packet);
    if (res < 0)
    {
        if (!reuse)
            av_packet_free(&packet);
        return to_status_tuple(res, {});
    }

    if (reuse)
        (*reuse)->ResetDisposeState();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return to_status_tuple(res,
        reuse
        ? (*reuse)->GetThisHandle(isolate)
        : ffi::JSObject::New<Packet>(isolate, packet)
    );
}

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
