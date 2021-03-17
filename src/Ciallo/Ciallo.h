#ifndef __GR_BASE_H__
#define __GR_BASE_H__

#include <tuple>
#include "include/core/SkColor.h"

#include "Core/Exception.h"

#define CIALLO_BEGIN_NS     namespace cocoa::ciallo {
#define CIALLO_END_NS       } /* namespace cocoa::ciallo */

#define RET_NODISCARD   [[nodiscard]]

CIALLO_BEGIN_NS

enum class ImageFormat
{
    kRGBA_8888,
    kRGBA_8888_Premultiplied,
    kBGRA_8888,
    kBGRA_8888_Premultiplied,
    kGrayscale_8,
    kUnknown
};

std::tuple<SkColorType, SkAlphaType> SkColorInfoFromImageFormat(ImageFormat fmt);

CIALLO_END_NS
#endif /* __GR_BASE_H__ */

