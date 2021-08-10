#include <endian.h>

#include <type_traits>
#include <concepts>

extern "C" {
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
}

#include "libyuv.h"
#include "include/core/SkImage.h"
#include "include/core/SkBitmap.h"

#include "Vanilla/Base.h"
#include "Vanilla/Codec/AVFrame.h"
#include "Vanilla/Codec/AVFramePrivate.h"
VANILLA_NS_BEGIN

namespace {
template<std::integral T>
inline double avq_mul(const AVRational& r, T v)
{
    return double(v) * double(r.num) / double(r.den);
}

struct
{
    ::AVPixelFormat     avFormat;
    SkColorType         skType;
} const gPixFormatMap[] = {
    { AV_PIX_FMT_RGB8,      SkColorType::kUnknown_SkColorType   },
    { AV_PIX_FMT_RGB444,    SkColorType::kUnknown_SkColorType   },
    { AV_PIX_FMT_RGB555,    SkColorType::kUnknown_SkColorType   },
    { AV_PIX_FMT_BGR555,    SkColorType::kUnknown_SkColorType   },
    { AV_PIX_FMT_RGB565,    SkColorType::kUnknown_SkColorType   },
    { AV_PIX_FMT_BGR565,    SkColorType::kUnknown_SkColorType   },
    { AV_PIX_FMT_RGB24,     SkColorType::kUnknown_SkColorType   },
    { AV_PIX_FMT_BGR24,     SkColorType::kUnknown_SkColorType   },
    { AV_PIX_FMT_0RGB,      SkColorType::kUnknown_SkColorType   },
    { AV_PIX_FMT_RGB0,      SkColorType::kRGB_888x_SkColorType  },
    { AV_PIX_FMT_0BGR,      SkColorType::kUnknown_SkColorType   },
    { AV_PIX_FMT_BGR0,      SkColorType::kUnknown_SkColorType   },
    { AV_PIX_FMT_ARGB,      SkColorType::kUnknown_SkColorType   },
    { AV_PIX_FMT_RGBA,      SkColorType::kRGBA_8888_SkColorType },
    { AV_PIX_FMT_ABGR,      SkColorType::kUnknown_SkColorType   },
    { AV_PIX_FMT_BGRA,      SkColorType::kBGRA_8888_SkColorType },
    { AV_PIX_FMT_YUV420P,   SkColorType::kUnknown_SkColorType   },
    { AV_PIX_FMT_YUYV422,   SkColorType::kUnknown_SkColorType   },
    { AV_PIX_FMT_UYVY422,   SkColorType::kUnknown_SkColorType   }
};

bool ConvertAndScale(::AVFrame *pFrame, uint8_t *dstBuffer,
                     int32_t dstWidth, int32_t dstHeight,
                     ::AVPixelFormat srcFmt, SkColorType dstFmt,
                     ::AVPixelFormat dstAVFmt)
{
    int32_t srcWidth = pFrame->width;
    int32_t srcHeight = pFrame->height;

    uint8_t *srcPixels[3] = {pFrame->data[0], pFrame->data[1], pFrame->data[2]};
    int32_t srcStride[3] = {pFrame->linesize[0], pFrame->linesize[1], pFrame->linesize[2]};
    bool pixelsReallocated = false;
    const size_t dstBufferSize = dstWidth * dstHeight * 4;

    if (srcWidth != dstWidth || srcHeight != dstHeight)
    {
        switch (srcFmt)
        {
        case AV_PIX_FMT_ARGB:
        case AV_PIX_FMT_0RGB:
        case AV_PIX_FMT_RGB0:
        case AV_PIX_FMT_RGBA:
        case AV_PIX_FMT_BGR0:
        case AV_PIX_FMT_BGRA:
        case AV_PIX_FMT_ABGR:
        case AV_PIX_FMT_0BGR:
            srcPixels[0] = static_cast<uint8_t*>(std::malloc(dstBufferSize));
            srcPixels[1] = srcPixels[2] = nullptr;
            srcStride[0] = dstWidth * 4;
            pixelsReallocated = true;
            libyuv::ARGBScale(pFrame->data[0],
                              pFrame->linesize[0],
                              srcWidth,
                              srcHeight,
                              srcPixels[0],
                              srcStride[0],
                              dstWidth,
                              dstHeight,
                              libyuv::FilterMode::kFilterBilinear);
            break;

        case AV_PIX_FMT_YUV420P:
            srcStride[0] = dstWidth;
            srcStride[1] = srcStride[2] = dstWidth >> 1;
            srcPixels[0] = static_cast<uint8_t*>(std::malloc(srcStride[0] * dstHeight));
            srcPixels[1] = static_cast<uint8_t*>(std::malloc(srcStride[1] * dstHeight));
            srcPixels[2] = static_cast<uint8_t*>(std::malloc(srcStride[2] * dstHeight));
            pixelsReallocated = true;
            libyuv::I420Scale(pFrame->data[0],
                              pFrame->linesize[0],
                              pFrame->data[1],
                              pFrame->linesize[1],
                              pFrame->data[2],
                              pFrame->linesize[2],
                              srcWidth,
                              srcHeight,
                              srcPixels[0],
                              srcStride[0],
                              srcPixels[1],
                              srcStride[1],
                              srcPixels[2],
                              srcStride[2],
                              dstWidth,
                              dstHeight,
                              libyuv::FilterMode::kFilterBilinear);
            break;

        default:
        {
            /* libswscale can scale and convert a picture in one step */
            ::SwsContext *sws = ::sws_getContext(srcWidth,
                                                 srcHeight,
                                                 srcFmt,
                                                 dstWidth,
                                                 dstHeight,
                                                 dstAVFmt,
                                                 SWS_FAST_BILINEAR,
                                                 nullptr,
                                                 nullptr,
                                                 nullptr);

            uint8_t *dst[] = {dstBuffer, nullptr};
            int32_t dstStride[] = {dstWidth * 4, 0};
            ::sws_scale(sws,
                        pFrame->data,
                        pFrame->linesize,
                        0,
                        srcHeight,
                        dst,
                        dstStride);

            ::sws_freeContext(sws);
            return true;
        }
        }
    }

    ScopeEpilogue epilogue([srcPixels, pixelsReallocated]() -> void {
        if (!pixelsReallocated)
            return;
        for (auto const ptr : srcPixels)
        {
            if (ptr)
                std::free(ptr);
        }
    });

    if (srcFmt == dstAVFmt)
    {
        std::memcpy(dstBuffer, srcPixels[0], dstBufferSize);
        return true;
    }

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define LIBYUV_ARGB_TO_BGRA     libyuv::BGRAToARGB
#define LIBYUV_YUV420P_TO_BGRA  libyuv::I420ToARGB
#else
#define LIBYUV_ARGB_TO_BGRA     libyuv::ARGBToBGRA
#define LIBYUV_YUV420P_TO_BGRA  libyuv::I420ToBGRA
#endif

    /* Optimization through Google's libyuv */
    switch (srcFmt)
    {
    case AV_PIX_FMT_ARGB:
    case AV_PIX_FMT_0RGB:
        switch (dstFmt)
        {
        case SkColorType::kBGRA_8888_SkColorType:
            LIBYUV_ARGB_TO_BGRA(srcPixels[0],
                                srcStride[0],
                                dstBuffer,
                                dstWidth * 4,
                                dstWidth,
                                dstHeight);
            return true;
        case SkColorType::kRGBA_8888_SkColorType:
        case SkColorType::kRGB_888x_SkColorType:
            /* Will be handled by libswscale later */
            break;
        default:
            return false;
        }
        break;

    case AV_PIX_FMT_YUV420P:
        switch (dstFmt)
        {
        case SkColorType::kBGRA_8888_SkColorType:
            LIBYUV_YUV420P_TO_BGRA(srcPixels[0],
                                   srcStride[0],
                                   srcPixels[1],
                                   srcStride[1],
                                   srcPixels[2],
                                   srcStride[2],
                                   dstBuffer,
                                   dstWidth * 4,
                                   dstWidth,
                                   dstHeight);
            return true;
        case SkColorType::kRGBA_8888_SkColorType:
        case SkColorType::kRGB_888x_SkColorType:
            /* Will be handled by libswscale later */
            break;
        default:
            return false;
        }
        break;

    default:
        /* Will be handled by libswscale later */
        break;
	}
    

    ::SwsContext *sws = ::sws_getContext(srcWidth,
                                         srcHeight,
                                         srcFmt,
                                         dstWidth,
                                         dstHeight,
                                         dstAVFmt,
                                         SWS_FAST_BILINEAR,
                                         nullptr,
                                         nullptr,
                                         nullptr);

    uint8_t *dst[] = {dstBuffer, nullptr};
    int32_t dstStride[] = {dstWidth * 4, 0};
    ::sws_scale(sws,
                pFrame->data,
                pFrame->linesize,
                0,
                srcHeight,
                dst,
                dstStride);

    ::sws_freeContext(sws);
    return true;
}

} // namespace anonymous

AVFrame::AVFrame(AVFramePrivate *pData, FrameType type)
    : fType(type),
      fData(pData)
{
}

AVFrame::~AVFrame()
{
    ::av_frame_free(&fData->pFrame);
    delete fData;
}

double AVFrame::presentTime()
{
    return avq_mul(fData->timeBase, fData->pFrame->pts);
}

int32_t AVVideoFrame::width() const
{
    return fData->pFrame->width;
}

int32_t AVVideoFrame::height() const
{
    return fData->pFrame->height;
}

SkBitmap AVVideoFrame::asBitmap(const SkImageInfo& info)
{
    int32_t dstWidth = info.width();
    int32_t dstHeight = info.height();
    SkColorType dstColorType = info.colorType();

    auto srcFormat = static_cast<::AVPixelFormat>(fData->pFrame->format);
    ::AVPixelFormat dstFormat;
    bool found = false, foundDst = false;

    for (const auto& pair : gPixFormatMap)
    {
        if (pair.avFormat == srcFormat)
            found = true;
        if (pair.skType == dstColorType &&
            pair.skType != SkColorType::kUnknown_SkColorType)
        {
            dstFormat = pair.avFormat;
            foundDst = true;
        }
    }
    if (!found || !foundDst)
        return {};

    SkBitmap bitmap;
    bitmap.setInfo(info, info.minRowBytes());
    bitmap.allocPixels();

    bool result = ConvertAndScale(fData->pFrame,
                                  static_cast<uint8_t*>(bitmap.getAddr(0, 0)),
                                  dstWidth,
                                  dstHeight,
                                  srcFormat,
                                  dstColorType,
                                  dstFormat);
    if (!result)
        return {};
    return bitmap;
}

VANILLA_NS_END
