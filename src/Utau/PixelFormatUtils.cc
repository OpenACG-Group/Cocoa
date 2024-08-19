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

#include <map>

#include "libyuv.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkYUVAInfo.h"

#include "Core/TraceEvent.h"
#include "Core/Exception.h"
#include "Utau/ffwrappers/libswscale.h"
#include "Utau/ffwrappers/libavutil.h"
#include "Utau/PixelFormatUtils.h"
UTAU_NAMESPACE_BEGIN
namespace {

SkImage::RescaleMode resampler_to_skimage_rescale_mode(ScaleResampler resampler)
{
    switch (resampler)
    {
        case ScaleResampler::kNearest:
            return SkImage::RescaleMode::kNearest;
        case ScaleResampler::kBilinear:
            return SkImage::RescaleMode::kLinear;
        case ScaleResampler::kBicubic:
            return SkImage::RescaleMode::kRepeatedCubic;
    }
    MARK_UNREACHABLE();
}

// FFmpeg Color Primaries <=> Skia (skcms) color matrix.
// `AVColorPrimaries` describes the color primaries standard of a certain colorspace,
// and Skia `skcms_Matrix3x3` matrix describes the transformation from a certain colorspace
// to a common “connection” color space called XYZ D50.
const std::map<AVColorPrimaries, skcms_Matrix3x3> cs_primaries_info{
    // BT709 and sRGB share the same color primaries and white point
    { AVCOL_PRI_BT709, SkNamedGamut::kSRGB },
    { AVCOL_PRI_SMPTE432, SkNamedGamut::kDisplayP3 },
    { AVCOL_PRI_BT2020, SkNamedGamut::kRec2020 },

    // The following matrices can be calculated from the xy coords of the RGB primaries and a
    // reference white point defined by the colorspace, using standard RGB-XYZ conversion algorithm,
    // with Bradford chromatic adaptation transform.
    // An online calculator: https://www.russellcottrell.com/photo/matrixCalculator.htm

    // xy coords of reference white points:
    // Standard white point chromaticities
    // C    0.310063  0.316158
    // E    1.0/3.0   1.0/3.0
    // D50  0.34570   0.3585
    // D65  0.312713  0.329016

    // The following color primaries are defined by coords in CIE XYZ
    // |     Rxy     |     Gxy     |     Bxy     | white point  |

    // SMPTE170M and SMPTE240M
    // | 0.63  0.34  | 0.31  0.595 | 0.155 0.07  |     D65      |
    {
        AVCOL_PRI_SMPTE170M, skcms_Matrix3x3{{
            { 0.4163064f, 0.3931883f, 0.1547053f },
            { 0.2216849f, 0.7032795f, 0.0750356f },
            { 0.0136488f, 0.0913340f, 0.7199173f }
        }}
    },
    {
        AVCOL_PRI_SMPTE240M, skcms_Matrix3x3{{
            { 0.4163064f, 0.3931883f, 0.1547053f },
            { 0.2216849f, 0.7032795f, 0.0750356f },
            { 0.0136488f, 0.0913340f, 0.7199173f }
        }}
    },

    // NTSC 1953 Y'I'O (ITU-R BT.470 System M)
    // | 0.67  0.33  | 0.21  0.71  | 0.14  0.08  |      C       |
    {
        AVCOL_PRI_BT470M, {{
            { 0.6343850f, 0.1852357f, 0.1445793f },
            { 0.3109552f, 0.5916116f, 0.0974332f },
            { -0.0011826f, 0.0555448f, 0.7705378f }
        }}
    },

    // EBU Y'U'V' (PAL/SECAM) (ITU-R BT.470 System B, G), also BT610
    // | 0.64  0.33  | 0.29  0.60  | 0.15  0.06  |     D65      |
    {
        AVCOL_PRI_BT470BG, {{
            { 0.4552593f, 0.3675882f, 0.1413525f },
            { 0.2322908f, 0.7078181f, 0.0598911f },
            { 0.0145362f, 0.1048833f, 0.7054805f }
        }}
    },

    // DCI-P3 / SMPTE ST 431-2 (2011)
    // | 0.68  0.32  | 0.265 0.69  | 0.15  0.06  | 0.314 0.351  |
    {
        AVCOL_PRI_SMPTE431, {{
            { 0.4861607f, 0.3238514f, 0.1541879f },
            { 0.2266839f, 0.7103336f, 0.0629826 },
            { -0.0008016f, 0.0432353f, 0.7824663 }
        }}
    },

    // EBU Tech. 3213-E
    // | 0.64  0.33  | 0.29  0.60  | 0.15  0.06  |     D65      |
    {
        AVCOL_PRI_EBU3213, {{
            { 0.4552593f, 0.3675882f, 0.1413525 },
            { 0.2322908f, 0.7078181f, 0.0598911 },
            { 0.0145362f, 0.1048833f, 0.7054805 }
        }}
    }
};

bool av_color_primaries_to_skcms(AVColorPrimaries prim, skcms_Matrix3x3& mat)
{
    if (!cs_primaries_info.contains(prim))
        return false;
    const skcms_Matrix3x3& entry = cs_primaries_info.at(prim);
    std::memcpy(&mat, &entry, sizeof(skcms_Matrix3x3));
    return true;
}

#define CS_SIMPLE_TRCF(g) \
    { g, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }

// FFmpeg Color Trc <=> Skia (skcms) color transfer function
// This describes how to linearize the gamma-encoded colors.
const std::map<AVColorTransferCharacteristic, skcms_TransferFunction> cs_transfer_map{
    // L = L' / 4.5,                        for 0 <= L' < 0.081
    //   = ((L' + 0.099) / 1.099)^(1/0.45)  for 0.081 <= L'
    { AVCOL_TRC_BT709, { 1/0.45, 1/1.099, 0.099/1.099, 1/4.5, 0.081, 0, 0 } },

    // Using Skia's definition
    { AVCOL_TRC_GAMMA22, SkNamedTransferFn::k2Dot2 },

    // L = L'^(1/2.8)
    { AVCOL_TRC_GAMMA28, CS_SIMPLE_TRCF(1/2.8) },

    // Same to BT709
    { AVCOL_TRC_SMPTE170M, { 1/0.45, 1/1.099, 0.099/1.099, 1/4.5, 0.081, 0, 0 } },

    // L = L' / 4,                              for 0 <= L' < 0.0913
    //   = ((L' + 0.1115) / 1.1115)^(1/0.45),   for 0.0913 >= L'
    { AVCOL_TRC_SMPTE240M, { 1/0.45, 1/1.1115, 0.1115/1.1115, 1/4.0, 0.0913, 0, 0 } },

    // L = L'
    { AVCOL_TRC_LINEAR, SkNamedTransferFn::kLinear },

    // Using Skia's definition
    { AVCOL_TRC_IEC61966_2_1, SkNamedTransferFn::kSRGB },
    { AVCOL_TRC_BT2020_10, SkNamedTransferFn::kRec2020 },
    { AVCOL_TRC_BT2020_12, SkNamedTransferFn::kRec2020 }
};

// FFmpeg Pixel Format <=> Skia Color Type
// Skia has only vestigial support for anything big-endian, and Cocoa
// does not support big-endian well. So only little-endian formats are
// considered.
std::map<AVPixelFormat, std::pair<SkColorType, SkAlphaType>> color_type_map{
    { AV_PIX_FMT_RGB565LE, { kRGB_565_SkColorType, kOpaque_SkAlphaType } },               // fourcc: RGBP / L565
    { AV_PIX_FMT_RGBA, { kRGBA_8888_SkColorType, kUnpremul_SkAlphaType } },               // fourcc: ABGR
    { AV_PIX_FMT_RGB0, { kRGB_888x_SkColorType, kOpaque_SkAlphaType } },                    // fourcc: XBGR
    { AV_PIX_FMT_BGRA, { kBGRA_8888_SkColorType, kUnpremul_SkAlphaType } },               // fourcc: ARGB
    { AV_PIX_FMT_BGR0, { kBGRA_8888_SkColorType, kOpaque_SkAlphaType } },                 // fourcc: XRGB
    { AV_PIX_FMT_X2BGR10LE, { kRGB_101010x_SkColorType, kOpaque_SkAlphaType } },            // fourcc: XB30 (AB30 for alpha)
    { AV_PIX_FMT_X2RGB10LE, { kBGR_101010x_SkColorType, kOpaque_SkAlphaType } },            // fourcc: XR30 (AR30 for alpha)
    { AV_PIX_FMT_RGBAF16LE, { kRGBA_F16_SkColorType, kUnpremul_SkAlphaType } },           // fourcc: ???
    { AV_PIX_FMT_RGBAF32LE, { kRGBA_F32_SkColorType, kUnpremul_SkAlphaType } },           // fourcc: ???
    { AV_PIX_FMT_RGBA64LE, { kR16G16B16A16_unorm_SkColorType, kUnpremul_SkAlphaType } },  // fourcc: AB64
    { AV_PIX_FMT_GRAY8, { kGray_8_SkColorType, kOpaque_SkAlphaType } }                    // fourcc: Y8 / Y800
};

struct ConversionRecord
{
    struct FormatRecord
    {
        SkISize dimensions = {0, 0};
        AVPixelFormat format = AV_PIX_FMT_NONE;
        const AVPixFmtDescriptor *fmtdesc = nullptr;
        AVColorPrimaries primaries = AVCOL_PRI_UNSPECIFIED;
        AVColorTransferCharacteristic trc = AVCOL_TRC_UNSPECIFIED;
        AVColorRange range = AVCOL_RANGE_UNSPECIFIED;
        // keep unspecified for dst, as it cannot be a YUV format
        AVColorSpace yuv_cs = AVCOL_SPC_UNSPECIFIED;
    } src, dst;
};

bool is_yuv_format_descriptor(const AVPixFmtDescriptor *desc)
{
    return !(desc->flags & AV_PIX_FMT_FLAG_RGB) && !(desc->flags & AV_PIX_FMT_FLAG_XYZ) &&
           (desc->nb_components == 3 || desc->nb_components == 4);
}

struct YuvColorSpaceMapEntry
{
    AVColorSpace av_colorspace;
    AVColorRange av_range;
    int bit_depth;
    SkYUVColorSpace sk_colorspace;
} const yuv_colorspace_map[] = {
    { AVCOL_SPC_BT470BG, AVCOL_RANGE_JPEG, 8, kJPEG_Full_SkYUVColorSpace },
    { AVCOL_SPC_BT470BG, AVCOL_RANGE_MPEG, 8, kRec601_Limited_SkYUVColorSpace },
    { AVCOL_SPC_BT709, AVCOL_RANGE_JPEG, 8, kRec709_Full_SkYUVColorSpace },
    { AVCOL_SPC_BT709, AVCOL_RANGE_MPEG, 8, kRec709_Limited_SkYUVColorSpace },
    { AVCOL_SPC_BT2020_NCL, AVCOL_RANGE_JPEG, 8, kBT2020_8bit_Full_SkYUVColorSpace },
    { AVCOL_SPC_BT2020_NCL, AVCOL_RANGE_MPEG, 8, kBT2020_8bit_Limited_SkYUVColorSpace },
    { AVCOL_SPC_BT2020_NCL, AVCOL_RANGE_JPEG, 10, kBT2020_10bit_Full_SkYUVColorSpace },
    { AVCOL_SPC_BT2020_NCL, AVCOL_RANGE_MPEG, 10, kBT2020_10bit_Limited_SkYUVColorSpace },
    { AVCOL_SPC_BT2020_NCL, AVCOL_RANGE_JPEG, 12, kBT2020_12bit_Full_SkYUVColorSpace },
    { AVCOL_SPC_BT2020_NCL, AVCOL_RANGE_MPEG, 12, kBT2020_12bit_Limited_SkYUVColorSpace },
    { AVCOL_SPC_YCGCO, AVCOL_RANGE_JPEG, 8, kYCgCo_8bit_Full_SkYUVColorSpace },
    { AVCOL_SPC_YCGCO, AVCOL_RANGE_MPEG, 8, kYCgCo_8bit_Limited_SkYUVColorSpace }
};

SkYUVAInfo make_SkYUVAInfo_from_format_record(const ConversionRecord::FormatRecord& record)
{
    const AVPixFmtDescriptor *desc = record.fmtdesc;
    int nb_planes = av_pix_fmt_count_planes(record.format);
    bool has_alpha = desc->flags & AV_PIX_FMT_FLAG_ALPHA;

    SkYUVAInfo::PlaneConfig plane_config;
    if (nb_planes == 1)
    {
        if (desc->comp[0].offset == 0)
            plane_config = has_alpha ? SkYUVAInfo::PlaneConfig::kYUVA : SkYUVAInfo::PlaneConfig::kYUV;
        else
            plane_config = has_alpha ? SkYUVAInfo::PlaneConfig::kUYVA : SkYUVAInfo::PlaneConfig::kUYV;
    }
    // If U, V components shares the same second plane
    else if (desc->comp[1].plane == 1 && desc->comp[2].plane == 1)
    {
        // In the second plane, if U component is the first
        if (desc->comp[1].offset == 0)
            plane_config = has_alpha ? SkYUVAInfo::PlaneConfig::kY_UV_A : SkYUVAInfo::PlaneConfig::kY_UV;
        else
            plane_config = has_alpha ? SkYUVAInfo::PlaneConfig::kY_VU_A : SkYUVAInfo::PlaneConfig::kY_VU;
    }
    // If U is the second plane, and V is the third plane
    else if (desc->comp[1].plane == 1 && desc->comp[2].plane == 2)
    {
        plane_config = has_alpha ? SkYUVAInfo::PlaneConfig::kY_U_V_A : SkYUVAInfo::PlaneConfig::kY_U_V;
    }
    // If U is the third plane, and V is the second plane
    else if (desc->comp[1].plane == 2 && desc->comp[2].plane == 1)
    {
        plane_config = has_alpha ? SkYUVAInfo::PlaneConfig::kY_V_U_A : SkYUVAInfo::PlaneConfig::kY_V_U;
    }
    else
        return {};

    SkYUVAInfo::Subsampling subsampling;
    if (desc->log2_chroma_w == 0 && desc->log2_chroma_h == 0)
        subsampling = SkYUVAInfo::Subsampling::k444;
    else if (desc->log2_chroma_w == 1 && desc->log2_chroma_h == 0)
        subsampling = SkYUVAInfo::Subsampling::k422;
    else if (desc->log2_chroma_w == 1 && desc->log2_chroma_h == 1)
        subsampling = SkYUVAInfo::Subsampling::k420;
    else if (desc->log2_chroma_w == 0 && desc->log2_chroma_h == 1)
        subsampling = SkYUVAInfo::Subsampling::k440;
    else if (desc->log2_chroma_w == 2 && desc->log2_chroma_h == 0)
        subsampling = SkYUVAInfo::Subsampling::k411;
    else if (desc->log2_chroma_w == 2 && desc->log2_chroma_h == 2)
        subsampling = SkYUVAInfo::Subsampling::k410;
    else
        return {};

    SkYUVColorSpace colorspace = kIdentity_SkYUVColorSpace;
    for (const YuvColorSpaceMapEntry& entry : yuv_colorspace_map)
    {
        if (entry.av_colorspace == record.yuv_cs && entry.av_range == record.range &&
            entry.bit_depth == desc->comp[0].depth)
        {
            colorspace = entry.sk_colorspace;
            break;
        }
    }
    if (colorspace == kIdentity_SkYUVColorSpace)
        return {};

    // TODO(sora): Take `SkYUVAInfo::Siting` into consider when Skia supports other sitings
    return { record.dimensions, plane_config, subsampling, colorspace, kTopLeft_SkEncodedOrigin,
               SkYUVAInfo::Siting::kCentered, SkYUVAInfo::Siting::kCentered };
}

struct FullPlanarFuncEntry
{
    template<typename SrcT>
    using FullPlanarConvertFuncT = int(const SrcT* src_y, int src_stride_y,
                                       const SrcT* src_u, int src_stride_u,
                                       const SrcT* src_v, int src_stride_v,
                                       uint8_t* dst, int dst_stride,
                                       const libyuv::YuvConstants *yuvconstants,
                                       int width, int height);

    template<typename SrcT>
    using FullPlanarAlphaConvertFuncT = int(const SrcT* src_y, int src_stride_y,
                                            const SrcT* src_u, int src_stride_u,
                                            const SrcT* src_v, int src_stride_v,
                                            const uint8_t* src_a, int src_stride_a,
                                            uint8_t* dst, int dst_stride,
                                            const libyuv::YuvConstants *yuvconstants,
                                            int width, int height,
                                            int attenuate);

    SkYUVAInfo::Subsampling chroma_subsampling = SkYUVAInfo::Subsampling::kUnknown;
    int src_bit_depth = 0;
#define BIT_DEPTH_8     8
#define BIT_DEPTH_10    10
#define BIT_DEPTH_12    12
#define BIT_DEPTH_16    16

    AVPixelFormat dstfmt = AV_PIX_FMT_NONE;
    FullPlanarConvertFuncT<uint8_t> *cvt_no_alpha = nullptr;
    FullPlanarAlphaConvertFuncT<uint8_t> *cvt_alpha = nullptr;
    FullPlanarConvertFuncT<uint16_t> *cvt_hdr_no_alpha = nullptr;
};

constexpr FullPlanarFuncEntry full_planar_func_entries[] {
#define S(from, to)         .cvt_no_alpha = libyuv::from##To##to##Matrix

#define SALPHA(from, to)    .cvt_no_alpha = libyuv::from##To##to##Matrix, \
                            .cvt_alpha = libyuv::from##AlphaTo##to##Matrix

#define SHDR(from, to)      .cvt_hdr_no_alpha = libyuv::from##To##to##Matrix

#define E(sub, depth, dstfmt_, ...)                                 \
    { .chroma_subsampling = SkYUVAInfo::Subsampling::k##sub,        \
      .src_bit_depth = BIT_DEPTH_##depth,                           \
      .dstfmt = AV_PIX_FMT_##dstfmt_ __VA_OPT__(,) __VA_ARGS__ }

    // SDR formars, 8bits
    E(420, 8, BGRA, SALPHA(I420, ARGB)),
    E(420, 8, BGR0, S(I420, ARGB)),
    E(420, 8, X2RGB10LE, S(I420, AR30)),
    E(420, 8, RGB565LE, S(I420, RGB565)),
    E(422, 8, BGRA, SALPHA(I422, ARGB)),
    E(422, 8, BGR0, S(I422, ARGB)),
    E(444, 8, BGRA, SALPHA(I444, ARGB)),
    E(444, 8, BGR0, S(I444, ARGB)),

    // HDR formats, 10bits
    E(420, 10, X2RGB10LE, SHDR(I010, AR30)),
    E(420, 10, BGRA, SHDR(I010, ARGB)),
    E(420, 10, BGR0, SHDR(I010, ARGB)),
    E(422, 10, X2RGB10LE, SHDR(I210, AR30)),
    E(422, 10, BGRA, SHDR(I210, ARGB)),
    E(422, 10, BGR0, SHDR(I210, ARGB)),
    E(444, 10, X2RGB10LE, SHDR(I410, AR30)),
    E(444, 10, BGRA, SHDR(I410, ARGB)),
    E(444, 10, BGR0, SHDR(I410, ARGB)),

    // HDR formats, 12bits
    E(420, 12, X2RGB10LE, SHDR(I012, AR30)),
    E(420, 12, BGRA, SHDR(I012, ARGB)),
    E(420, 12, BGR0, SHDR(I012, ARGB)),

    // not supported by libyuv
    /*
    E(422, 12, X2RGB10LE, SHDR(I212, AR30)),
    E(422, 12, BGRA, SHDR(I212, ARGB)),
    E(422, 12, BGR0, SHDR(I212, ARGB)),
    E(444, 12, X2RGB10LE, SHDR(I412, AR30)),
    E(444, 12, BGRA, SHDR(I412, ARGB)),
    E(444, 12, BGR0, SHDR(I412, ARGB)),
    */

#undef E
#undef SHDR
#undef S
#undef SALPHA
};

const FullPlanarFuncEntry *find_full_planar_func_entry(SkYUVAInfo::Subsampling chromasub,
                                                       int depth,
                                                       AVPixelFormat dstfmt)
{
    for (const FullPlanarFuncEntry& entry : full_planar_func_entries)
        if (entry.chroma_subsampling == chromasub && entry.src_bit_depth == depth && entry.dstfmt == dstfmt)
            return &entry;
    return nullptr;
}

struct SemiplanarFuncEntry
{
    template<typename SrcT>
    using SemiplanarConvertFuncT = int(const SrcT* src_y, int src_stride_y,
                                       const SrcT* src_uv, int src_stride_uv,
                                       uint8_t* dst_rgba, int dst_stride_rgba,
                                       const libyuv::YuvConstants *yuvconstants,
                                       int width, int height);

    AVPixelFormat srcfmt = AV_PIX_FMT_NONE;
    AVPixelFormat dstfmt = AV_PIX_FMT_NONE;
    SemiplanarConvertFuncT<uint8_t> *cvt_no_alpha = nullptr;
    SemiplanarConvertFuncT<uint16_t> *cvt_hdr_no_alpha = nullptr;
};
constexpr SemiplanarFuncEntry semiplanar_func_entries[] {
#define E(src, dst, from, to)                                   \
    { .srcfmt = AV_PIX_FMT_##src, .dstfmt = AV_PIX_FMT_##dst,   \
      .cvt_no_alpha = libyuv::from##To##to##Matrix }

#define EHDR(src, dst, from, to) \
    { .srcfmt = AV_PIX_FMT_##src, .dstfmt = AV_PIX_FMT_##dst,   \
      .cvt_no_alpha = nullptr,                                  \
      .cvt_hdr_no_alpha = libyuv::from##To##to##Matrix }

    E(NV12, BGRA, NV12, ARGB),
    E(NV12, BGR0, NV12, ARGB),
    E(NV21, BGRA, NV21, ARGB),
    E(NV21, BGR0, NV21, ARGB),

    EHDR(P010LE, X2RGB10LE, P010, AR30),
    EHDR(P010LE, BGRA, P010, ARGB),
    EHDR(P010LE, BGR0, P010, ARGB),
    EHDR(P210LE, X2RGB10LE, P210, AR30),
    EHDR(P210LE, BGRA, P210, ARGB),
    EHDR(P210LE, BGR0, P210, ARGB),

    EHDR(P012LE, X2RGB10LE, P012, AR30),
    EHDR(P012LE, BGRA, P012, ARGB),
    EHDR(P012LE, BGR0, P012, ARGB),
    EHDR(P212LE, X2RGB10LE, P212, AR30),
    EHDR(P212LE, BGRA, P212, ARGB),
    EHDR(P212LE, BGR0, P212, ARGB),

    EHDR(P016LE, X2RGB10LE, P016, AR30),
    EHDR(P016LE, BGRA, P016, ARGB),
    EHDR(P016LE, BGR0, P016, ARGB),
    EHDR(P216LE, X2RGB10LE, P216, AR30),
    EHDR(P216LE, BGRA, P216, ARGB),
    EHDR(P216LE, BGR0, P216, ARGB)

#undef E
#undef EHDR
};

const SemiplanarFuncEntry *find_semiplanar_func_entry(AVPixelFormat src, AVPixelFormat dst)
{
    for (const SemiplanarFuncEntry& entry : semiplanar_func_entries)
        if (entry.srcfmt == src && entry.dstfmt == dst)
            return &entry;
    return nullptr;
}

using ConvertFuncT = std::function<bool(const AVFrame *src, const AVFrame *dst)>;
ConvertFuncT try_resolve_YUV_direct_convert_func(const ConversionRecord& record)
{
    static const std::map<SkYUVColorSpace, const libyuv::YuvConstants*> yuv_matrix_map{
        { kRec601_SkYUVColorSpace,                  &libyuv::kYuvI601Constants  },
        { kJPEG_Full_SkYUVColorSpace,               &libyuv::kYuvJPEGConstants  },
        { kRec709_Limited_SkYUVColorSpace,          &libyuv::kYuvH709Constants  },
        { kRec709_Full_SkYUVColorSpace,             &libyuv::kYuvF709Constants  },
        { kBT2020_8bit_Limited_SkYUVColorSpace,     &libyuv::kYuv2020Constants  },
        { kBT2020_8bit_Full_SkYUVColorSpace,        &libyuv::kYuvV2020Constants },
        { kBT2020_10bit_Limited_SkYUVColorSpace,    &libyuv::kYuv2020Constants  },
        { kBT2020_10bit_Full_SkYUVColorSpace,       &libyuv::kYuvV2020Constants },
        { kBT2020_12bit_Limited_SkYUVColorSpace,    &libyuv::kYuv2020Constants  },
        { kBT2020_12bit_Full_SkYUVColorSpace,       &libyuv::kYuvV2020Constants }
    };

    // Get YUVA info first
    const SkYUVAInfo src_info = make_SkYUVAInfo_from_format_record(record.src);
    if (!src_info.isValid())
        return nullptr;

    // Select a YUV => RGB conversion matrix, which is up to the source YUV colorspace
    // and color range (full or limited).
    if (!yuv_matrix_map.contains(src_info.yuvColorSpace()))
        return nullptr;
    const libyuv::YuvConstants *cs_matrix = yuv_matrix_map.at(src_info.yuvColorSpace());

    const SkYUVAInfo::PlaneConfig src_plane_layout = src_info.planeConfig();
    const int src_bit_depth = record.src.fmtdesc->comp[0].depth;

    // Full planar layout
    if (src_plane_layout == SkYUVAInfo::PlaneConfig::kY_U_V ||
        src_plane_layout == SkYUVAInfo::PlaneConfig::kY_U_V_A ||
        src_plane_layout == SkYUVAInfo::PlaneConfig::kY_V_U ||
        src_plane_layout == SkYUVAInfo::PlaneConfig::kY_V_U_A)
    {
        const FullPlanarFuncEntry *entry = find_full_planar_func_entry(
            src_info.subsampling(), src_bit_depth, record.dst.format);
        if (!entry)
            return {};

        // U, V plane indices
        int pu = 1, pv = 2;
        if (src_plane_layout == SkYUVAInfo::PlaneConfig::kY_V_U ||
            src_plane_layout == SkYUVAInfo::PlaneConfig::kY_V_U_A)
        {
            pu = 2;
            pv = 1;
        }

        switch (src_bit_depth)
        {
        // SDR formats, 8bits
        case BIT_DEPTH_8:
            if (src_info.hasAlpha() && entry->cvt_alpha)
            {
                return [entry, cs_matrix, pu, pv](const AVFrame *src, const AVFrame *dst) {
                    int res = entry->cvt_alpha(
                        src->data[0], src->linesize[0],
                        src->data[pu], src->linesize[pu],
                        src->data[pv], src->linesize[pv],
                        src->data[3], src->linesize[3],
                        dst->data[0], dst->linesize[0],
                        cs_matrix,
                        src->width, src->height,
                        // we only use unpremultiplied ARGB format
                        /* attenuate (premultiply) = */ 0
                    );
                    return res >= 0;
                };
            }

            CHECK(entry->cvt_no_alpha);
            return [entry, cs_matrix, pu, pv](const AVFrame *src, const AVFrame *dst) {
                int res = entry->cvt_no_alpha(
                    src->data[0], src->linesize[0],
                    src->data[pu], src->linesize[pu],
                    src->data[pv], src->linesize[pv],
                    dst->data[0], dst->linesize[0],
                    cs_matrix,
                    src->width, src->height
                );
                return res >= 0;
            };

        // HDR formats, 10bits and 12bits
        case BIT_DEPTH_10:
        case BIT_DEPTH_12:
            if (src_info.hasAlpha())
                return {};

            CHECK(entry->cvt_hdr_no_alpha);
            return [entry, cs_matrix, pu, pv](const AVFrame *src, const AVFrame *dst) {
                int res = entry->cvt_hdr_no_alpha(
                    reinterpret_cast<const uint16_t*>(src->data[0]), src->linesize[0],
                    reinterpret_cast<const uint16_t*>(src->data[pu]), src->linesize[pu],
                    reinterpret_cast<const uint16_t*>(src->data[pv]), src->linesize[pv],
                    dst->data[0], dst->linesize[0],
                    cs_matrix,
                    src->width, src->height
                );
                return res >= 0;
            };

        default:
            return {};
        }
    }

    // Semiplanar layout (without alpha plane)
    if (src_plane_layout == SkYUVAInfo::PlaneConfig::kY_UV ||
        src_plane_layout == SkYUVAInfo::PlaneConfig::kY_VU)
    {
        const SemiplanarFuncEntry *entry = find_semiplanar_func_entry(
            record.src.format, record.dst.format);
        if (!entry)
            return {};

        switch (src_bit_depth)
        {
        // SDR formats, 8bits
        case BIT_DEPTH_8:
            CHECK(entry->cvt_no_alpha);
            return [entry, cs_matrix](const AVFrame *src, const AVFrame *dst) {
                int res = entry->cvt_no_alpha(
                    src->data[0], src->linesize[0],
                    src->data[1], src->linesize[1],
                    dst->data[0], dst->linesize[0],
                    cs_matrix,
                    src->width, src->height
                );
                return res >= 0;
            };

        // HDR formats, 10bits, 12bits, and 16bits
        case BIT_DEPTH_10:
        case BIT_DEPTH_12:
        case BIT_DEPTH_16:
            CHECK(entry->cvt_hdr_no_alpha);
            return [entry, cs_matrix](const AVFrame *src, const AVFrame *dst) {
                int res = entry->cvt_hdr_no_alpha(
                    reinterpret_cast<const uint16_t*>(src->data[0]), src->linesize[0],
                    reinterpret_cast<const uint16_t*>(src->data[1]), src->linesize[1],
                    dst->data[0], dst->linesize[0],
                    cs_matrix,
                    src->width, src->height
                );
                return res >= 0;
            };

        default:
            return {};
        }
    }

    return {};
}

AVFrame *convert_YUV_to_RGB(const AVFrame *src, const ConversionRecord& record)
{
    CHECK(record.src.dimensions == record.dst.dimensions &&
          record.src.primaries == record.dst.primaries &&
          record.src.trc == record.dst.trc);

    ConvertFuncT convert_func = try_resolve_YUV_direct_convert_func(record);
    if (!convert_func)
    {
        // If direct conversion (one-step conversion) is impossible, converts to a
        // trampoline format (AV_PIX_FMT_BGRA) first.

        // TODO(sora): implement this.
        return nullptr;
    }

    AVFrame *dst = av_frame_alloc();
    CHECK(dst && "allocation failed");
    dst->format = record.dst.format;
    dst->width = record.dst.dimensions.width();
    dst->height = record.dst.dimensions.height();
    CHECK(av_frame_get_buffer(dst, 0) >= 0 && "allocation failed");

    if (!convert_func(src, dst))
        av_frame_free(&dst);

    return dst;
}

} // namespace anonymous

sk_sp<SkColorSpace> ResolveFrameColorCharacteristicsToSkColorSpace(AVColorPrimaries primaries,
                                                                   AVColorTransferCharacteristic trc,
                                                                   const sk_sp<SkColorSpace>& fallback)
{
    skcms_Matrix3x3 gamut_mat;
    if (!av_color_primaries_to_skcms(primaries, gamut_mat))
        return fallback;

    if (!cs_transfer_map.contains(trc))
        return fallback;
    skcms_TransferFunction transfer_func = cs_transfer_map.at(trc);

    return SkColorSpace::MakeRGB(transfer_func, gamut_mat);
}

SkYUVAInfo ResolveFrameSkYUVAInfo(const SkISize& dimensions,
                                  AVPixelFormat format, AVColorSpace spc, AVColorRange range)
{
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(format);
    if (!is_yuv_format_descriptor(desc))
        return {};
    return make_SkYUVAInfo_from_format_record({
        .dimensions = dimensions,
        .format = format,
        .fmtdesc = desc,
        .range = range,
        .yuv_cs = spc
    });
}

namespace {

FrameBlitErrorCode blit_supported_rgba_frame_to_pixmap(const uint8_t *rgba_pixels,
                                                       const AVPixelFormat src_format,
                                                       const AVPixFmtDescriptor *src_fmt_desc,
                                                       const int32_t src_row_bytes,
                                                       const sk_sp<SkColorSpace>& src_colorspace,
                                                       const SkIRect& src_region,
                                                       const SkPixmap& dst_pixmap,
                                                       const SkIRect& dst_region,
                                                       const ScaleResampler scale_resampler)
{
    TRACE_EVENT("multimedia", "blit_supported_rgba_frame_to_pixmap");

    CHECK(!(src_fmt_desc->flags & AV_PIX_FMT_FLAG_PLANAR));
    auto [src_color_type, src_alpha_type] = color_type_map.at(src_format);

    SkImageInfo src_region_image_info = SkImageInfo::Make(
        src_region.size(), src_color_type, src_alpha_type, src_colorspace);
    int bytes_per_pixel = av_get_padded_bits_per_pixel(src_fmt_desc) >> 3;
    CHECK(bytes_per_pixel == src_region_image_info.bytesPerPixel());

    SkPixmap src_region_pixmap(
        src_region_image_info,
        rgba_pixels + src_region.y() * src_row_bytes + src_region.x() * bytes_per_pixel,
        src_row_bytes
    );

    SkPixmap dst_region_pixmap;
    CHECK(dst_pixmap.extractSubset(&dst_region_pixmap, dst_region));
    CHECK(dst_region_pixmap.colorSpace());

    // fastpath: Direct block transfer without any conversions.
    // This requires the src and dst has the same color info, including format, alpha type,
    // and colorspace (primaries and gamma).
    if (src_region_image_info.colorInfo() == dst_pixmap.info().colorInfo())
    {
        CHECK(src_region_pixmap.readPixels(dst_region_pixmap));
        return FrameBlitErrorCode::kSuccess;
    }

    // Conversion and scaling is not avoidable
    bool can_release_src_pixels = false;
    sk_sp<SkImage> src_region_image = SkImages::RasterFromPixmap(
        src_region_pixmap,

        // It is not guaranteed that Skia will call this callback immediately after the SkImage
        // is released. But according to its code, at least the current version of Skia does
        // do that. So we can just rely on it.
        +[](const void *, void *userdata) {
            *static_cast<bool*>(userdata) = true;
        },
        &can_release_src_pixels
    );
    CHECK(src_region_image);

    struct Closure
    {
        FrameBlitErrorCode status = FrameBlitErrorCode::kUndefined;
        SkPixmap pixmap;
    } closure{ .pixmap = dst_region_pixmap };

    // TODO(sora): this function is incredibly slow (~30ms). try to find a solution.

    // This operation is actually synchronous on raster images
    src_region_image->asyncRescaleAndReadPixels(
        dst_region_pixmap.info(),
        src_region_pixmap.bounds(),
        SkImage::RescaleGamma::kLinear,
        resampler_to_skimage_rescale_mode(scale_resampler),
        +[](void *userdata, std::unique_ptr<const SkImage::AsyncReadResult> result) {
            Closure* const self = static_cast<Closure*>(userdata);
            if (!result)
            {
                self->status = FrameBlitErrorCode::kConversionFailure;
                return;
            }
            CHECK(result->count() == 1 && "multiple plane RGB");
            SkPixmap pixmap(self->pixmap.info(), result->data(0), result->rowBytes(0));
            pixmap.readPixels(self->pixmap);
            self->status = FrameBlitErrorCode::kSuccess;
        },
        &closure
    );

    src_region_image.reset();
    CHECK(can_release_src_pixels);
    CHECK(closure.status != FrameBlitErrorCode::kUndefined);
    return closure.status;
}

} // namespace anonymous

FrameBlitErrorCode BlitFrameToSkPixmap(const AVFrame *src_frame, const SkIRect& src_region,
                                       SkPixmap& dst_pixmap, const SkIRect& dst_region,
                                       ScaleResampler scale_resampler)
{
    TRACE_EVENT("multimedia", "BlitFrameToSkPixmap");

    if (!src_frame || src_frame->width <= 0 || src_frame->height <= 0)
        return FrameBlitErrorCode::kSrcFrameInvalid;

    // Only 1:1 SAR is supported
    if ((src_frame->sample_aspect_ratio.den != 1 ||
         src_frame->sample_aspect_ratio.num != 1) &&
        src_frame->sample_aspect_ratio.num != 0)
    {
        return FrameBlitErrorCode::kSrcUnsupportedSAR;
    }

    if (!SkIRect::MakeWH(src_frame->width, src_frame->height).contains(src_region))
        return FrameBlitErrorCode::kSrcRegionOutOfRange;
    if (!dst_pixmap.addr())
        return FrameBlitErrorCode::kDstPixmapInvalid;
    if (!SkIRect::MakeWH(dst_pixmap.width(), dst_pixmap.height()).contains(dst_region))
        return FrameBlitErrorCode::kDstRegionOutOfRange;

    // Negotiate source and destination color space info
    AVColorPrimaries src_primaries = src_frame->color_primaries;
    if (src_primaries == AVCOL_PRI_UNSPECIFIED)
        src_primaries = AVCOL_PRI_BT709;
    AVColorTransferCharacteristic src_trc = src_frame->color_trc;
    if (src_trc == AVCOL_TRC_UNSPECIFIED)
        src_trc = AVCOL_TRC_BT709;
    AVColorRange src_range = src_frame->color_range;
    if (src_range == AVCOL_RANGE_UNSPECIFIED)
        src_range = AVCOL_RANGE_MPEG;
    sk_sp<SkColorSpace> src_colorspace = ResolveFrameColorCharacteristicsToSkColorSpace(
        src_primaries, src_trc, nullptr);
    if (!src_primaries)
        return FrameBlitErrorCode::kIncompatibleColorSpace;

    // If the dst colorspace is not speicifed, we assume it has the same colorspace
    // with the src colorspace.
    if (!dst_pixmap.colorSpace())
        dst_pixmap.setColorSpace(src_colorspace);

    const AVPixelFormat src_format = static_cast<AVPixelFormat>(src_frame->format);
    const AVPixFmtDescriptor *src_fmt_desc = av_pix_fmt_desc_get(src_format);

    // If the source color format is directly supported by Skia,
    // use Skia to perform all the conversions.
    if (color_type_map.contains(src_format))
    {
        return blit_supported_rgba_frame_to_pixmap(
            src_frame->data[0],
            src_format,
            src_fmt_desc,
            src_frame->linesize[0],
            src_colorspace,
            src_region,
            dst_pixmap,
            dst_region,
            scale_resampler
        );
    }

    if (is_yuv_format_descriptor(src_fmt_desc))
    {
        TRACE_EVENT("multimedia", "BlitFrameToSkPixmap[yuv_conv]");

        AVColorSpace src_yuv_space = src_frame->colorspace;
        if (src_yuv_space == AVCOL_SPC_UNSPECIFIED)
            src_yuv_space = AVCOL_SPC_BT709;

        const auto value = std::ranges::find_if(
            color_type_map,
            [match_pair = std::make_pair(dst_pixmap.colorType(), dst_pixmap.alphaType())]
            (const auto& kv_pair) {
                return kv_pair.second == match_pair;
            }
        );
        if (value == color_type_map.end())
            return FrameBlitErrorCode::kConversionFailure;
        const AVPixelFormat dst_format = value->first;

        ConversionRecord rec;
        rec.src = {
            .dimensions = src_region.size(),
            .format = src_format,
            .fmtdesc = src_fmt_desc,
            .primaries = src_primaries,
            .trc = src_trc,
            .range = src_range,
            .yuv_cs = src_yuv_space
        };

        rec.dst = {
            .dimensions = src_region.size(),
            .format = dst_format,
            .fmtdesc = av_pix_fmt_desc_get(dst_format),
            .primaries = src_primaries,
            .trc = src_trc,
            .range = AVCOL_RANGE_JPEG
        };

        // Crop the frame
        AVFrame *src_crop_frame = av_frame_clone(src_frame);
        src_crop_frame->crop_top = src_region.top();
        src_crop_frame->crop_left = src_region.left();
        src_crop_frame->crop_bottom =  src_crop_frame->height - src_region.bottom();
        src_crop_frame->crop_right = src_crop_frame->width - src_region.right();
        // Allows unaligned cropping to get the accurate cropping. FFmpeg API does not accept
        // unaligned pointers but we use libyuv to process the conversion instead.
        CHECK(av_frame_apply_cropping(src_crop_frame, AV_FRAME_CROP_UNALIGNED) >= 0);

        AVFrame *rgba_frame = convert_YUV_to_RGB(src_crop_frame, rec);
        av_frame_free(&src_crop_frame);

        if (!rgba_frame)
            return FrameBlitErrorCode::kConversionFailure;

        // Now `rgb_frame` is a cropped frame (by `src_region`)  with RGB format, having the same
        // color primaries and gamma transfer function with the src frame.

        FrameBlitErrorCode status = blit_supported_rgba_frame_to_pixmap(
            rgba_frame->data[0],
            rec.dst.format,
            rec.dst.fmtdesc,
            rgba_frame->linesize[0],
            src_colorspace,
            SkIRect::MakeSize(rec.dst.dimensions),
            dst_pixmap,
            dst_region,
            scale_resampler
        );

        av_frame_free(&rgba_frame);
        return status;
    }

    return FrameBlitErrorCode::kSrcUnsupportedFormat;
}

sk_sp<SkImage> BlitFrameToSkImage(const AVFrame *src_frame, const SkIRect& src_region,
                                  const SkImageInfo& dst_image_info,
                                  ScaleResampler scale_resampler,
                                  FrameBlitErrorCode *out_status_code)
{
    TRACE_EVENT("multimedia", "BlitFrameToSkImage");

    SkImageInfo image_info = dst_image_info;
    if (image_info.isEmpty())
        image_info = image_info.makeDimensions(src_region.size());
    if (image_info.colorType() == kUnknown_SkColorType)
        image_info = image_info.makeColorType(kBGRA_8888_SkColorType);
    if (image_info.alphaType() == kUnknown_SkAlphaType)
        image_info = image_info.makeAlphaType(kOpaque_SkAlphaType);

    size_t dst_row_bytes = image_info.minRowBytes64();
    sk_sp<SkData> data = SkData::MakeUninitialized(image_info.computeMinByteSize());
    CHECK(data && "allocation failed");
    SkPixmap dst_pixmap(image_info, data->writable_data(), dst_row_bytes);

    FrameBlitErrorCode status = BlitFrameToSkPixmap(
        src_frame, src_region, dst_pixmap, dst_pixmap.bounds(), scale_resampler);
    if (out_status_code)
        *out_status_code = status;

    return SkImages::RasterFromData(image_info, data, dst_row_bytes);
}

SkYUVAPixmaps WrapFrameToSkYUVAPixmaps(const AVFrame *frame)
{
    const AVPixFmtDescriptor *format_desc = av_pix_fmt_desc_get(static_cast<AVPixelFormat>(frame->format));
    CHECK(format_desc && "invalid pixel format");
    if (!is_yuv_format_descriptor(format_desc))
        return {};

    SkYUVAInfo info = make_SkYUVAInfo_from_format_record({
        .dimensions = SkISize::Make(frame->width, frame->height),
        .format = static_cast<AVPixelFormat>(frame->format),
        .fmtdesc = format_desc,
        .primaries = frame->color_primaries == AVCOL_PRI_UNSPECIFIED ?
                     AVCOL_PRI_BT709 : frame->color_primaries,
        .trc = frame->color_trc == AVCOL_TRC_UNSPECIFIED ?
               AVCOL_TRC_BT709 : frame->color_trc,
        .range = frame->color_range == AVCOL_RANGE_UNSPECIFIED ?
                 AVCOL_RANGE_MPEG : frame->color_range,
        .yuv_cs = frame->colorspace == AVCOL_SPC_UNSPECIFIED ?
                  AVCOL_SPC_BT709 : frame->colorspace
    });
    if (!info.isValid())
        return {};

    SkPixmap pixmaps[SkYUVAPixmapInfo::kMaxPlanes];
    SkISize plane_dimensions[SkYUVAPixmapInfo::kMaxPlanes];
    info.planeDimensions(plane_dimensions);

    for (int i = 0; i < info.numPlanes(); i++)
    {
        std::optional<SkYUVAPixmapInfo::DataType> plane_data_type;
        for (int c = 0; c < format_desc->nb_components; c++)
        {
            if (format_desc->comp[c].plane != i)
                continue;

            SkYUVAPixmapInfo::DataType data_type;
            int depth = format_desc->comp[c].depth;
            if (depth == 8)
                data_type = SkYUVAPixmapInfo::DataType::kUnorm8;
            else if (depth == 10 || depth == 12 || depth == 16)
                data_type = SkYUVAPixmapInfo::DataType::kUnorm16;
            else
                return {};

            if (!plane_data_type)
                plane_data_type = data_type;
            // All the components in the same plane must have consistent data types
            else if (*plane_data_type != data_type)
                return {};
        }

        int plane_channels = info.numChannelsInPlane(i);
        SkColorType ct = SkYUVAPixmapInfo::DefaultColorTypeForDataType(*plane_data_type, plane_channels);
        if (ct == kUnknown_SkColorType)
            return {};

        SkImageInfo image_info = SkImageInfo::Make(plane_dimensions[i], ct, kOpaque_SkAlphaType, nullptr);
        pixmaps[i].reset(image_info, frame->data[i], frame->linesize[i]);
    }

    return SkYUVAPixmaps::FromExternalPixmaps(info, pixmaps);
}

namespace
{

struct YuvaDataFormatMap
{
    SkYUVAPixmapInfo::DataType data_type;
    SkYUVAPixmapInfo::Subsampling subsampling;
    SkYUVAPixmapInfo::PlaneConfig plane_config;

    AVPixelFormat format;
} yuva_data_format_map[] {
#define T(dt, sub, pl) SkYUVAPixmapInfo::DataType::k##dt, \
                       SkYUVAPixmapInfo::Subsampling::k##sub, \
                       SkYUVAPixmapInfo::PlaneConfig::k##pl

    // Full planar SDR formats
    { T(Unorm8, 420, Y_U_V), AV_PIX_FMT_YUV420P  },
    { T(Unorm8, 420, Y_U_V_A), AV_PIX_FMT_YUVA420P },
    { T(Unorm8, 444, Y_U_V), AV_PIX_FMT_YUV444P },
    { T(Unorm8, 444, Y_U_V_A), AV_PIX_FMT_YUVA444P },
    { T(Unorm8, 422, Y_U_V), AV_PIX_FMT_YUV422P },
    { T(Unorm8, 422, Y_U_V_A), AV_PIX_FMT_YUVA422P },
    { T(Unorm8, 440, Y_U_V), AV_PIX_FMT_YUV440P },
    { T(Unorm8, 410, Y_U_V), AV_PIX_FMT_YUV410P },

    // Semiplanar SDR formats
    { T(Unorm8, 420, Y_UV), AV_PIX_FMT_NV12 },
    { T(Unorm8, 420, Y_VU), AV_PIX_FMT_NV21 }

    // TODO(sora): support HDR formats
#undef T
};

} // namespace anonymous

AVFrame *MakeFrameFromSkYUVAPixmapsWithCopy(const SkYUVAPixmaps& pixmaps,
                                            AVColorPrimaries color_primaries,
                                            AVColorTransferCharacteristic color_trc)
{
    if (!pixmaps.isValid())
        return nullptr;

    const SkYUVAPixmapInfo& info = pixmaps.pixmapsInfo();
    AVPixelFormat format = AV_PIX_FMT_NONE;
    for (const YuvaDataFormatMap& entry : yuva_data_format_map)
    {
        if (entry.data_type == info.dataType() &&
            entry.subsampling == info.yuvaInfo().subsampling() &&
            entry.plane_config == info.yuvaInfo().planeConfig())
        {
            format = entry.format;
            break;
        }
    }
    if (format == AV_PIX_FMT_NONE)
        return nullptr;

    AVFrame *frame = av_frame_alloc();
    CHECK(frame && "allocation failed");

    // `pixmaps` probably owns the pixel memory, which means buffer sharing is possible.
    // But FFmpeg has special requirements of memory address (alignment) and length (extra 16bytes for
    // some filters). To avoid buggy situations, we just let FFmpeg allocate its buffers and copy
    // pixels into them.

    frame->format = format;
    frame->width = info.yuvaInfo().width();
    frame->height = pixmaps.yuvaInfo().height();
    CHECK(av_frame_get_buffer(frame, 0) >= 0 && "allocation failed");

    int nb_planes = info.numPlanes();
    for (int i = 0; i < nb_planes; i++)
    {
        const SkPixmap& plane = pixmaps.plane(i);
        uint8_t *plane_addr = static_cast<uint8_t*>(plane.writable_addr());
        size_t src_valid_linesize = plane.width() * plane.info().bytesPerPixel();

        for (int32_t y = 0; y < plane.height(); y++)
        {
            std::memcpy(frame->data[i] + y * frame->linesize[i], plane_addr + y * plane.rowBytes(),
                        src_valid_linesize);
        }
    }

    // Fill color characteristics
    for (const YuvColorSpaceMapEntry& entry : yuv_colorspace_map)
    {
        // TODO(sora): support HDR formats
        if (entry.sk_colorspace == pixmaps.yuvaInfo().yuvColorSpace() &&
            entry.bit_depth == 8)
        {
            frame->colorspace = entry.av_colorspace;
            frame->color_range = entry.av_range;
            break;
        }
    }

    // Skia only supports center now.
    frame->chroma_location = AVCHROMA_LOC_CENTER;

    // There is no enough information to assert the color_primaries and color_trc.
    // We leave them unspecified.
    frame->color_primaries = color_primaries;
    frame->color_trc = color_trc;

    return frame;
}

std::optional<std::tuple<SkColorType, SkAlphaType>> PixelFormatToSkiaTypes(AVPixelFormat format)
{
    if (!color_type_map.contains(format))
        return std::nullopt;
    return color_type_map.at(format);
}

std::optional<AVPixelFormat> SkiaColorTypesToPixelFormat(SkColorType ct, SkAlphaType at)
{
    if (ct == kRGBA_8888_SkColorType && at == kOpaque_SkAlphaType)
        ct = kRGB_888x_SkColorType;

    for (const auto& [format, pair] : color_type_map)
    {
        if (pair == std::make_pair(ct, at))
            return format;
    }
    return std::nullopt;
}

UTAU_NAMESPACE_END
