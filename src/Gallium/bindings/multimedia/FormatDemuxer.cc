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

#include "fmt/format.h"

#include "Gallium/bindings/multimedia/FormatDemuxer.h"
#include "Gallium/bindings/multimedia/MediaInputOutput.h"
#include "Gallium/bindings/multimedia/AChannelLayout.h"
#include "Gallium/bindings/multimedia/CodecParameters.h"
#include "Gallium/bindings/multimedia/Packet.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

namespace {

void free_avformat_context(AVFormatContext *ctx)
{
    if (ctx->codec_whitelist)
        av_free(ctx->codec_whitelist);
    if (ctx->format_whitelist)
        av_free(ctx->format_whitelist);
    avformat_free_context(ctx);
}

} // namespace anonymous

ffi::RetLocal<v8::Value> FormatDemuxer::Make(const ffi::Class<MediaIOContext>& context,
                                             const ffi::IFace<DemuxerOptions>& options)
{
    if (context->GetDisposeState() != DisposeState::kNot)
        return ffi::Fail(ffi::kErr, "invalid IO context: object has been disposed");
    if (context->IsLocked())
        return ffi::Fail(ffi::kErr, "IO context has been locked by another retainer");

    // Lock the IO context to occupy it uniquely
    std::shared_ptr<MediaIOContext::ContextOwner> ioctx = context->GetAndLockContext();
    CHECK(ioctx);

    // Allocate and initialize the format context
    AVFormatContext *fmtctx = avformat_alloc_context();
    CHECK(fmtctx && "failed to allocate");
    fmtctx->pb = ioctx->ctx;
    fmtctx->flags |= AVFMT_FLAG_CUSTOM_IO;
    if (options->format_whitelist)
        fmtctx->format_whitelist = av_strdup(options->format_whitelist->c_str());
    if (options->codec_whitelist)
        fmtctx->codec_whitelist = av_strdup(options->codec_whitelist->c_str());

    if (int error = avformat_open_input(&fmtctx, "", nullptr, nullptr))
    {
        // `fmtctx` is freed by `avformat_open_input()` on failure
        return ffi::Fail(ffi::kErr, fmt::format("failed to initialize input: {}", av_err2str(error)));
    }

    // Probe the stream info; this may read some packets
    if (int error = avformat_find_stream_info(fmtctx, nullptr); error < 0)
    {
        free_avformat_context(fmtctx);
        return ffi::Fail(ffi::kErr, fmt::format("failed to probe stream info: {}", av_err2str(error)));
    }

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<FormatDemuxer>(isolate, std::move(ioctx), fmtctx);
}

ffi::Ret<void> FormatDemuxer::dispose()
{
    if (format_ctx_->format_whitelist)
        av_free(format_ctx_->format_whitelist);
    if (format_ctx_->codec_whitelist)
        av_free(format_ctx_->codec_whitelist);

    avformat_close_input(&format_ctx_);
    // Now the IO context can be unlocked
    ioctx_.reset();
    NotifyDisposeState(DisposeState::kDisposed);
    return {};
}

ffi::RetLocal<v8::Value> FormatDemuxer::getFormatInfo()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> jsctx = isolate->GetCurrentContext();

    v8::Local<v8::Map> metadata_map = v8::Map::New(isolate);
    if (format_ctx_->metadata)
    {
        const AVDictionaryEntry *e = nullptr;
        while ((e = av_dict_iterate(format_ctx_->metadata, e)))
        {
            metadata_map->Set(
                jsctx,
                ffi::Cast<const char*>::ToChecked(isolate, e->key),
                ffi::Cast<const char*>::ToChecked(isolate, e->value)
            ).ToLocalChecked();
        }
    }

    return ffi::Cast<ffi::ObjectLiteralMap>::ToChecked(isolate, {
        { "formatName", ffi::Cast<std::string>::ToChecked(isolate, format_ctx_->iformat->name) },
        { "formatLongName", ffi::Cast<std::string>::ToChecked(isolate, format_ctx_->iformat->long_name) },
        { "streamCount", v8::Uint32::NewFromUnsigned(isolate, format_ctx_->nb_streams) },
        { "duration", v8::Number::New(isolate, static_cast<double>(format_ctx_->duration)) },
        { "totalStreamBitRate", v8::Number::New(isolate, static_cast<double>(format_ctx_->bit_rate)) },
        { "metadata", metadata_map }
    });
}

ffi::Ret<int32_t> FormatDemuxer::findBestStream(ffi::Enum<MediaType> type)
{
    int res = av_find_best_stream(
            format_ctx_, static_cast<AVMediaType>(type.GetInteger()), -1, -1, nullptr, 0);
    if (res < 0)
        return -1;
    return res;
}

ffi::RetLocal<v8::Value> FormatDemuxer::matchStreamSpecifier(const std::string& specifier)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    std::vector<v8::Local<v8::Value>> matched;
    for (int32_t st = 0; st < format_ctx_->nb_streams; st++)
    {
        int res = avformat_match_stream_specifier(
                format_ctx_, format_ctx_->streams[st], specifier.c_str());
        if (res < 0)
            return ffi::Fail(ffi::kErr, "invalid stream specifier");

        if (res > 0)
            matched.emplace_back(v8::Int32::New(isolate, st));
    }
    return v8::Array::New(isolate, matched.data(), matched.size());
}

ffi::RetLocal<v8::Value> FormatDemuxer::getStreamInfo(int32_t index)
{
    if (index >= format_ctx_->nb_streams)
        return ffi::Fail(ffi::kRangeErr, "invalid stream index");
    AVStream *st = format_ctx_->streams[index];

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> jsctx = isolate->GetCurrentContext();

    ffi::ObjectLiteralMap res;
    res["index"] = v8::Int32::New(isolate, st->index);
    res["type"] = v8::Uint32::New(isolate, st->codecpar->codec_type);
    res["timeBase"] = CreateJSRational(isolate, st->time_base);
    if (st->start_time != AV_NOPTS_VALUE)
        res["startTime"] = v8::Number::New(isolate, static_cast<double>(st->start_time));
    res["duration"] = v8::Number::New(isolate, static_cast<double>(st->duration));
    res["disposition"] = v8::Uint32::NewFromUnsigned(isolate, st->disposition);

    v8::Local<v8::Map> metadata = v8::Map::New(isolate);
    if (st->metadata)
    {
        const AVDictionaryEntry *e = nullptr;
        while ((e = av_dict_iterate(st->metadata, e)))
        {
            metadata->Set(
                jsctx,
                ffi::Cast<const char*>::ToChecked(isolate, e->key),
                ffi::Cast<const char*>::ToChecked(isolate, e->value)
            ).ToLocalChecked();
        }
    }
    res["metadata"] = metadata;

    AVCodecParameters *codecpar = st->codecpar;
    if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
    {
        ffi::ObjectLiteralMap vinfo;
        if (codecpar->sample_aspect_ratio.num != 0)
            vinfo["SAR"] = CreateJSRational(isolate, codecpar->sample_aspect_ratio);
        vinfo["avgFramerate"] = CreateJSRational(isolate, st->avg_frame_rate);
        vinfo["framerate"] = CreateJSRational(isolate, st->r_frame_rate);
        vinfo["format"] = v8::Uint32::New(isolate, codecpar->format);
        vinfo["bitrate"] = v8::Number::New(isolate, static_cast<double>(codecpar->bit_rate));
        vinfo["width"] = v8::Uint32::New(isolate, codecpar->width);
        vinfo["height"] = v8::Uint32::New(isolate, codecpar->height);
        vinfo["colorRange"] = v8::Uint32::New(isolate, codecpar->color_range);
        vinfo["colorPrimaries"] = v8::Uint32::New(isolate, codecpar->color_primaries);
        vinfo["colorTrc"] = v8::Uint32::New(isolate, codecpar->color_trc);
        vinfo["colorSpace"] = v8::Uint32::New(isolate, codecpar->color_space);
        vinfo["chromaLocation"] = v8::Uint32::New(isolate, codecpar->chroma_location);
        vinfo["delayFrames"] = v8::Int32::New(isolate, codecpar->video_delay);
        res["videoInfo"] = ffi::Cast<ffi::ObjectLiteralMap>::ToChecked(isolate, vinfo);
    }
    else if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
    {
        ffi::ObjectLiteralMap ainfo;
        ainfo["format"] = v8::Uint32::New(isolate, codecpar->format);
        ainfo["channelLayout"] = ffi::JSObject::New<AChannelLayout>(
                isolate, codecpar->ch_layout, AChannelLayout::kNotFreeOriginal);
        ainfo["sampleRate"] = v8::Uint32::New(isolate, codecpar->sample_rate);
        ainfo["initialPadding"] = v8::Uint32::New(isolate, codecpar->initial_padding);
        ainfo["trailingPadding"] = v8::Uint32::New(isolate, codecpar->trailing_padding);
        ainfo["seekPreroll"] = v8::Uint32::New(isolate, codecpar->seek_preroll);
        res["audioInfo"] = ffi::Cast<ffi::ObjectLiteralMap>::ToChecked(isolate, ainfo);
    }

    return ffi::Cast<ffi::ObjectLiteralMap>::ToChecked(isolate, res);
}

ffi::Ret<void> FormatDemuxer::setStreamDiscard(int32_t index, const ffi::Enum<Discard>& discard)
{
    if (index >= format_ctx_->nb_streams)
        return ffi::Fail(ffi::kRangeErr, "invalid stream index");
    AVStream *st = format_ctx_->streams[index];
    st->discard = static_cast<AVDiscard>(discard.GetInteger());
    return {};
}

ffi::RetLocal<v8::Value> FormatDemuxer::getCodecParameters(int32_t index)
{
    if (index >= format_ctx_->nb_streams)
        return ffi::Fail(ffi::kRangeErr, "invalid stream index");
    AVStream *st = format_ctx_->streams[index];

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    AVCodecParameters *params = avcodec_parameters_alloc();
    CHECK(params && "allocation failed");
    avcodec_parameters_copy(params, st->codecpar);
    return ffi::JSObject::New<CodecParameters>(isolate, params);
}

ffi::Ret<void> FormatDemuxer::seekFrame(int32_t index, int64_t timestamp, int flags)
{
    if (int res = av_seek_frame(format_ctx_, index, timestamp, flags); res < 0)
        return ffi::Fail(ffi::kErr, fmt::format("failed to seek frame: {}", av_err2str(res)));
    return {};
}

ffi::Ret<void> FormatDemuxer::readPlay()
{
    if (int res = av_read_play(format_ctx_); res < 0)
        return ffi::Fail(ffi::kErr, fmt::format("failed to start playing: {}", av_err2str(res)));
    return {};
}

ffi::Ret<void> FormatDemuxer::readPause()
{
    if (int res = av_read_pause(format_ctx_); res < 0)
        return ffi::Fail(ffi::kErr, fmt::format("failed to pause: {}", av_err2str(res)));
    return {};
}

ffi::RetLocal<v8::Value> FormatDemuxer::readFrame(ffi::Opt<ffi::Class<Packet>> reuse)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    AVPacket *packet;
    if (reuse)
    {
        const ffi::Class<Packet>& cl = *reuse;
        if (!cl->IsReusable())
            return ffi::Fail(ffi::kErr, "provided packet instance is not reusable");
        packet = cl->GetPacket();
        // we will reset the dispose-state of `reuse` later
    }
    else
    {
        packet = av_packet_alloc();
        CHECK(packet && "allocation failed");
        // we will create `Packet` instance later
    }

    int ret;
    do
    {
        ret = av_read_frame(format_ctx_, packet);
    } while (ret == AVERROR(EAGAIN));
    if (ret < 0 && !reuse)
        av_packet_free(&packet);

    if (ret == AVERROR_EOF)
        return v8::Null(isolate);
    if (ret < 0)
        return ffi::Fail(ffi::kErr, fmt::format("failed to read frame: {}", av_err2str(ret)));

    if (reuse)
    {
        (*reuse)->ResetDisposeState();
        return (*reuse)->GetThisHandle(isolate);
    }

    return ffi::JSObject::New<Packet>(isolate, packet);
}

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
