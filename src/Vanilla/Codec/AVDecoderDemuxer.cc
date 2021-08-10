#include <iostream>
#include <atomic>
#include <cassert>
#include <map>
#include <memory>
#include <optional>
#include <tuple>

extern "C" {
#include <libavformat/avio.h>
#include <libavformat/avformat.h>
}

#include "Core/Properties.h"
#include "Core/Journal.h"
#include "Vanilla/Base.h"
#include "Vanilla/Codec/AVDecoder.h"
#include "Vanilla/Codec/AVDecoderPrivate.h"
VANILLA_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Vanilla.Codec)

namespace {

std::atomic<bool> gLibAVInitialized(false);
thread_local char gAVLogFormatBuffer[4096];

void AVDefaultLogCallback(void *avcl, int level, const char *fmt, va_list arg)
{
    if (level > AV_LOG_DEBUG)
        return;

    LogType type;
    switch (level)
    {
    case AV_LOG_DEBUG:
        type = LOG_DEBUG;
        break;
    case AV_LOG_INFO:
        type = LOG_INFO;
        break;
    case AV_LOG_WARNING:
        type = LOG_WARNING;
        break;
    case AV_LOG_ERROR:
    case AV_LOG_FATAL:
    case AV_LOG_PANIC:
        type = LOG_ERROR;
        break;
    default:
        type = LOG_DEBUG;
        break;
    }

    AVClass *_class = avcl ? *reinterpret_cast<AVClass**>(avcl) : nullptr;
    std::string prefix;
    if (_class)
        prefix = fmt::format("%fg<cy,hl>({})%reset ", _class->item_name(avcl));
    int len = vsnprintf(gAVLogFormatBuffer, 4096, fmt, arg);
    if (len > 0 && gAVLogFormatBuffer[len - 1] == '\n')
        gAVLogFormatBuffer[len - 1] = '\0';
    LOGF(type, "{}{}", prefix, gAVLogFormatBuffer)
}

void InitializeCodecs()
{
    if (gLibAVInitialized)
        return;
    ::av_log_set_callback(AVDefaultLogCallback);
    gLibAVInitialized = true;
}

int AVIOReadPacketCallback(void *opaque, uint8_t *buf, int size)
{
    auto *pInfo = reinterpret_cast<AVIOCallbackInfo*>(opaque);
    ssize_t readSize = pInfo->data->read(buf, size);
    if (readSize < 0)
        return -1;
    else if (readSize == 0)
        return AVERROR_EOF;
    return static_cast<int>(readSize);
}

int64_t AVIOSeekCallback(void *opaque, int64_t offset, int whence)
{
    auto *pInfo = reinterpret_cast<AVIOCallbackInfo*>(opaque);
    vfs::SeekWhence dir;

    switch (whence)
    {
    case AVSEEK_SIZE:
        return static_cast<int64_t>(pInfo->data->size());
    case SEEK_SET:
        dir = vfs::SeekWhence::kSet;
        break;
    case SEEK_CUR:
        dir = vfs::SeekWhence::kCurrent;
        break;
    case SEEK_END:
        dir = vfs::SeekWhence::kEnd;
        break;
    default:
        return -1;
    }
    return pInfo->data->seek(dir, offset);
}

std::string AVGetErrorString(int err)
{
    std::string buffer;
    buffer.resize(1024);
    ::av_make_error_string(buffer.data(), 1024, err);
    return buffer;
}

AVStreamType AVMediaTypeToStreamTypeEnumClass(AVMediaType type)
{
    switch (type)
    {
    case AVMEDIA_TYPE_AUDIO:
        return AVStreamType::kAudio;
    case AVMEDIA_TYPE_VIDEO:
        return AVStreamType::kVideo;
    case AVMEDIA_TYPE_SUBTITLE:
        return AVStreamType::kSubtitle;
    default:
        return AVStreamType::kUnknown;
    }
}

} // namespace anonymous

void AVStreamSelector::setAudio(const std::string& specifier)
{
    fAudio = specifier;
}

void AVStreamSelector::setVideo(const std::string& specifier)
{
    fVideo = specifier;
}

void AVStreamSelector::setSubtitle(const std::string& subtitle)
{
    fSubtitle = subtitle;
}

const std::string & AVStreamSelector::getSpecifier(AVStreamType type) const
{
    switch (type)
    {
    case AVStreamType::kAudio:
        return fAudio;
    case AVStreamType::kVideo:
        return fVideo;
    case AVStreamType::kSubtitle:
        return fSubtitle;
    case AVStreamType::kUnknown:
        throw RuntimeException(__func__, "Unknown stream type");
    }
}

namespace {

std::optional<std::tuple<::AVCodecContext*, ::AVCodec*>> AVStreamOpenComponent(::AVStream *pStream)
{
    ::AVCodec *codec = ::avcodec_find_decoder(pStream->codecpar->codec_id);
    if (!codec)
        return {};

    ::AVCodecContext *codecCtx = ::avcodec_alloc_context3(codec);
    if (codecCtx == nullptr)
        return {};

    int32_t ret = ::avcodec_parameters_to_context(codecCtx, pStream->codecpar);
    if (ret < 0)
    {
        ::avcodec_free_context(&codecCtx);
        return {};
    }

    if (::avcodec_open2(codecCtx, codec, nullptr) < 0)
    {
        ::avcodec_free_context(&codecCtx);
        return {};
    }

    using Tuple = std::tuple<::AVCodecContext*, ::AVCodec*>;
    return std::make_optional<Tuple>({codecCtx, codec});
}

} // namespace anonymous

Handle<AVDecoder> AVDecoder::MakeFromStream(std::string name,
                                            const std::shared_ptr<Data>& data,
                                            const AVStreamSelector& selector)
{
    InitializeCodecs();

    auto avIOCallbackInfo = std::make_unique<AVIOCallbackInfo>(AVIOCallbackInfo{data, data->size()});

    size_t bufferSize;
    try {
        bufferSize = prop::Cast<PropertyDataNode>(
                prop::Get()->next("system")->next("mem-page-size"))->extract<size_t>();
    } catch (const std::exception& e) {
        bufferSize = 4096;
    }
    auto *ioBuffer = reinterpret_cast<uint8_t*>(::av_malloc(bufferSize));
    if (!ioBuffer)
        return nullptr;

    ::AVIOContext *avioCtx = ::avio_alloc_context(ioBuffer, static_cast<int>(bufferSize), 0,
                                                  avIOCallbackInfo.get(),
                                                  AVIOReadPacketCallback,
                                                  nullptr,
                                                  AVIOSeekCallback);
    if (!avioCtx)
        return nullptr;

    ::AVFormatContext *avFormatCtx = ::avformat_alloc_context();
    if (!avFormatCtx)
    {
        ::avio_context_free(&avioCtx);
        ::av_free(ioBuffer);
        return nullptr;
    }
    avFormatCtx->pb = avioCtx;
    avFormatCtx->flags |= AVFMT_FLAG_CUSTOM_IO;

    int err = ::avformat_open_input(&avFormatCtx, name.c_str(), nullptr, nullptr);
    if (err < 0)
    {
        LOGF(LOG_ERROR, "Failed to create a libavformat input: {}", AVGetErrorString(err))
        ::avformat_close_input(&avFormatCtx);
        return nullptr;
    }

    err = ::avformat_find_stream_info(avFormatCtx, nullptr);
    if (err < 0)
    {
        LOGF(LOG_ERROR, "Failed to find stream info in {}: {}", name, AVGetErrorString(err))
        ::avformat_close_input(&avFormatCtx);
        return nullptr;
    }

    /* Find proper streams and construct stream index map */
    std::map<AVStreamType, int32_t> streamIdxMap;
    streamIdxMap[AVStreamType::kAudio] = -1;
    streamIdxMap[AVStreamType::kVideo] = -1;
    streamIdxMap[AVStreamType::kSubtitle] = -1;

    for (int32_t i = 0; i < avFormatCtx->nb_streams; i++)
    {
        AVStream *stream = avFormatCtx->streams[i];
        AVStreamType type = AVMediaTypeToStreamTypeEnumClass(stream->codecpar->codec_type);
        if (type == AVStreamType::kUnknown ||
            selector.getSpecifier(type).empty() ||
            streamIdxMap[type] >= 0)
            continue;
        if (::avformat_match_stream_specifier(avFormatCtx, stream, selector.getSpecifier(type).c_str()) > 0)
            streamIdxMap[type] = i;
    }

    for (auto type : {AVStreamType::kAudio, AVStreamType::kVideo, AVStreamType::kSubtitle})
    {
        if (!selector.getSpecifier(type).empty() && streamIdxMap[type] < 0)
            streamIdxMap[type] = INT_MAX;
    }

    streamIdxMap[AVStreamType::kVideo] = ::av_find_best_stream(avFormatCtx,
                                                             AVMEDIA_TYPE_VIDEO,
                                                             streamIdxMap[AVStreamType::kVideo],
                                                             -1, nullptr, 0);
    streamIdxMap[AVStreamType::kAudio] = ::av_find_best_stream(avFormatCtx,
                                                             AVMEDIA_TYPE_AUDIO,
                                                             streamIdxMap[AVStreamType::kAudio],
                                                             streamIdxMap[AVStreamType::kVideo],
                                                             nullptr, 0);
    streamIdxMap[AVStreamType::kSubtitle] = ::av_find_best_stream(avFormatCtx,
                                                                AVMEDIA_TYPE_SUBTITLE,
                                                                streamIdxMap[AVStreamType::kSubtitle],
                                                                (streamIdxMap[AVStreamType::kAudio] >= 0 ?
                                                                 streamIdxMap[AVStreamType::kAudio] :
                                                                 streamIdxMap[AVStreamType::kVideo]),
                                                                nullptr, 0);

    std::map<AVStreamType, DecoderPrivate::StreamWithCodec> streamCodecMap;
    for (const auto& p : streamIdxMap)
    {
        if (p.second < 0)
            continue;
        ::AVStream *stream = avFormatCtx->streams[p.second];

        auto maybeCodec = AVStreamOpenComponent(stream);
        if (!maybeCodec)
        {
            LOGF(LOG_DEBUG, "Failed to open a codec for stream #{}", stream->index)
            continue;
        }
        auto [codecCtx, codec] = maybeCodec.value();
        streamCodecMap[p.first] = {
                .stream = stream,
                .codecContext = codecCtx,
                .codec = codec
        };
    }

    /* Demuxing is successfully finished. */
    auto fields = new DecoderPrivate{std::move(avIOCallbackInfo),
                                     ioBuffer,
                                     avFormatCtx,
                                     std::move(streamCodecMap),
                                     ::av_packet_alloc(),
                                     ::av_frame_alloc()};
    return std::make_shared<AVDecoder>(std::move(name), fields);
}

AVDecoder::AVDecoder(std::string name, DecoderPrivate *pData)
    : fName(std::move(name)),
      fData(pData)
{
    assert(fData != nullptr);
}

AVDecoder::~AVDecoder()
{
    ::av_frame_free(&fData->frame);
    ::av_packet_free(&fData->packet);
    for (auto& p : fData->streams)
        ::avcodec_free_context(&p.second.codecContext);
    ::avformat_close_input(&fData->avFormatContext);
    delete fData;
}

int32_t AVDecoder::getStreamCount()
{
    return static_cast<int32_t>(fData->streams.size());
}

bool AVDecoder::hasStream(AVStreamType type)
{
    for (const auto& pair : fData->streams)
    {
        if (pair.first == type)
            return true;
    }
    return false;
}

VANILLA_NS_END
