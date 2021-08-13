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

namespace {

::AVSampleFormat sample_spec_to_avformat(const AuSampleSpec& spec)
{
    switch (spec.format)
    {
    case AuSampleSpec::Format::kPCM:
        if (spec.bitsPerSample == 8)
            return AV_SAMPLE_FMT_U8;
        else if (spec.bitsPerSample == 16)
            return AV_SAMPLE_FMT_S16;
        else
            return AV_SAMPLE_FMT_NONE;
    case AuSampleSpec::Format::kALAW:
    case AuSampleSpec::Format::kULAW:
        return AV_SAMPLE_FMT_NONE;
    }
}

} // namespace anonymous

std::tuple<size_t, size_t, uint8_t*> AVAudioFrame::resample(const AuSampleSpec& spec)
{
    ::AVSampleFormat outFormat = sample_spec_to_avformat(spec);
    if (outFormat == AV_SAMPLE_FMT_NONE)
        return {0, 0, nullptr};

    ::SwrContext *context = ::swr_alloc();
    ::swr_alloc_set_opts(context,
                         static_cast<int64_t>(fData->pFrame->channel_layout),
                         outFormat,
                         spec.samplesPerSec,
                         static_cast<int64_t>(fData->pFrame->channel_layout),
                         static_cast<::AVSampleFormat>(fData->pFrame->format),
                         fData->pFrame->sample_rate,
                         0,
                         nullptr);
    ::swr_init(context);

    ScopeEpilogue epilogue([&context]() -> void {
        ::swr_free(&context);
    });

    /*dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, src_rate) +
                                        src_nb_samples, dst_rate, src_rate, AV_ROUND_UP);*/

    int srcNbSamples = fData->pFrame->nb_samples;
    int dstNbSamples = ::av_rescale_rnd(::swr_get_delay(context, fData->pFrame->sample_rate) + srcNbSamples,
                                        spec.samplesPerSec, fData->pFrame->sample_rate, AV_ROUND_UP);
    uint8_t *pBuffer;
    int linesize;
    int ret = ::av_samples_alloc(&pBuffer, &linesize, spec.channels, dstNbSamples, outFormat, 0);
    if (ret < 0)
        return {0, 0, nullptr};

    ret = ::swr_convert(context,
                        &pBuffer,
                        dstNbSamples,
                        const_cast<const uint8_t **>(fData->pFrame->data),
                        srcNbSamples);
    if (ret < 0)
    {
        ::av_free(pBuffer);
        return {0, 0, nullptr};
    }

    size_t dstBufferSize = ::av_samples_get_buffer_size(&linesize, spec.channels, ret, outFormat, 1);
    return {linesize, dstBufferSize, pBuffer};
}

void AVAudioFrame::freeResampleData(uint8_t *data)
{
    ::av_free(data);
}

VANILLA_NS_END
