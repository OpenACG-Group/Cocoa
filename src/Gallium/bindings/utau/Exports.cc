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

#include <unordered_map>

#include "Gallium/bindings/utau/Exports.h"
#include "Gallium/binder/ThrowExcept.h"
#include "Utau/Utau.h"
#include "Utau/AVStreamDecoder.h"
#include "Utau/AudioSinkStream.h"
#include "fmt/core.h"
GALLIUM_BINDINGS_UTAU_NS_BEGIN

void SetInstanceProperties(v8::Local<v8::Object> instance)
{
#define I(x) static_cast<int32_t>(x)

    using SFMT = utau::SampleFormat;
    using ChM = utau::AudioChannelMode;

    std::unordered_map<std::string, int32_t> constants{
        { "DAG_RECEIVE_STATUS_OK", I(utau::AVFilterDAG::ReceiveStatus::kOk) },
        { "DAG_RECEIVE_STATUS_ERROR", I(utau::AVFilterDAG::ReceiveStatus::kError) },
        { "DAG_RECEIVE_STATUS_AGAIN", I(utau::AVFilterDAG::ReceiveStatus::kAgain) },
        { "DAG_RECEIVE_STATUS_EOF", I(utau::AVFilterDAG::ReceiveStatus::kEOF) },

        { "SAMPLE_FORMAT_UNKNOWN", I(SFMT::kUnknown) },
        { "SAMPLE_FORMAT_U8",   I(SFMT::kU8)     },
        { "SAMPLE_FORMAT_S16",  I(SFMT::kS16)    },
        { "SAMPLE_FORMAT_S32",  I(SFMT::kS32)    },
        { "SAMPLE_FORMAT_F32",  I(SFMT::kF32)    },
        { "SAMPLE_FORMAT_F64",  I(SFMT::kF64)    },
        { "SAMPLE_FORMAT_U8P",  I(SFMT::kU8P)    },
        { "SAMPLE_FORMAT_S16P", I(SFMT::kS16P)   },
        { "SAMPLE_FORMAT_S32P", I(SFMT::kS32P)   },
        { "SAMPLE_FORMAT_F32P", I(SFMT::kF32P)   },
        { "SAMPLE_FORMAT_F64P", I(SFMT::kF64P)   },

        { "CH_MODE_MONO",       I(ChM::kMono)   },
        { "CH_MODE_STEREO",     I(ChM::kStereo) },

        { "PIXEL_FORMAT_NONE", AV_PIX_FMT_NONE },
        { "PIXEL_FORMAT_YUV420P", AV_PIX_FMT_YUV420P },
        { "PIXEL_FORMAT_YUYV422", AV_PIX_FMT_YUYV422 },
        { "PIXEL_FORMAT_RGB24", AV_PIX_FMT_RGB24 },
        { "PIXEL_FORMAT_BGR24", AV_PIX_FMT_BGR24 },
        { "PIXEL_FORMAT_YUV422P", AV_PIX_FMT_YUV422P },
        { "PIXEL_FORMAT_YUV444P", AV_PIX_FMT_YUV444P },
        { "PIXEL_FORMAT_YUV410P", AV_PIX_FMT_YUV410P },
        { "PIXEL_FORMAT_YUV411P", AV_PIX_FMT_YUV411P },
        { "PIXEL_FORMAT_GRAY8", AV_PIX_FMT_GRAY8 },
        { "PIXEL_FORMAT_MONOWHITE", AV_PIX_FMT_MONOWHITE },
        { "PIXEL_FORMAT_MONOBLACK", AV_PIX_FMT_MONOBLACK },
        { "PIXEL_FORMAT_PAL8", AV_PIX_FMT_PAL8 },
        { "PIXEL_FORMAT_YUVJ420P", AV_PIX_FMT_YUVJ420P },
        { "PIXEL_FORMAT_YUVJ422P", AV_PIX_FMT_YUVJ422P },
        { "PIXEL_FORMAT_YUVJ444P", AV_PIX_FMT_YUVJ444P },
        { "PIXEL_FORMAT_UYVY422", AV_PIX_FMT_UYVY422 },
        { "PIXEL_FORMAT_UYYVYY411", AV_PIX_FMT_UYYVYY411 },
        { "PIXEL_FORMAT_BGR8", AV_PIX_FMT_BGR8 },
        { "PIXEL_FORMAT_BGR4", AV_PIX_FMT_BGR4 },
        { "PIXEL_FORMAT_BGR4_BYTE", AV_PIX_FMT_BGR4_BYTE },
        { "PIXEL_FORMAT_RGB8", AV_PIX_FMT_RGB8 },
        { "PIXEL_FORMAT_RGB4", AV_PIX_FMT_RGB4 },
        { "PIXEL_FORMAT_RGB4_BYTE", AV_PIX_FMT_RGB4_BYTE },
        { "PIXEL_FORMAT_NV12", AV_PIX_FMT_NV12 },
        { "PIXEL_FORMAT_NV21", AV_PIX_FMT_NV21 },
        { "PIXEL_FORMAT_ARGB", AV_PIX_FMT_ARGB },
        { "PIXEL_FORMAT_RGBA", AV_PIX_FMT_RGBA },
        { "PIXEL_FORMAT_ABGR", AV_PIX_FMT_ABGR },
        { "PIXEL_FORMAT_BGRA", AV_PIX_FMT_BGRA },
        { "PIXEL_FORMAT_GRAY16", AV_PIX_FMT_GRAY16 },
        { "PIXEL_FORMAT_YUV440P", AV_PIX_FMT_YUV440P },
        { "PIXEL_FORMAT_YUVJ440P", AV_PIX_FMT_YUVJ440P },
        { "PIXEL_FORMAT_YUVA420P", AV_PIX_FMT_YUVA420P },
        { "PIXEL_FORMAT_RGB48", AV_PIX_FMT_RGB48 },
        { "PIXEL_FORMAT_RGB565", AV_PIX_FMT_RGB565 },
        { "PIXEL_FORMAT_RGB555", AV_PIX_FMT_RGB555 },
        { "PIXEL_FORMAT_BGR565", AV_PIX_FMT_BGR565 },
        { "PIXEL_FORMAT_BGR555", AV_PIX_FMT_BGR555 },
        { "PIXEL_FORMAT_VAAPI", AV_PIX_FMT_VAAPI },
        { "PIXEL_FORMAT_YUV420P16", AV_PIX_FMT_YUV420P16 },
        { "PIXEL_FORMAT_YUV422P16", AV_PIX_FMT_YUV422P16 },
        { "PIXEL_FORMAT_YUV444P16", AV_PIX_FMT_YUV444P16 },
        { "PIXEL_FORMAT_DXVA2_VLD", AV_PIX_FMT_DXVA2_VLD },
        { "PIXEL_FORMAT_RGB444", AV_PIX_FMT_RGB444 },
        { "PIXEL_FORMAT_BGR444", AV_PIX_FMT_BGR444 },
        { "PIXEL_FORMAT_YA8", AV_PIX_FMT_YA8 },
        { "PIXEL_FORMAT_BGR48", AV_PIX_FMT_BGR48 },
        { "PIXEL_FORMAT_YUV420P9", AV_PIX_FMT_YUV420P9 },
        { "PIXEL_FORMAT_YUV420P10", AV_PIX_FMT_YUV420P10 },
        { "PIXEL_FORMAT_YUV422P10", AV_PIX_FMT_YUV422P10 },
        { "PIXEL_FORMAT_YUV444P9", AV_PIX_FMT_YUV444P9 },
        { "PIXEL_FORMAT_YUV444P10", AV_PIX_FMT_YUV444P10 },
        { "PIXEL_FORMAT_YUV422P9", AV_PIX_FMT_YUV422P9 },
        { "PIXEL_FORMAT_GBRP", AV_PIX_FMT_GBRP },
        { "PIXEL_FORMAT_GBRP9", AV_PIX_FMT_GBRP9 },
        { "PIXEL_FORMAT_GBRP10", AV_PIX_FMT_GBRP10 },
        { "PIXEL_FORMAT_GBRP16", AV_PIX_FMT_GBRP16 },
        { "PIXEL_FORMAT_YUVA422P", AV_PIX_FMT_YUVA422P },
        { "PIXEL_FORMAT_YUVA444P", AV_PIX_FMT_YUVA444P },
        { "PIXEL_FORMAT_YUVA420P9", AV_PIX_FMT_YUVA420P9 },
        { "PIXEL_FORMAT_YUVA422P9", AV_PIX_FMT_YUVA422P9 },
        { "PIXEL_FORMAT_YUVA444P9", AV_PIX_FMT_YUVA444P9 },
        { "PIXEL_FORMAT_YUVA420P10", AV_PIX_FMT_YUVA420P10 },
        { "PIXEL_FORMAT_YUVA422P10", AV_PIX_FMT_YUVA422P10 },
        { "PIXEL_FORMAT_YUVA444P10", AV_PIX_FMT_YUVA444P10 },
        { "PIXEL_FORMAT_YUVA420P16", AV_PIX_FMT_YUVA420P16 },
        { "PIXEL_FORMAT_YUVA422P16", AV_PIX_FMT_YUVA422P16 },
        { "PIXEL_FORMAT_YUVA444P16", AV_PIX_FMT_YUVA444P16 },
        { "PIXEL_FORMAT_VDPAU", AV_PIX_FMT_VDPAU },
        { "PIXEL_FORMAT_XYZ12", AV_PIX_FMT_XYZ12 },
        { "PIXEL_FORMAT_NV16", AV_PIX_FMT_NV16 },
        { "PIXEL_FORMAT_NV20", AV_PIX_FMT_NV20 },
        { "PIXEL_FORMAT_RGBA64", AV_PIX_FMT_RGBA64 },
        { "PIXEL_FORMAT_BGRA64", AV_PIX_FMT_BGRA64 },
        { "PIXEL_FORMAT_YVYU422", AV_PIX_FMT_YVYU422 },
        { "PIXEL_FORMAT_YA16", AV_PIX_FMT_YA16 },
        { "PIXEL_FORMAT_GBRAP", AV_PIX_FMT_GBRAP },
        { "PIXEL_FORMAT_GBRAP16", AV_PIX_FMT_GBRAP16 },
        { "PIXEL_FORMAT_QSV", AV_PIX_FMT_QSV },
        { "PIXEL_FORMAT_MMAL", AV_PIX_FMT_MMAL },
        { "PIXEL_FORMAT_D3D11VA_VLD", AV_PIX_FMT_D3D11VA_VLD },
        { "PIXEL_FORMAT_CUDA", AV_PIX_FMT_CUDA },
        { "PIXEL_FORMAT_0RGB", AV_PIX_FMT_0RGB },
        { "PIXEL_FORMAT_RGB0", AV_PIX_FMT_RGB0 },
        { "PIXEL_FORMAT_0BGR", AV_PIX_FMT_0BGR },
        { "PIXEL_FORMAT_BGR0", AV_PIX_FMT_BGR0 },
        { "PIXEL_FORMAT_YUV420P12", AV_PIX_FMT_YUV420P12 },
        { "PIXEL_FORMAT_YUV420P14", AV_PIX_FMT_YUV420P14 },
        { "PIXEL_FORMAT_YUV422P12", AV_PIX_FMT_YUV422P12 },
        { "PIXEL_FORMAT_YUV422P14", AV_PIX_FMT_YUV422P14 },
        { "PIXEL_FORMAT_YUV444P12", AV_PIX_FMT_YUV444P12 },
        { "PIXEL_FORMAT_YUV444P14", AV_PIX_FMT_YUV444P14 },
        { "PIXEL_FORMAT_GBRP12", AV_PIX_FMT_GBRP12 },
        { "PIXEL_FORMAT_GBRP14", AV_PIX_FMT_GBRP14 },
        { "PIXEL_FORMAT_YUVJ411P", AV_PIX_FMT_YUVJ411P },
        { "PIXEL_FORMAT_BAYER_BGGR8", AV_PIX_FMT_BAYER_BGGR8 },
        { "PIXEL_FORMAT_BAYER_RGGB8", AV_PIX_FMT_BAYER_RGGB8 },
        { "PIXEL_FORMAT_BAYER_GBRG8", AV_PIX_FMT_BAYER_GBRG8 },
        { "PIXEL_FORMAT_BAYER_GRBG8", AV_PIX_FMT_BAYER_GRBG8 },
        { "PIXEL_FORMAT_BAYER_BGGR16", AV_PIX_FMT_BAYER_BGGR16 },
        { "PIXEL_FORMAT_BAYER_RGGB16", AV_PIX_FMT_BAYER_RGGB16 },
        { "PIXEL_FORMAT_BAYER_GBRG16", AV_PIX_FMT_BAYER_GBRG16 },
        { "PIXEL_FORMAT_BAYER_GRBG16", AV_PIX_FMT_BAYER_GRBG16 },
        { "PIXEL_FORMAT_YUV440P10", AV_PIX_FMT_YUV440P10 },
        { "PIXEL_FORMAT_YUV440P12", AV_PIX_FMT_YUV440P12 },
        { "PIXEL_FORMAT_AYUV64", AV_PIX_FMT_AYUV64 },
        { "PIXEL_FORMAT_VIDEOTOOLBOX", AV_PIX_FMT_VIDEOTOOLBOX },
        { "PIXEL_FORMAT_P010", AV_PIX_FMT_P010 },
        { "PIXEL_FORMAT_GBRAP12", AV_PIX_FMT_GBRAP12 },
        { "PIXEL_FORMAT_GBRAP10", AV_PIX_FMT_GBRAP10 },
        { "PIXEL_FORMAT_MEDIACODEC", AV_PIX_FMT_MEDIACODEC },
        { "PIXEL_FORMAT_GRAY12", AV_PIX_FMT_GRAY12 },
        { "PIXEL_FORMAT_GRAY10", AV_PIX_FMT_GRAY10 },
        { "PIXEL_FORMAT_P016", AV_PIX_FMT_P016 },
        { "PIXEL_FORMAT_D3D11", AV_PIX_FMT_D3D11 },
        { "PIXEL_FORMAT_GRAY9", AV_PIX_FMT_GRAY9 },
        { "PIXEL_FORMAT_GBRPF32", AV_PIX_FMT_GBRPF32 },
        { "PIXEL_FORMAT_GBRAPF32", AV_PIX_FMT_GBRAPF32 },
        { "PIXEL_FORMAT_DRM_PRIME", AV_PIX_FMT_DRM_PRIME },
        { "PIXEL_FORMAT_OPENCL", AV_PIX_FMT_OPENCL },
        { "PIXEL_FORMAT_GRAY14", AV_PIX_FMT_GRAY14 },
        { "PIXEL_FORMAT_GRAYF32", AV_PIX_FMT_GRAYF32 },
        { "PIXEL_FORMAT_YUVA422P12", AV_PIX_FMT_YUVA422P12 },
        { "PIXEL_FORMAT_YUVA444P12", AV_PIX_FMT_YUVA444P12 },
        { "PIXEL_FORMAT_NV24", AV_PIX_FMT_NV24 },
        { "PIXEL_FORMAT_NV42", AV_PIX_FMT_NV42 },
        { "PIXEL_FORMAT_VULKAN", AV_PIX_FMT_VULKAN },
        { "PIXEL_FORMAT_Y210", AV_PIX_FMT_Y210 },
        { "PIXEL_FORMAT_X2RGB10", AV_PIX_FMT_X2RGB10 },
        { "PIXEL_FORMAT_X2BGR10", AV_PIX_FMT_X2BGR10 },
        { "PIXEL_FORMAT_P210", AV_PIX_FMT_P210 },
        { "PIXEL_FORMAT_P410", AV_PIX_FMT_P410 },
        { "PIXEL_FORMAT_P216", AV_PIX_FMT_P216 },
        { "PIXEL_FORMAT_P416", AV_PIX_FMT_P416 },
        { "PIXEL_FORMAT_VUYA", AV_PIX_FMT_VUYA },
        { "PIXEL_FORMAT_RGBAF16", AV_PIX_FMT_RGBAF16 },
        { "PIXEL_FORMAT_VUYX", AV_PIX_FMT_VUYX },
        { "PIXEL_FORMAT_P012", AV_PIX_FMT_P012 },
        { "PIXEL_FORMAT_Y212", AV_PIX_FMT_Y212 },
        { "PIXEL_FORMAT_XV30", AV_PIX_FMT_XV30 },
        { "PIXEL_FORMAT_XV36", AV_PIX_FMT_XV36 },
        { "PIXEL_FORMAT_RGBF32", AV_PIX_FMT_RGBF32 },
        { "PIXEL_FORMAT_RGBAF32", AV_PIX_FMT_RGBAF32 },

        { "PIXEL_COMPONENT_SELECTOR_LUMA", I(ComponentSelector::kLuma) },
        { "PIXEL_COMPONENT_SELECTOR_CHROMA_U", I(ComponentSelector::kChromaU) },
        { "PIXEL_COMPONENT_SELECTOR_CHROMA_V", I(ComponentSelector::kChromaV) },
        { "PIXEL_COMPONENT_SELECTOR_R", I(ComponentSelector::kR) },
        { "PIXEL_COMPONENT_SELECTOR_G", I(ComponentSelector::kG) },
        { "PIXEL_COMPONENT_SELECTOR_B", I(ComponentSelector::kB) },
        { "PIXEL_COMPONENT_SELECTOR_ALPHA", I(ComponentSelector::kAlpha) },

        { "STREAM_SELECTOR_VIDEO", I(utau::AVStreamDecoder::kVideo_StreamType) },
        { "STREAM_SELECTOR_AUDIO", I(utau::AVStreamDecoder::kAudio_StreamType) },

        { "DECODE_BUFFER_AUDIO", I(utau::AVStreamDecoder::AVGenericDecoded::kAudio) },
        { "DECODE_BUFFER_VIDEO", I(utau::AVStreamDecoder::AVGenericDecoded::kVideo) },
        { "DECODE_BUFFER_EOF",   I(utau::AVStreamDecoder::AVGenericDecoded::kEOF)   },
        { "DECODE_BUFFER_NULL",  I(utau::AVStreamDecoder::AVGenericDecoded::kNull)  },

        { "MEDIA_TYPE_VIDEO",    I(utau::MediaType::kVideo) },
        { "MEDIA_TYPE_AUDIO",    I(utau::MediaType::kAudio) },

        { "VIDEO_FRAME_TYPE_NONE", I(utau::VideoBufferInfo::FrameType::kNone) },
        { "VIDEO_FRAME_TYPE_I", I(utau::VideoBufferInfo::FrameType::kI) },
        { "VIDEO_FRAME_TYPE_B", I(utau::VideoBufferInfo::FrameType::kB) },
        { "VIDEO_FRAME_TYPE_P", I(utau::VideoBufferInfo::FrameType::kP) },
        { "VIDEO_FRAME_TYPE_S", I(utau::VideoBufferInfo::FrameType::kS) },
        { "VIDEO_FRAME_TYPE_BI", I(utau::VideoBufferInfo::FrameType::kBI) },
        { "VIDEO_FRAME_TYPE_SI", I(utau::VideoBufferInfo::FrameType::kSI) },
        { "VIDEO_FRAME_TYPE_SP", I(utau::VideoBufferInfo::FrameType::kSP) }
    };

#undef I

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    instance->Set(context, binder::to_v8(isolate, "Constants"),
                  binder::to_v8(isolate, constants)).Check();
}

v8::Local<v8::Object> MakeRational(v8::Isolate *i, int32_t num, int32_t denom)
{
    CHECK(i && "Invalid isolate");
    return binder::to_v8(i, std::unordered_map<std::string_view, v8::Local<v8::Value>>{
        { "num", binder::to_v8(i, num) },
        { "denom", binder::to_v8(i, denom) }
    });
}

utau::Ratio ExtractRational(v8::Isolate *i, v8::Local<v8::Value> v)
{
    CHECK(i && "Invalid isolate");

    utau::Ratio r;

    if (!v->IsObject())
        g_throw(TypeError, "Type `Rational` must be an object");

    auto obj = v8::Local<v8::Object>::Cast(v);
    CHECK(!obj.IsEmpty());

    auto maybe = obj->Get(i->GetCurrentContext(), binder::to_v8(i, "num"));
    if (maybe.IsEmpty())
        g_throw(TypeError, "Missing `num` property for `Rational` type");
    r.num = binder::from_v8<int32_t>(i, maybe.ToLocalChecked());

    maybe = obj->Get(i->GetCurrentContext(), binder::to_v8(i, "denom"));
    if (maybe.IsEmpty())
        g_throw(TypeError, "Missing `denom` property for `Rational` type");
    r.denom = binder::from_v8<int32_t>(i, maybe.ToLocalChecked());

    return r;
}

uint64_t getCurrentTimestampMs()
{
    return utau::GlobalContext::Ref().GetCurrentTimestampMs();
}

v8::Local<v8::Value> getPixelFormatDescriptor(int32_t fmt)
{
    using Map = std::unordered_map<std::string_view, v8::Local<v8::Value>>;

    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(
            static_cast<AVPixelFormat>(fmt));
    if (!desc)
        g_throw(RangeError, "Not a valid format enumeration value");

    bool has_palette = desc->flags & AV_PIX_FMT_FLAG_PAL;
    bool is_hwaccel = desc->flags & AV_PIX_FMT_FLAG_HWACCEL;
    bool is_planar = desc->flags & AV_PIX_FMT_FLAG_PLANAR;
    bool is_rgb_like = desc->flags & AV_PIX_FMT_FLAG_RGB;
    bool is_bayer = desc->flags & AV_PIX_FMT_FLAG_BAYER;
    bool has_alpha = desc->flags & AV_PIX_FMT_FLAG_ALPHA;
    bool is_float = desc->flags & AV_PIX_FMT_FLAG_FLOAT;

    int32_t planes = av_pix_fmt_count_planes(static_cast<AVPixelFormat>(fmt));
    int32_t bits_per_pixel = av_get_bits_per_pixel(desc);

    std::vector<v8::Local<v8::Value>> components(desc->nb_components);
    for (int32_t i = 0; i < desc->nb_components; i++)
    {
        components[i] = binder::to_v8(isolate, Map{
            { "plane", v8::Int32::New(isolate, desc->comp[i].plane) },
            { "step", v8::Int32::New(isolate, desc->comp[i].step) },
            { "offset", v8::Int32::New(isolate, desc->comp[i].offset) },
            { "shift", v8::Int32::New(isolate, desc->comp[i].shift) },
            { "depth", v8::Int32::New(isolate, desc->comp[i].depth) }
        });
    }

    return binder::to_v8(isolate, Map{
        { "name", binder::to_v8(isolate, desc->name) },
        { "components", binder::to_v8(isolate, components) },
        { "hasPalette", binder::to_v8(isolate, has_palette) },
        { "isHWAccel", binder::to_v8(isolate, is_hwaccel) },
        { "isPlanar", binder::to_v8(isolate, is_planar) },
        { "isRGBLike", binder::to_v8(isolate, is_rgb_like) },
        { "isBayer", binder::to_v8(isolate, is_bayer) },
        { "hasAlpha", binder::to_v8(isolate, has_alpha) },
        { "isFloat", binder::to_v8(isolate, is_float) },
        { "planes", binder::to_v8(isolate, planes) },
        { "bitsPerPixel", binder::to_v8(isolate, bits_per_pixel) },
        { "chromaWidthRShift", binder::to_v8(isolate, desc->log2_chroma_w) },
        { "chromaHeightRShift", binder::to_v8(isolate, desc->log2_chroma_h) }
    });
}

GALLIUM_BINDINGS_UTAU_NS_END
