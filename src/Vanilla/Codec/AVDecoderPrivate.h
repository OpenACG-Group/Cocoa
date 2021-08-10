#ifndef COCOA_AVDECODERPRIVATE_H
#define COCOA_AVDECODERPRIVATE_H

#include <map>
#include <memory>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include "Vanilla/Base.h"
#include "Vanilla/Codec/AVDecoder.h"
VANILLA_NS_BEGIN

struct AVIOCallbackInfo
{
    std::shared_ptr<Data>   data;
    size_t                  dataSize;
};

struct AVDecoder::DecoderPrivate
{
    struct StreamWithCodec
    {
        ::AVStream          *stream;
        ::AVCodecContext    *codecContext;
        ::AVCodec           *codec;
    };

    va_maybe_unused
    std::unique_ptr<AVIOCallbackInfo>  avIOCallbackInfo;
    va_maybe_unused
    uint8_t                           *avIOInputBuffer;
    ::AVFormatContext                 *avFormatContext;
    std::map<AVStreamType, StreamWithCodec> streams;
    ::AVPacket                        *packet;
    ::AVFrame                         *frame;
};

VANILLA_NS_END
#endif //COCOA_AVDECODERPRIVATE_H
