#ifndef COCOA_AVFRAME_H
#define COCOA_AVFRAME_H

#include <tuple>

#include "include/core/SkRefCnt.h"
#include "include/core/SkBitmap.h"
#include "Vanilla/Base.h"
#include "Vanilla/Audio/AuCommon.h"
VANILLA_NS_BEGIN

class AVFrame
{
public:
    struct AVFramePrivate;
    enum class FrameType
    {
        kVideo,
        kAudio
    };

    explicit AVFrame(AVFramePrivate *pData, FrameType type);
    virtual ~AVFrame();

    va_nodiscard inline FrameType getType()
    { return fType; }

    double presentTime();

protected:
    FrameType            fType;
    AVFramePrivate      *fData;
};

class AVAudioFrame : public AVFrame
{
public:
    static Handle<AVAudioFrame> Cast(const Handle<AVFrame>& ptr)
    { return std::dynamic_pointer_cast<AVAudioFrame>(ptr); }

    explicit AVAudioFrame(AVFramePrivate *pData);
    ~AVAudioFrame() override = default;

    /* total (aligned) size, valid size, pointer to buffer */
    std::tuple<size_t, size_t, uint8_t*> resample(const AuSampleSpec& spec);
    void freeResampleData(uint8_t *data);
};

class AVVideoFrame : public AVFrame
{
public:
    static Handle<AVVideoFrame> Cast(const Handle<AVFrame>& ptr)
    { return std::dynamic_pointer_cast<AVVideoFrame>(ptr); }

    explicit AVVideoFrame(AVFramePrivate *pData);
    ~AVVideoFrame() override = default;

    va_nodiscard int32_t width() const;
    va_nodiscard int32_t height() const;

    /**
     * Convert this frame to Skia image.
     * Pixels in the frame may be scaled and converted into the color format
     * that @a info specified.
     * Scaling and converting is implemented by software with SIMD
     * optimization.
     *
     * @param info   Specify dimensions and SkColorType.
     *               Only kBGRA_8888_SkColorType, kRGBA_8888_SkColorType and kRGB_888x_SkColorType works.
     *               SkAlphaType will be ignored and the alpha channel is always unpremultiplied.
     * @return       SkBitmap which contains this frame.
     */
    SkBitmap asBitmap(const SkImageInfo& info);
};

VANILLA_NS_END
#endif //COCOA_AVFRAME_H
