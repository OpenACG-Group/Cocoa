#include "Ciallo/Ciallo.h"
CIALLO_BEGIN_NS

std::tuple<SkColorType, SkAlphaType> SkColorInfoFromImageFormat(ImageFormat fmt)
{
    switch (fmt)
    {
    case ImageFormat::kRGBA_8888:
        return std::make_tuple(SkColorType::kRGBA_8888_SkColorType,
                               SkAlphaType::kUnpremul_SkAlphaType);

    case ImageFormat::kRGBA_8888_Premultiplied:
        return std::make_tuple(SkColorType::kRGBA_8888_SkColorType,
                               SkAlphaType::kPremul_SkAlphaType);

    case ImageFormat::kBGRA_8888:
        return std::make_tuple(SkColorType::kBGRA_8888_SkColorType,
                               SkAlphaType::kUnpremul_SkAlphaType);

    case ImageFormat::kBGRA_8888_Premultiplied:
        return std::make_tuple(SkColorType::kBGRA_8888_SkColorType,
                               SkAlphaType::kPremul_SkAlphaType);

    case ImageFormat::kGrayscale_8:
        return std::make_tuple(SkColorType::kGray_8_SkColorType,
                               SkAlphaType::kUnknown_SkAlphaType);

    case ImageFormat::kUnknown:
        return std::make_tuple(SkColorType::kUnknown_SkColorType,
                               SkAlphaType::kUnknown_SkAlphaType);
    }
}

CIALLO_END_NS
