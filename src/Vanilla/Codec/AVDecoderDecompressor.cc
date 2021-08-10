#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include "Vanilla/Base.h"
#include "Vanilla/Codec/AVDecoder.h"
#include "Vanilla/Codec/AVDecoderPrivate.h"
#include "Vanilla/Codec/AVFramePrivate.h"
VANILLA_NS_BEGIN

namespace {

bool AVDecodePacketToFrame(::AVPacket *packet, ::AVCodecContext *context, ::AVFrame *frame,
                           AVStreamType type, std::vector<Handle<AVFrame>>& out)
{
    int response = ::avcodec_send_packet(context, packet);
    if (response < 0)
        return false;

    while (response >= 0)
    {
        response = ::avcodec_receive_frame(context, frame);
        if (response == AVERROR_EOF || response == AVERROR(EAGAIN))
            break;
        else if (response < 0)
            return false;

        // TODO(sora): Memory pool (zone allocator) for better performance
        //             and less memory pressure.
        ::AVFrame *dumpFrame = ::av_frame_alloc();
        ::av_frame_move_ref(dumpFrame, frame);

        auto data = new AVFrame::AVFramePrivate{
            context->time_base,
            dumpFrame
        };

        Handle<AVFrame> handle;
        if (type == AVStreamType::kAudio)
        {
            handle = std::make_shared<AVAudioFrame>(data);
        }
        else if (type == AVStreamType::kVideo)
        {
            handle = std::make_shared<AVVideoFrame>(data);
        }
        else
        {
            // TODO(sora): Support subtitle frame
            std::cout << "Unsupported subtitle stream" << std::endl;
            ::av_frame_free(&dumpFrame);
            delete data;
        }
        out.emplace_back(std::move(handle));
    }
    return true;
}

} // namespace anonymous

AVDecoder::ReadingStatus AVDecoder::readFrame(std::vector<Handle<AVFrame>>& out)
{
    ::AVPacket *packet = fData->packet;
    int ret = ::av_read_frame(fData->avFormatContext, packet);
    if (ret < 0)
    {
        if (ret == AVERROR_EOF)
            return ReadingStatus::kEof;
        else
            return ReadingStatus::kError;
    }

    ScopeEpilogue epilogue([packet]() -> void {
        ::av_packet_unref(packet);
    });

    DecoderPrivate::StreamWithCodec *streamCodecPair = nullptr;
    AVStreamType streamType;
    for (auto& stream : fData->streams)
    {
        if (stream.second.stream->index == packet->stream_index)
        {
            streamCodecPair = &stream.second;
            streamType = stream.first;
            break;
        }
    }
    if (!streamCodecPair)
        return ReadingStatus::kAgain;

    if (!AVDecodePacketToFrame(packet, streamCodecPair->codecContext, fData->frame,
                               streamType, out))
        return ReadingStatus::kError;

    return ReadingStatus::kOk;
}

VANILLA_NS_END
