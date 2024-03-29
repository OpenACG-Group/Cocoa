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

#include "Core/TraceEvent.h"
#include "Core/EventLoop.h"
#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Core/Exception.h"
#include "Utau/ffwrappers/libavformat.h"
#include "Utau/ffwrappers/libavcodec.h"
#include "Utau/AVStreamDecoder.h"
#include "Utau/AudioBuffer.h"
#include "Utau/HWDeviceContext.h"
UTAU_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Utau.AVStreamDecoder)

namespace {

const char *get_averror_str(int error)
{
    static thread_local char buf[512];
    av_strerror(error, buf, 512);
    return buf;
}

} // namespace anonymous

struct AVStreamDecoder::DataAVIOContextPriv
{
public:
    // 4K streaming data buffer
    constexpr static size_t kBufferSize = 4 * 1024;

    explicit DataAVIOContextPriv(std::shared_ptr<Data> from_data)
        : data_(std::move(from_data))
        , avio_context_(nullptr)
    {
        CHECK(data_);

        // There is no need to free `buffer` as it is freed
        // by libavformat automatically.
        auto *buffer = static_cast<uint8_t*>(av_malloc(kBufferSize));
        CHECK(buffer && "Failed to allocate memory");

        avio_context_ = avio_alloc_context(buffer, kBufferSize, 0, this,
                                           &read_packet, nullptr, &seek);
        CHECK(avio_context_ && "Failed to allocate AVIOContext");
    }

    ~DataAVIOContextPriv()
    {
        av_free(avio_context_);
    }

    static int read_packet(void *opaque, uint8_t *buf_ptr, int buf_size)
    {
        CHECK(opaque);
        auto *self = static_cast<DataAVIOContextPriv*>(opaque);
        return static_cast<int>(self->data_->read(buf_ptr, buf_size));
    }

    static int64_t seek(void *opaque, int64_t offset, int whence)
    {
        CHECK(opaque);
        auto *self = static_cast<DataAVIOContextPriv*>(opaque);

        static std::unordered_map<int, vfs::SeekWhence> g_whence_map{
            { SEEK_SET,     vfs::SeekWhence::kSet },
            { SEEK_CUR,     vfs::SeekWhence::kCurrent },
            { SEEK_END,     vfs::SeekWhence::kEnd }
        };
        if (g_whence_map.count(whence) == 0)
            return -1;

        return self->data_->seek(g_whence_map[whence], offset);
    }

    std::shared_ptr<Data>        data_;
    AVIOContext                 *avio_context_;
};

struct AVStreamDecoder::DecoderPriv
{
#define SAFE_FREE(f, x) if (x) { f(x); }
    ~DecoderPriv()
    {
        SAFE_FREE(av_frame_free, &current_frame_)
        SAFE_FREE(av_packet_free, &packet_)
        SAFE_FREE(avcodec_free_context, &acodec_ctx_)
        SAFE_FREE(avcodec_free_context, &vcodec_ctx_)
        SAFE_FREE(avformat_free_context, format_ctx_)
        SAFE_FREE(av_buffer_unref, &hw_frames_ctx_);
    }
#undef SAFE_FREE

    AVFormatContext *format_ctx_ = nullptr;
    int audio_stream_idx_ = -1;
    int video_stream_idx_ = -1;

    AVCodecContext *acodec_ctx_ = nullptr;
    AVCodecContext *vcodec_ctx_ = nullptr;

    AVBufferRef *hw_frames_ctx_ = nullptr;

    AVPacket    *packet_ = nullptr;
    AVFrame     *current_frame_ = nullptr;
};

enum class StreamType
{
    kAudio,
    kVideo
};

namespace {

AVPixelFormat vcodec_get_format_cb(g_maybe_unused AVCodecContext *ctx,
                                   const AVPixelFormat *formats)
{
    bool found_vaapi = false;
    for (const AVPixelFormat *p = formats; *p != AV_PIX_FMT_NONE; p++)
    {
        if (*p == AV_PIX_FMT_VAAPI)
        {
            found_vaapi = true;
            break;
        }
    }

    if (!found_vaapi)
    {
        QLOG(LOG_ERROR, "VAAPI is not supported by chosen decoder");
        return AV_PIX_FMT_NONE;
    }

    auto priv = reinterpret_cast<AVStreamDecoder::DecoderPriv*>(ctx->opaque);
    CHECK(priv && priv->hw_frames_ctx_);

    ctx->hw_frames_ctx = av_buffer_ref(priv->hw_frames_ctx_);

    return AV_PIX_FMT_VAAPI;
}

bool open_stream_decoder(AVStreamDecoder::DecoderPriv *priv, StreamType st_type,
                         const AVStreamDecoder::Options& options)
{
    CHECK(priv);

    // Select a stream index
    int st_index = -1;
    if (st_type == StreamType::kVideo)
        st_index = priv->video_stream_idx_;
    else
        st_index = priv->audio_stream_idx_;

    if (st_index < 0)
        return false;

    // Prepare codec context
    AVCodecContext *codec_ctx = avcodec_alloc_context3(nullptr);
    if (!codec_ctx)
        return false;

    codec_ctx->opaque = priv;

    AVStream *stream = priv->format_ctx_->streams[st_index];
    int error = avcodec_parameters_to_context(codec_ctx, stream->codecpar);
    if (error < 0)
    {
        QLOG(LOG_ERROR, "Failed to fill parameters to codec context");
        avcodec_free_context(&codec_ctx);
        return false;
    }

    codec_ctx->pkt_timebase = stream->time_base;

    // Find an appropriate codec
    const AVCodec *codec = avcodec_find_decoder(codec_ctx->codec_id);
    std::string_view force_codec_name;

    if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
        force_codec_name = options.video_codec_name;
    else if (codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO)
        force_codec_name = options.audio_codec_name;

    if (!force_codec_name.empty())
        codec = avcodec_find_decoder_by_name(force_codec_name.data());

    if (!codec)
    {
        if (!force_codec_name.empty())
            QLOG(LOG_ERROR, "Could not find a decoder named {}", force_codec_name);
        else
            QLOG(LOG_ERROR, "Could not find a decoder for current stream");
        avcodec_free_context(&codec_ctx);
        return false;
    }

    codec_ctx->codec_id = codec->id;

    if (st_type == StreamType::kVideo && options.use_hw_decode)
    {
        std::shared_ptr<HWDeviceContext> device_ctx =
                GlobalContext::Ref().GetHWDeviceContext();
        if (!device_ctx)
        {
            QLOG(LOG_ERROR, "Failed to create hardware device context");
            avcodec_free_context(&codec_ctx);
            return false;
        }

        codec_ctx->get_format = vcodec_get_format_cb;
        codec_ctx->hw_device_ctx = av_buffer_ref(device_ctx->GetAVContext());

        // Create a new `AVHWFramesContext` to allocate frames
        AVBufferRef *hwframe_ctx_buf = av_hwframe_ctx_alloc(codec_ctx->hw_device_ctx);
        CHECK(hwframe_ctx_buf);

        auto *hwframe_ctx = reinterpret_cast<AVHWFramesContext*>(hwframe_ctx_buf->data);
        hwframe_ctx->format = AV_PIX_FMT_VAAPI;
        hwframe_ctx->sw_format = AV_PIX_FMT_NV12;
        hwframe_ctx->width = stream->codecpar->width;
        hwframe_ctx->height = stream->codecpar->height;
        hwframe_ctx->initial_pool_size = 20;

        if (av_hwframe_ctx_init(hwframe_ctx_buf) < 0)
        {
            QLOG(LOG_ERROR, "Failed to create AVHWFramesContext");
            return AV_PIX_FMT_NONE;
        }

        priv->hw_frames_ctx_ = hwframe_ctx_buf;
        QLOG(LOG_DEBUG, "Attempt to use hwaccel for video decoding");
    }

    // Open the decoder
    error = avcodec_open2(codec_ctx, codec, nullptr);
    if (error < 0)
    {
        QLOG(LOG_ERROR, "Failed to open decoder: {}", get_averror_str(error));
        return false;
    }

    if (st_type == StreamType::kVideo)
        priv->vcodec_ctx_ = codec_ctx;
    else
        priv->acodec_ctx_ = codec_ctx;

    return true;
}

} // namespace anonymous

std::unique_ptr<AVStreamDecoder>
AVStreamDecoder::MakeFromData(const std::shared_ptr<Data>& data,
                              const Options& options)
{
    if (!data)
        return nullptr;

    auto decoder = std::make_unique<AVStreamDecoder>();

    // Setup stream IO related objects
    decoder->avio_context_priv_ = std::make_unique<DataAVIOContextPriv>(data);
    decoder->decoder_priv_ = std::make_unique<DecoderPriv>();
    auto& decoder_priv = decoder->decoder_priv_;

    decoder_priv->format_ctx_ = avformat_alloc_context();
    if (!decoder_priv->format_ctx_)
        return nullptr;

    decoder_priv->format_ctx_->pb = decoder->avio_context_priv_->avio_context_;
    int error = avformat_open_input(
            &decoder_priv->format_ctx_, "internal-memory", nullptr, nullptr);
    if (error < 0)
    {
        QLOG(LOG_ERROR, "Failed to open AVFormat input: {}", get_averror_str(error));
        return nullptr;
    }

    // Find stream info
    error = avformat_find_stream_info(decoder_priv->format_ctx_, nullptr);
    if (error < 0)
    {
        QLOG(LOG_ERROR, "Failed to find stream information: {}", get_averror_str(error));
        return nullptr;
    }

    // Find & match available streams
    int audio_stream_idx = -1, video_stream_idx = -1;

    if (!options.disable_video)
    {
        video_stream_idx = av_find_best_stream(decoder_priv->format_ctx_,
                                               AVMEDIA_TYPE_VIDEO,
                                               -1, -1, nullptr, 0);
    }

    if (!options.disable_audio)
    {
        audio_stream_idx = av_find_best_stream(decoder_priv->format_ctx_,
                                               AVMEDIA_TYPE_AUDIO,
                                               -1, video_stream_idx, nullptr, 0);
    }

    decoder->has_video_stream_ = video_stream_idx >= 0;
    decoder->has_audio_stream_ = audio_stream_idx >= 0;
    decoder_priv->video_stream_idx_ = video_stream_idx;
    decoder_priv->audio_stream_idx_ = audio_stream_idx;

    // Open the corresponding codecs
    if (video_stream_idx >= 0)
    {
        if (!open_stream_decoder(decoder_priv.get(), StreamType::kVideo, options))
            return nullptr;
    }

    if (audio_stream_idx >= 0)
    {
        if (!open_stream_decoder(decoder_priv.get(), StreamType::kAudio, options))
            return nullptr;
    }

    return decoder;
}

AVStreamDecoder::AVStreamDecoder()
    : has_video_stream_(false)
    , has_audio_stream_(false)
{
}

AVStreamDecoder::~AVStreamDecoder()
{
    decoder_priv_.reset();
    avio_context_priv_.reset();
}

AVBufferRef *AVStreamDecoder::GetHWFramesContext() const
{
    if (!has_video_stream_)
        return nullptr;
    return decoder_priv_->hw_frames_ctx_;
}

bool AVStreamDecoder::SeekStreamTo(StreamSelector stream, int64_t ts)
{
    int32_t stream_idx = -1;
    if (stream == StreamSelector::kAudio_StreamType && has_audio_stream_)
        stream_idx = decoder_priv_->audio_stream_idx_;
    else if (stream == StreamSelector::kVideo_StreamType && has_video_stream_)
        stream_idx = decoder_priv_->video_stream_idx_;

    if (stream_idx < 0)
        return false;

    int ret = av_seek_frame(decoder_priv_->format_ctx_, stream_idx, ts, 0);
    return (ret >= 0);
}

bool AVStreamDecoder::FlushDecoderBuffers(StreamSelector stream)
{
    AVCodecContext *ctx = nullptr;
    if (stream == StreamSelector::kAudio_StreamType && has_audio_stream_)
        ctx = decoder_priv_->acodec_ctx_;
    else if (stream == StreamSelector::kVideo_StreamType && has_video_stream_)
        ctx = decoder_priv_->vcodec_ctx_;
    if (!ctx)
        return false;

    avcodec_flush_buffers(ctx);
    return true;
}

AVStreamDecoder::AVGenericDecoded AVStreamDecoder::DecodeNextFrame()
{
    TRACE_EVENT("multimedia", "AVStreamDecoder::DecodeNextFrame");

    if (!decoder_priv_->packet_)
    {
        decoder_priv_->packet_ = av_packet_alloc();
        CHECK(decoder_priv_->packet_ && "Failed to allocate memory");
    }

    if (!decoder_priv_->current_frame_)
    {
        decoder_priv_->current_frame_ = av_frame_alloc();
        CHECK(decoder_priv_->current_frame_ && "Failed to allocate memory");
    }

    AVPacket *packet = decoder_priv_->packet_;
    AVFrame *frame = decoder_priv_->current_frame_;
    bool is_video_frame = false;
    while (true)
    {
        int ret = av_read_frame(decoder_priv_->format_ctx_, packet);
        if (ret == AVERROR_EOF)
        {
            return AVGenericDecoded(AVGenericDecoded::kEOF);
        }
        else if (ret < 0)
        {
            QLOG(LOG_ERROR, "Failed to read next frame packet: {}", get_averror_str(ret));
            return AVGenericDecoded(AVGenericDecoded::kNull);
        }

        ScopeExitAutoInvoker packet_unref([packet] { av_packet_unref(packet); });

        AVCodecContext *codec_ctx;
        if (packet->stream_index == decoder_priv_->audio_stream_idx_ &&
            decoder_priv_->acodec_ctx_)
        {
            is_video_frame = false;
            codec_ctx = decoder_priv_->acodec_ctx_;
        }
        else if (packet->stream_index == decoder_priv_->video_stream_idx_ &&
                 decoder_priv_->vcodec_ctx_)
        {
            is_video_frame = true;
            codec_ctx = decoder_priv_->vcodec_ctx_;
        }
        else
        {
            // Invalid media type (disabled by decoder options or not supported)
            // will be skipped.
            continue;
        }

        ret = avcodec_send_packet(codec_ctx, packet);
        if (ret < 0)
        {
            QLOG(LOG_ERROR, "Failed to send decoder frame packet: {}", get_averror_str(ret));
            return AVGenericDecoded(AVGenericDecoded::kNull);
        }

        ret = avcodec_receive_frame(codec_ctx, frame);
        if (ret == AVERROR(EAGAIN))
            continue;
        else if (ret < 0)
        {
            QLOG(LOG_ERROR, "Failed to decode frame: {}", get_averror_str(ret));
            return AVGenericDecoded(AVGenericDecoded::kNull);
        }

        break;
    }

    ScopeExitAutoInvoker frame_unref([frame] { av_frame_unref(frame); });

    // Wrap the decoded frame into a certain buffer
    AVGenericDecoded decoded(AVGenericDecoded::kNull);
    if (is_video_frame)
    {
        decoded.type = AVGenericDecoded::kVideo;
        decoded.video = VideoBuffer::MakeFromAVFrame(frame);
    }
    else
    {
        decoded.type = AVGenericDecoded::kAudio;
        decoded.audio = AudioBuffer::MakeFromAVFrame(frame);
    }

    if (!decoded.audio && !decoded.video && decoded.type != AVGenericDecoded::kNull)
    {
        QLOG(LOG_ERROR, "Failed in reading buffers in frame");
        decoded.type = AVGenericDecoded::kNull;
    }

    return decoded;
}

std::optional<AVStreamDecoder::StreamInfo>
AVStreamDecoder::GetStreamInfo(StreamSelector selector)
{
    AVFormatContext *fmt_ctx = decoder_priv_->format_ctx_;
    AVStream *st = nullptr;
    if (selector == kVideo_StreamType && has_video_stream_)
    {
        st = fmt_ctx->streams[decoder_priv_->video_stream_idx_];
        CHECK(st);
    }
    else if (selector == kAudio_StreamType && has_audio_stream_)
    {
        st = fmt_ctx->streams[decoder_priv_->audio_stream_idx_];
        CHECK(st);
    }

    if (!st)
        return {};

    StreamInfo info{};

    info.time_base = Ratio(st->time_base.num, st->time_base.den);
    info.duration = st->duration;

    if (st->metadata)
    {
        AVDictionaryEntry *entry = nullptr;
        while ((entry = av_dict_get(st->metadata, "", entry, AV_DICT_IGNORE_SUFFIX)))
            info.metadata[entry->key] = entry->value;
    }

    if (selector == kAudio_StreamType)
    {
        info.sample_rate = st->codecpar->sample_rate;
        if (st->codecpar->ch_layout.nb_channels == 1)
            info.channel_mode = AudioChannelMode::kMono;
        else if (st->codecpar->ch_layout.nb_channels == 2)
            info.channel_mode = AudioChannelMode::kStereo;
        else
            info.channel_mode = AudioChannelMode::kUnknown;

        info.sample_fmt = LibavFormatToSampleFormat(
                static_cast<AVSampleFormat>(st->codecpar->format));
    }
    else
    {
        info.pixel_fmt = static_cast<AVPixelFormat>(st->codecpar->format);
        info.width = st->codecpar->width;
        info.height = st->codecpar->height;
        info.sar = Ratio(st->sample_aspect_ratio.num, st->sample_aspect_ratio.den);
    }

    return info;
}

UTAU_NAMESPACE_END
