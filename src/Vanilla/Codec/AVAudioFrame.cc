extern "C" {
#include <libswresample/swresample.h>
}

#include "Vanilla/Base.h"
#include "Vanilla/Codec/AVFrame.h"
#include "Vanilla/Codec/AVFramePrivate.h"
VANILLA_NS_BEGIN

AVAudioFrame::AVAudioFrame(AVFramePrivate *pData)
    : AVFrame(pData, FrameType::kAudio)
{
    ::AVSampleFormat sampleFormat;
}

AVVideoFrame::AVVideoFrame(AVFramePrivate *pData)
    : AVFrame(pData, FrameType::kVideo)
{
}

VANILLA_NS_END
