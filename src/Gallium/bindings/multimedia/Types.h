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

#ifndef COCOA_GALLIUM_BINDINGS_MULTIMEDIA_TYPES_H
#define COCOA_GALLIUM_BINDINGS_MULTIMEDIA_TYPES_H

#include <cstdint>

#include "Gallium/bindings/multimedia/ffwrappers/libavutil.h"
#include "Gallium/bindings/multimedia/ffwrappers/libavformat.h"
#include "Gallium/ffi/JSObject.h"

#define GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN    namespace cocoa::gallium::bindings::multimedia {
#define GALLIUM_BINDINGS_MULTIMEDIA_NS_END      }

GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

ffi::Ret<AVRational> UnwrapJSRational(v8::Isolate *isolate, v8::Local<v8::Value> value);
v8::Local<v8::Object> CreateJSRational(v8::Isolate *isolate, const AVRational& q);

class AVRationalAdapter : public ffi::ArgAdapter
{
public:
    static ffi::Ret<AVRationalAdapter> Cast(
            v8::Isolate *isolate, v8::Local<v8::Value> value);

    const AVRational &operator*() const {
        return q;
    }

    AVRational q;
};

// The following are enum definitions

//! TSDecl: @enum MediaType
enum class MediaType
{
    //! TSDecl: @enumitem Unknown
    kUnknown = AVMEDIA_TYPE_UNKNOWN,
    //! TSDecl: @enumitem Video
    kVideo = AVMEDIA_TYPE_VIDEO,
    //! TSDecl: @enumitem Audio
    kAudio = AVMEDIA_TYPE_AUDIO,
    //! TSDecl: @enumitem Subtitle
    kSubtitle = AVMEDIA_TYPE_SUBTITLE,
    //! TSDecl: @enumitem Attachment
    kAttachment = AVMEDIA_TYPE_ATTACHMENT,
    //! TSDecl: @enumitem Data
    kData = AVMEDIA_TYPE_DATA
};
//! TSDecl: @end

//! TSDecl: @enum StreamDisposition
enum class StreamDisposition : uint32_t
{
    //! TSDecl: @enumitem Default
    kDefault = AV_DISPOSITION_DEFAULT,
    //! TSDecl: @enumitem Dub
    kDub = AV_DISPOSITION_DUB,
    //! TSDecl: @enumitem Original
    kOriginal = AV_DISPOSITION_ORIGINAL,
    //! TSDecl: @enumitem Comment
    kComment = AV_DISPOSITION_COMMENT,
    //! TSDecl: @enumitem Lyrics
    kLyrics = AV_DISPOSITION_LYRICS,
    //! TSDecl: @enumitem Karaoke
    kKaraoke = AV_DISPOSITION_KARAOKE,
    //! TSDecl: @enumitem Forced
    kForced = AV_DISPOSITION_FORCED,
    //! TSDecl: @enumitem HearingImpaired
    kHearingImpaired = AV_DISPOSITION_HEARING_IMPAIRED,
    //! TSDecl: @enumitem VisualImpaired
    kVisualImpaired = AV_DISPOSITION_VISUAL_IMPAIRED,
    //! TSDecl: @enumitem CleanEffects
    kCleanEffects = AV_DISPOSITION_CLEAN_EFFECTS,
    //! TSDecl: @enumitem AttachedPic
    kAttachedPic = AV_DISPOSITION_ATTACHED_PIC,
    //! TSDecl: @enumitem TimedThumbnails
    kTimedThumbnails = AV_DISPOSITION_TIMED_THUMBNAILS,
    //! TSDecl: @enumitem NonDiegetic
    kNonDiegetic = AV_DISPOSITION_NON_DIEGETIC,
    //! TSDecl: @enumitem Captions
    kCaptions = AV_DISPOSITION_CAPTIONS,
    //! TSDecl: @enumitem Descriptions
    kDescriptions = AV_DISPOSITION_DESCRIPTIONS,
    //! TSDecl: @enumitem Metadata
    kMetadata = AV_DISPOSITION_METADATA,
    //! TSDecl: @enumitem Dependent
    kDependent = AV_DISPOSITION_DEPENDENT,
    //! TSDecl: @enumitem StillImage
    kStillImage = AV_DISPOSITION_STILL_IMAGE
};
//! TSDecl: @end

//! TSDecl: @enum PixelFormat
enum class PixelFormat : int32_t
{
    //! TSDecl: @enumitem kNone
    kNone = AV_PIX_FMT_NONE,

    //! @tsdocbegin
    //! planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
    //! @tsdocend
    //! TSDecl: @enumitem kYUV420P
    kYUV420P = AV_PIX_FMT_YUV420P,
    //! @tsdocbegin
    //! packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr
    //! @tsdocend
    //! TSDecl: @enumitem kYUYV422
    kYUYV422 = AV_PIX_FMT_YUYV422,
    //! @tsdocbegin
    //! packed RGB 8:8:8, 24bpp, RGBRGB...
    //! @tsdocend
    //! TSDecl: @enumitem kRGB24
    kRGB24 = AV_PIX_FMT_RGB24,
    //! @tsdocbegin
    //! packed RGB 8:8:8, 24bpp, BGRBGR...
    //! @tsdocend
    //! TSDecl: @enumitem kBGR24
    kBGR24 = AV_PIX_FMT_BGR24,
    //! @tsdocbegin
    //! planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
    //! @tsdocend
    //! TSDecl: @enumitem kYUV422P
    kYUV422P = AV_PIX_FMT_YUV422P,
    //! @tsdocbegin
    //! planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
    //! @tsdocend
    //! TSDecl: @enumitem kYUV444P
    kYUV444P = AV_PIX_FMT_YUV444P,
    //! @tsdocbegin
    //! planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
    //! @tsdocend
    //! TSDecl: @enumitem kYUV410P
    kYUV410P = AV_PIX_FMT_YUV410P,
    //! @tsdocbegin
    //! planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)
    //! @tsdocend
    //! TSDecl: @enumitem kYUV411P
    kYUV411P = AV_PIX_FMT_YUV411P,
    //! @tsdocbegin
    //!        Y        ,  8bpp
    //! @tsdocend
    //! TSDecl: @enumitem kGRAY8
    kGRAY8 = AV_PIX_FMT_GRAY8,
    //! @tsdocbegin
    //!        Y        ,  1bpp, 0 is white, 1 is black, in each byte pixels are ordered from the msb to the lsb
    //! @tsdocend
    //! TSDecl: @enumitem kMONOWHITE
    kMONOWHITE = AV_PIX_FMT_MONOWHITE,
    //! @tsdocbegin
    //!        Y        ,  1bpp, 0 is black, 1 is white, in each byte pixels are ordered from the msb to the lsb
    //! @tsdocend
    //! TSDecl: @enumitem kMONOBLACK
    kMONOBLACK = AV_PIX_FMT_MONOBLACK,
    //! @tsdocbegin
    //! 8 bits with AV_PIX_FMT_RGB32 palette
    //! @tsdocend
    //! TSDecl: @enumitem kPAL8
    kPAL8 = AV_PIX_FMT_PAL8,
    //! @tsdocbegin
    //! packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1
    //! @tsdocend
    //! TSDecl: @enumitem kUYVY422
    kUYVY422 = AV_PIX_FMT_UYVY422,
    //! @tsdocbegin
    //! packed YUV 4:1:1, 12bpp, Cb Y0 Y1 Cr Y2 Y3
    //! @tsdocend
    //! TSDecl: @enumitem kUYYVYY411
    kUYYVYY411 = AV_PIX_FMT_UYYVYY411,
    //! @tsdocbegin
    //! packed RGB 3:3:2,  8bpp, (msb)2B 3G 3R(lsb)
    //! @tsdocend
    //! TSDecl: @enumitem kBGR8
    kBGR8 = AV_PIX_FMT_BGR8,
    //! @tsdocbegin
    //! packed RGB 1:2:1 bitstream,  4bpp, (msb)1B 2G 1R(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
    //! @tsdocend
    //! TSDecl: @enumitem kBGR4
    kBGR4 = AV_PIX_FMT_BGR4,
    //! @tsdocbegin
    //! packed RGB 1:2:1,  8bpp, (msb)1B 2G 1R(lsb)
    //! @tsdocend
    //! TSDecl: @enumitem kBGR4_BYTE
    kBGR4_BYTE = AV_PIX_FMT_BGR4_BYTE,
    //! @tsdocbegin
    //! packed RGB 3:3:2,  8bpp, (msb)2R 3G 3B(lsb)
    //! @tsdocend
    //! TSDecl: @enumitem kRGB8
    kRGB8 = AV_PIX_FMT_RGB8,
    //! @tsdocbegin
    //! packed RGB 1:2:1 bitstream,  4bpp, (msb)1R 2G 1B(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
    //! @tsdocend
    //! TSDecl: @enumitem kRGB4
    kRGB4 = AV_PIX_FMT_RGB4,
    //! @tsdocbegin
    //! packed RGB 1:2:1,  8bpp, (msb)1R 2G 1B(lsb)
    //! @tsdocend
    //! TSDecl: @enumitem kRGB4_BYTE
    kRGB4_BYTE = AV_PIX_FMT_RGB4_BYTE,
    //! @tsdocbegin
    //! planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V)
    //! @tsdocend
    //! TSDecl: @enumitem kNV12
    kNV12 = AV_PIX_FMT_NV12,
    //! @tsdocbegin
    //! as above, but U and V bytes are swapped
    //! @tsdocend
    //! TSDecl: @enumitem kNV21
    kNV21 = AV_PIX_FMT_NV21,
    //! @tsdocbegin
    //! packed ARGB 8:8:8:8, 32bpp, ARGBARGB...
    //! @tsdocend
    //! TSDecl: @enumitem kARGB
    kARGB = AV_PIX_FMT_ARGB,
    //! @tsdocbegin
    //! packed RGBA 8:8:8:8, 32bpp, RGBARGBA...
    //! @tsdocend
    //! TSDecl: @enumitem kRGBA
    kRGBA = AV_PIX_FMT_RGBA,
    //! @tsdocbegin
    //! packed ABGR 8:8:8:8, 32bpp, ABGRABGR...
    //! @tsdocend
    //! TSDecl: @enumitem kABGR
    kABGR = AV_PIX_FMT_ABGR,
    //! @tsdocbegin
    //! packed BGRA 8:8:8:8, 32bpp, BGRABGRA...
    //! @tsdocend
    //! TSDecl: @enumitem kBGRA
    kBGRA = AV_PIX_FMT_BGRA,
    //! @tsdocbegin
    //!        Y        , 16bpp, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGRAY16BE
    kGRAY16BE = AV_PIX_FMT_GRAY16BE,
    //! @tsdocbegin
    //!        Y        , 16bpp, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGRAY16LE
    kGRAY16LE = AV_PIX_FMT_GRAY16LE,
    //! @tsdocbegin
    //! planar YUV 4:4:0 (1 Cr & Cb sample per 1x2 Y samples)
    //! @tsdocend
    //! TSDecl: @enumitem kYUV440P
    kYUV440P = AV_PIX_FMT_YUV440P,
    //! @tsdocbegin
    //! planar YUV 4:2:0, 20bpp, (1 Cr & Cb sample per 2x2 Y & A samples)
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA420P
    kYUVA420P = AV_PIX_FMT_YUVA420P,
    //! @tsdocbegin
    //! packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kRGB48BE
    kRGB48BE = AV_PIX_FMT_RGB48BE,
    //! @tsdocbegin
    //! packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kRGB48LE
    kRGB48LE = AV_PIX_FMT_RGB48LE,
    //! @tsdocbegin
    //! packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kRGB565BE
    kRGB565BE = AV_PIX_FMT_RGB565BE,
    //! @tsdocbegin
    //! packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kRGB565LE
    kRGB565LE = AV_PIX_FMT_RGB565LE,
    //! @tsdocbegin
    //! packed RGB 5:5:5, 16bpp, (msb)1X 5R 5G 5B(lsb), big-endian   , X=unused/undefined
    //! @tsdocend
    //! TSDecl: @enumitem kRGB555BE
    kRGB555BE = AV_PIX_FMT_RGB555BE,
    //! @tsdocbegin
    //! packed RGB 5:5:5, 16bpp, (msb)1X 5R 5G 5B(lsb), little-endian, X=unused/undefined
    //! @tsdocend
    //! TSDecl: @enumitem kRGB555LE
    kRGB555LE = AV_PIX_FMT_RGB555LE,
    //! @tsdocbegin
    //! packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kBGR565BE
    kBGR565BE = AV_PIX_FMT_BGR565BE,
    //! @tsdocbegin
    //! packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kBGR565LE
    kBGR565LE = AV_PIX_FMT_BGR565LE,
    //! @tsdocbegin
    //! packed BGR 5:5:5, 16bpp, (msb)1X 5B 5G 5R(lsb), big-endian   , X=unused/undefined
    //! @tsdocend
    //! TSDecl: @enumitem kBGR555BE
    kBGR555BE = AV_PIX_FMT_BGR555BE,
    //! @tsdocbegin
    //! packed BGR 5:5:5, 16bpp, (msb)1X 5B 5G 5R(lsb), little-endian, X=unused/undefined
    //! @tsdocend
    //! TSDecl: @enumitem kBGR555LE
    kBGR555LE = AV_PIX_FMT_BGR555LE,
    //! @tsdocbegin
    //! planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV420P16LE
    kYUV420P16LE = AV_PIX_FMT_YUV420P16LE,
    //! @tsdocbegin
    //! planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV420P16BE
    kYUV420P16BE = AV_PIX_FMT_YUV420P16BE,
    //! @tsdocbegin
    //! planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV422P16LE
    kYUV422P16LE = AV_PIX_FMT_YUV422P16LE,
    //! @tsdocbegin
    //! planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV422P16BE
    kYUV422P16BE = AV_PIX_FMT_YUV422P16BE,
    //! @tsdocbegin
    //! planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV444P16LE
    kYUV444P16LE = AV_PIX_FMT_YUV444P16LE,
    //! @tsdocbegin
    //! planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV444P16BE
    kYUV444P16BE = AV_PIX_FMT_YUV444P16BE,
    //! @tsdocbegin
    //! HW decoding through DXVA2, Picture.data[3] contains a LPDIRECT3DSURFACE9 pointer
    //! @tsdocend
    //! TSDecl: @enumitem kDXVA2_VLD
    kDXVA2_VLD = AV_PIX_FMT_DXVA2_VLD,
    //! @tsdocbegin
    //! packed RGB 4:4:4, 16bpp, (msb)4X 4R 4G 4B(lsb), little-endian, X=unused/undefined
    //! @tsdocend
    //! TSDecl: @enumitem kRGB444LE
    kRGB444LE = AV_PIX_FMT_RGB444LE,
    //! @tsdocbegin
    //! packed RGB 4:4:4, 16bpp, (msb)4X 4R 4G 4B(lsb), big-endian,    X=unused/undefined
    //! @tsdocend
    //! TSDecl: @enumitem kRGB444BE
    kRGB444BE = AV_PIX_FMT_RGB444BE,
    //! @tsdocbegin
    //! packed BGR 4:4:4, 16bpp, (msb)4X 4B 4G 4R(lsb), little-endian, X=unused/undefined
    //! @tsdocend
    //! TSDecl: @enumitem kBGR444LE
    kBGR444LE = AV_PIX_FMT_BGR444LE,
    //! @tsdocbegin
    //! packed BGR 4:4:4, 16bpp, (msb)4X 4B 4G 4R(lsb), big-endian,    X=unused/undefined
    //! @tsdocend
    //! TSDecl: @enumitem kBGR444BE
    kBGR444BE = AV_PIX_FMT_BGR444BE,
    //! @tsdocbegin
    //! 8 bits gray, 8 bits alpha
    //! @tsdocend
    //! TSDecl: @enumitem kYA8
    kYA8 = AV_PIX_FMT_YA8,
    //! @tsdocbegin
    //! alias for kYA8
    //! @tsdocend
    //! TSDecl: @enumitem kY400A
    kY400A = AV_PIX_FMT_Y400A,
    //! @tsdocbegin
    //! alias for kYA8
    //! @tsdocend
    //! TSDecl: @enumitem kGRAY8A
    kGRAY8A = AV_PIX_FMT_GRAY8A,
    //! @tsdocbegin
    //! packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kBGR48BE
    kBGR48BE = AV_PIX_FMT_BGR48BE,
    //! @tsdocbegin
    //! packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kBGR48LE
    kBGR48LE = AV_PIX_FMT_BGR48LE,
    //! @tsdocbegin
    //! planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV420P9BE
    kYUV420P9BE = AV_PIX_FMT_YUV420P9BE,
    //! @tsdocbegin
    //! planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV420P9LE
    kYUV420P9LE = AV_PIX_FMT_YUV420P9LE,
    //! @tsdocbegin
    //! planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV420P10BE
    kYUV420P10BE = AV_PIX_FMT_YUV420P10BE,
    //! @tsdocbegin
    //! planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV420P10LE
    kYUV420P10LE = AV_PIX_FMT_YUV420P10LE,
    //! @tsdocbegin
    //! planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV422P10BE
    kYUV422P10BE = AV_PIX_FMT_YUV422P10BE,
    //! @tsdocbegin
    //! planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV422P10LE
    kYUV422P10LE = AV_PIX_FMT_YUV422P10LE,
    //! @tsdocbegin
    //! planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV444P9BE
    kYUV444P9BE = AV_PIX_FMT_YUV444P9BE,
    //! @tsdocbegin
    //! planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV444P9LE
    kYUV444P9LE = AV_PIX_FMT_YUV444P9LE,
    //! @tsdocbegin
    //! planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV444P10BE
    kYUV444P10BE = AV_PIX_FMT_YUV444P10BE,
    //! @tsdocbegin
    //! planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV444P10LE
    kYUV444P10LE = AV_PIX_FMT_YUV444P10LE,
    //! @tsdocbegin
    //! planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV422P9BE
    kYUV422P9BE = AV_PIX_FMT_YUV422P9BE,
    //! @tsdocbegin
    //! planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV422P9LE
    kYUV422P9LE = AV_PIX_FMT_YUV422P9LE,
    //! @tsdocbegin
    //! planar GBR 4:4:4 24bpp
    //! @tsdocend
    //! TSDecl: @enumitem kGBRP
    kGBRP = AV_PIX_FMT_GBRP,
    //! @tsdocbegin
    //! alias for kGBRP
    //! @tsdocend
    //! TSDecl: @enumitem kGBR24P
    kGBR24P = AV_PIX_FMT_GBR24P,
    //! @tsdocbegin
    //! planar GBR 4:4:4 27bpp, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGBRP9BE
    kGBRP9BE = AV_PIX_FMT_GBRP9BE,
    //! @tsdocbegin
    //! planar GBR 4:4:4 27bpp, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGBRP9LE
    kGBRP9LE = AV_PIX_FMT_GBRP9LE,
    //! @tsdocbegin
    //! planar GBR 4:4:4 30bpp, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGBRP10BE
    kGBRP10BE = AV_PIX_FMT_GBRP10BE,
    //! @tsdocbegin
    //! planar GBR 4:4:4 30bpp, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGBRP10LE
    kGBRP10LE = AV_PIX_FMT_GBRP10LE,
    //! @tsdocbegin
    //! planar GBR 4:4:4 48bpp, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGBRP16BE
    kGBRP16BE = AV_PIX_FMT_GBRP16BE,
    //! @tsdocbegin
    //! planar GBR 4:4:4 48bpp, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGBRP16LE
    kGBRP16LE = AV_PIX_FMT_GBRP16LE,
    //! @tsdocbegin
    //! planar YUV 4:2:2 24bpp, (1 Cr & Cb sample per 2x1 Y & A samples)
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA422P
    kYUVA422P = AV_PIX_FMT_YUVA422P,
    //! @tsdocbegin
    //! planar YUV 4:4:4 32bpp, (1 Cr & Cb sample per 1x1 Y & A samples)
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA444P
    kYUVA444P = AV_PIX_FMT_YUVA444P,
    //! @tsdocbegin
    //! planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA420P9BE
    kYUVA420P9BE = AV_PIX_FMT_YUVA420P9BE,
    //! @tsdocbegin
    //! planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA420P9LE
    kYUVA420P9LE = AV_PIX_FMT_YUVA420P9LE,
    //! @tsdocbegin
    //! planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA422P9BE
    kYUVA422P9BE = AV_PIX_FMT_YUVA422P9BE,
    //! @tsdocbegin
    //! planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA422P9LE
    kYUVA422P9LE = AV_PIX_FMT_YUVA422P9LE,
    //! @tsdocbegin
    //! planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA444P9BE
    kYUVA444P9BE = AV_PIX_FMT_YUVA444P9BE,
    //! @tsdocbegin
    //! planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA444P9LE
    kYUVA444P9LE = AV_PIX_FMT_YUVA444P9LE,
    //! @tsdocbegin
    //! planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA420P10BE
    kYUVA420P10BE = AV_PIX_FMT_YUVA420P10BE,
    //! @tsdocbegin
    //! planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA420P10LE
    kYUVA420P10LE = AV_PIX_FMT_YUVA420P10LE,
    //! @tsdocbegin
    //! planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA422P10BE
    kYUVA422P10BE = AV_PIX_FMT_YUVA422P10BE,
    //! @tsdocbegin
    //! planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA422P10LE
    kYUVA422P10LE = AV_PIX_FMT_YUVA422P10LE,
    //! @tsdocbegin
    //! planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA444P10BE
    kYUVA444P10BE = AV_PIX_FMT_YUVA444P10BE,
    //! @tsdocbegin
    //! planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA444P10LE
    kYUVA444P10LE = AV_PIX_FMT_YUVA444P10LE,
    //! @tsdocbegin
    //! planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA420P16BE
    kYUVA420P16BE = AV_PIX_FMT_YUVA420P16BE,
    //! @tsdocbegin
    //! planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA420P16LE
    kYUVA420P16LE = AV_PIX_FMT_YUVA420P16LE,
    //! @tsdocbegin
    //! planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA422P16BE
    kYUVA422P16BE = AV_PIX_FMT_YUVA422P16BE,
    //! @tsdocbegin
    //! planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA422P16LE
    kYUVA422P16LE = AV_PIX_FMT_YUVA422P16LE,
    //! @tsdocbegin
    //! planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA444P16BE
    kYUVA444P16BE = AV_PIX_FMT_YUVA444P16BE,
    //! @tsdocbegin
    //! planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA444P16LE
    kYUVA444P16LE = AV_PIX_FMT_YUVA444P16LE,
    //! @tsdocbegin
    //! packed XYZ 4:4:4, 36 bpp, (msb) 12X, 12Y, 12Z (lsb), the 2-byte value for each X/Y/Z is stored as little-endian, the 4 lower bits are set to 0
    //! @tsdocend
    //! TSDecl: @enumitem kXYZ12LE
    kXYZ12LE = AV_PIX_FMT_XYZ12LE,
    //! @tsdocbegin
    //! packed XYZ 4:4:4, 36 bpp, (msb) 12X, 12Y, 12Z (lsb), the 2-byte value for each X/Y/Z is stored as big-endian, the 4 lower bits are set to 0
    //! @tsdocend
    //! TSDecl: @enumitem kXYZ12BE
    kXYZ12BE = AV_PIX_FMT_XYZ12BE,
    //! @tsdocbegin
    //! interleaved chroma YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
    //! @tsdocend
    //! TSDecl: @enumitem kNV16
    kNV16 = AV_PIX_FMT_NV16,
    //! @tsdocbegin
    //! interleaved chroma YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kNV20LE
    kNV20LE = AV_PIX_FMT_NV20LE,
    //! @tsdocbegin
    //! interleaved chroma YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kNV20BE
    kNV20BE = AV_PIX_FMT_NV20BE,
    //! @tsdocbegin
    //! packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kRGBA64BE
    kRGBA64BE = AV_PIX_FMT_RGBA64BE,
    //! @tsdocbegin
    //! packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kRGBA64LE
    kRGBA64LE = AV_PIX_FMT_RGBA64LE,
    //! @tsdocbegin
    //! packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kBGRA64BE
    kBGRA64BE = AV_PIX_FMT_BGRA64BE,
    //! @tsdocbegin
    //! packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kBGRA64LE
    kBGRA64LE = AV_PIX_FMT_BGRA64LE,
    //! @tsdocbegin
    //! packed YUV 4:2:2, 16bpp, Y0 Cr Y1 Cb
    //! @tsdocend
    //! TSDecl: @enumitem kYVYU422
    kYVYU422 = AV_PIX_FMT_YVYU422,
    //! @tsdocbegin
    //! 16 bits gray, 16 bits alpha (big-endian)
    //! @tsdocend
    //! TSDecl: @enumitem kYA16BE
    kYA16BE = AV_PIX_FMT_YA16BE,
    //! @tsdocbegin
    //! 16 bits gray, 16 bits alpha (little-endian)
    //! @tsdocend
    //! TSDecl: @enumitem kYA16LE
    kYA16LE = AV_PIX_FMT_YA16LE,
    //! @tsdocbegin
    //! planar GBRA 4:4:4:4 32bpp
    //! @tsdocend
    //! TSDecl: @enumitem kGBRAP
    kGBRAP = AV_PIX_FMT_GBRAP,
    //! @tsdocbegin
    //! planar GBRA 4:4:4:4 64bpp, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGBRAP16BE
    kGBRAP16BE = AV_PIX_FMT_GBRAP16BE,
    //! @tsdocbegin
    //! planar GBRA 4:4:4:4 64bpp, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGBRAP16LE
    kGBRAP16LE = AV_PIX_FMT_GBRAP16LE,
    //! @tsdocbegin
    //! packed RGB 8:8:8, 32bpp, XRGBXRGB...   X=unused/undefined
    //! @tsdocend
    //! TSDecl: @enumitem k0RGB
    k0RGB = AV_PIX_FMT_0RGB,
    //! @tsdocbegin
    //! packed RGB 8:8:8, 32bpp, RGBXRGBX...   X=unused/undefined
    //! @tsdocend
    //! TSDecl: @enumitem kRGB0
    kRGB0 = AV_PIX_FMT_RGB0,
    //! @tsdocbegin
    //! packed BGR 8:8:8, 32bpp, XBGRXBGR...   X=unused/undefined
    //! @tsdocend
    //! TSDecl: @enumitem k0BGR
    k0BGR = AV_PIX_FMT_0BGR,
    //! @tsdocbegin
    //! packed BGR 8:8:8, 32bpp, BGRXBGRX...   X=unused/undefined
    //! @tsdocend
    //! TSDecl: @enumitem kBGR0
    kBGR0 = AV_PIX_FMT_BGR0,
    //! @tsdocbegin
    //! planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV420P12BE
    kYUV420P12BE = AV_PIX_FMT_YUV420P12BE,
    //! @tsdocbegin
    //! planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV420P12LE
    kYUV420P12LE = AV_PIX_FMT_YUV420P12LE,
    //! @tsdocbegin
    //! planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV420P14BE
    kYUV420P14BE = AV_PIX_FMT_YUV420P14BE,
    //! @tsdocbegin
    //! planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV420P14LE
    kYUV420P14LE = AV_PIX_FMT_YUV420P14LE,
    //! @tsdocbegin
    //! planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV422P12BE
    kYUV422P12BE = AV_PIX_FMT_YUV422P12BE,
    //! @tsdocbegin
    //! planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV422P12LE
    kYUV422P12LE = AV_PIX_FMT_YUV422P12LE,
    //! @tsdocbegin
    //! planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV422P14BE
    kYUV422P14BE = AV_PIX_FMT_YUV422P14BE,
    //! @tsdocbegin
    //! planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV422P14LE
    kYUV422P14LE = AV_PIX_FMT_YUV422P14LE,
    //! @tsdocbegin
    //! planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV444P12BE
    kYUV444P12BE = AV_PIX_FMT_YUV444P12BE,
    //! @tsdocbegin
    //! planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV444P12LE
    kYUV444P12LE = AV_PIX_FMT_YUV444P12LE,
    //! @tsdocbegin
    //! planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV444P14BE
    kYUV444P14BE = AV_PIX_FMT_YUV444P14BE,
    //! @tsdocbegin
    //! planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV444P14LE
    kYUV444P14LE = AV_PIX_FMT_YUV444P14LE,
    //! @tsdocbegin
    //! planar GBR 4:4:4 36bpp, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGBRP12BE
    kGBRP12BE = AV_PIX_FMT_GBRP12BE,
    //! @tsdocbegin
    //! planar GBR 4:4:4 36bpp, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGBRP12LE
    kGBRP12LE = AV_PIX_FMT_GBRP12LE,
    //! @tsdocbegin
    //! planar GBR 4:4:4 42bpp, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGBRP14BE
    kGBRP14BE = AV_PIX_FMT_GBRP14BE,
    //! @tsdocbegin
    //! planar GBR 4:4:4 42bpp, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGBRP14LE
    kGBRP14LE = AV_PIX_FMT_GBRP14LE,
    //! @tsdocbegin
    //! bayer, BGBG..(odd line), GRGR..(even line), 8-bit samples
    //! @tsdocend
    //! TSDecl: @enumitem kBAYER_BGGR8
    kBAYER_BGGR8 = AV_PIX_FMT_BAYER_BGGR8,
    //! @tsdocbegin
    //! bayer, RGRG..(odd line), GBGB..(even line), 8-bit samples
    //! @tsdocend
    //! TSDecl: @enumitem kBAYER_RGGB8
    kBAYER_RGGB8 = AV_PIX_FMT_BAYER_RGGB8,
    //! @tsdocbegin
    //! bayer, GBGB..(odd line), RGRG..(even line), 8-bit samples
    //! @tsdocend
    //! TSDecl: @enumitem kBAYER_GBRG8
    kBAYER_GBRG8 = AV_PIX_FMT_BAYER_GBRG8,
    //! @tsdocbegin
    //! bayer, GRGR..(odd line), BGBG..(even line), 8-bit samples
    //! @tsdocend
    //! TSDecl: @enumitem kBAYER_GRBG8
    kBAYER_GRBG8 = AV_PIX_FMT_BAYER_GRBG8,
    //! @tsdocbegin
    //! bayer, BGBG..(odd line), GRGR..(even line), 16-bit samples, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kBAYER_BGGR16LE
    kBAYER_BGGR16LE = AV_PIX_FMT_BAYER_BGGR16LE,
    //! @tsdocbegin
    //! bayer, BGBG..(odd line), GRGR..(even line), 16-bit samples, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kBAYER_BGGR16BE
    kBAYER_BGGR16BE = AV_PIX_FMT_BAYER_BGGR16BE,
    //! @tsdocbegin
    //! bayer, RGRG..(odd line), GBGB..(even line), 16-bit samples, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kBAYER_RGGB16LE
    kBAYER_RGGB16LE = AV_PIX_FMT_BAYER_RGGB16LE,
    //! @tsdocbegin
    //! bayer, RGRG..(odd line), GBGB..(even line), 16-bit samples, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kBAYER_RGGB16BE
    kBAYER_RGGB16BE = AV_PIX_FMT_BAYER_RGGB16BE,
    //! @tsdocbegin
    //! bayer, GBGB..(odd line), RGRG..(even line), 16-bit samples, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kBAYER_GBRG16LE
    kBAYER_GBRG16LE = AV_PIX_FMT_BAYER_GBRG16LE,
    //! @tsdocbegin
    //! bayer, GBGB..(odd line), RGRG..(even line), 16-bit samples, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kBAYER_GBRG16BE
    kBAYER_GBRG16BE = AV_PIX_FMT_BAYER_GBRG16BE,
    //! @tsdocbegin
    //! bayer, GRGR..(odd line), BGBG..(even line), 16-bit samples, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kBAYER_GRBG16LE
    kBAYER_GRBG16LE = AV_PIX_FMT_BAYER_GRBG16LE,
    //! @tsdocbegin
    //! bayer, GRGR..(odd line), BGBG..(even line), 16-bit samples, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kBAYER_GRBG16BE
    kBAYER_GRBG16BE = AV_PIX_FMT_BAYER_GRBG16BE,
    //! @tsdocbegin
    //! planar YUV 4:4:0,20bpp, (1 Cr & Cb sample per 1x2 Y samples), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV440P10LE
    kYUV440P10LE = AV_PIX_FMT_YUV440P10LE,
    //! @tsdocbegin
    //! planar YUV 4:4:0,20bpp, (1 Cr & Cb sample per 1x2 Y samples), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV440P10BE
    kYUV440P10BE = AV_PIX_FMT_YUV440P10BE,
    //! @tsdocbegin
    //! planar YUV 4:4:0,24bpp, (1 Cr & Cb sample per 1x2 Y samples), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV440P12LE
    kYUV440P12LE = AV_PIX_FMT_YUV440P12LE,
    //! @tsdocbegin
    //! planar YUV 4:4:0,24bpp, (1 Cr & Cb sample per 1x2 Y samples), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUV440P12BE
    kYUV440P12BE = AV_PIX_FMT_YUV440P12BE,
    //! @tsdocbegin
    //! packed AYUV 4:4:4,64bpp (1 Cr & Cb sample per 1x1 Y & A samples), little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kAYUV64LE
    kAYUV64LE = AV_PIX_FMT_AYUV64LE,
    //! @tsdocbegin
    //! packed AYUV 4:4:4,64bpp (1 Cr & Cb sample per 1x1 Y & A samples), big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kAYUV64BE
    kAYUV64BE = AV_PIX_FMT_AYUV64BE,
    //! @tsdocbegin
    //! like NV12, with 10bpp per component, data in the high bits, zeros in the low bits, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kP010LE
    kP010LE = AV_PIX_FMT_P010LE,
    //! @tsdocbegin
    //! like NV12, with 10bpp per component, data in the high bits, zeros in the low bits, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kP010BE
    kP010BE = AV_PIX_FMT_P010BE,
    //! @tsdocbegin
    //! planar GBR 4:4:4:4 48bpp, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGBRAP12BE
    kGBRAP12BE = AV_PIX_FMT_GBRAP12BE,
    //! @tsdocbegin
    //! planar GBR 4:4:4:4 48bpp, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGBRAP12LE
    kGBRAP12LE = AV_PIX_FMT_GBRAP12LE,
    //! @tsdocbegin
    //! planar GBR 4:4:4:4 40bpp, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGBRAP10BE
    kGBRAP10BE = AV_PIX_FMT_GBRAP10BE,
    //! @tsdocbegin
    //! planar GBR 4:4:4:4 40bpp, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGBRAP10LE
    kGBRAP10LE = AV_PIX_FMT_GBRAP10LE,
    //! @tsdocbegin
    //!        Y        , 12bpp, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGRAY12BE
    kGRAY12BE = AV_PIX_FMT_GRAY12BE,
    //! @tsdocbegin
    //!        Y        , 12bpp, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGRAY12LE
    kGRAY12LE = AV_PIX_FMT_GRAY12LE,
    //! @tsdocbegin
    //!        Y        , 10bpp, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGRAY10BE
    kGRAY10BE = AV_PIX_FMT_GRAY10BE,
    //! @tsdocbegin
    //!        Y        , 10bpp, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGRAY10LE
    kGRAY10LE = AV_PIX_FMT_GRAY10LE,
    //! @tsdocbegin
    //! like NV12, with 16bpp per component, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kP016LE
    kP016LE = AV_PIX_FMT_P016LE,
    //! @tsdocbegin
    //! like NV12, with 16bpp per component, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kP016BE
    kP016BE = AV_PIX_FMT_P016BE,
    //! @tsdocbegin
    //!        Y        , 9bpp, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGRAY9BE
    kGRAY9BE = AV_PIX_FMT_GRAY9BE,
    //! @tsdocbegin
    //!        Y        , 9bpp, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGRAY9LE
    kGRAY9LE = AV_PIX_FMT_GRAY9LE,
    //! @tsdocbegin
    //! IEEE-754 single precision planar GBR 4:4:4,     96bpp, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGBRPF32BE
    kGBRPF32BE = AV_PIX_FMT_GBRPF32BE,
    //! @tsdocbegin
    //! IEEE-754 single precision planar GBR 4:4:4,     96bpp, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGBRPF32LE
    kGBRPF32LE = AV_PIX_FMT_GBRPF32LE,
    //! @tsdocbegin
    //! IEEE-754 single precision planar GBRA 4:4:4:4, 128bpp, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGBRAPF32BE
    kGBRAPF32BE = AV_PIX_FMT_GBRAPF32BE,
    //! @tsdocbegin
    //! IEEE-754 single precision planar GBRA 4:4:4:4, 128bpp, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGBRAPF32LE
    kGBRAPF32LE = AV_PIX_FMT_GBRAPF32LE,
    //! @tsdocbegin
    //!        Y        , 14bpp, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGRAY14BE
    kGRAY14BE = AV_PIX_FMT_GRAY14BE,
    //! @tsdocbegin
    //!        Y        , 14bpp, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGRAY14LE
    kGRAY14LE = AV_PIX_FMT_GRAY14LE,
    //! @tsdocbegin
    //! IEEE-754 single precision Y, 32bpp, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGRAYF32BE
    kGRAYF32BE = AV_PIX_FMT_GRAYF32BE,
    //! @tsdocbegin
    //! IEEE-754 single precision Y, 32bpp, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kGRAYF32LE
    kGRAYF32LE = AV_PIX_FMT_GRAYF32LE,
    //! @tsdocbegin
    //! planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), 12b alpha, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA422P12BE
    kYUVA422P12BE = AV_PIX_FMT_YUVA422P12BE,
    //! @tsdocbegin
    //! planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), 12b alpha, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA422P12LE
    kYUVA422P12LE = AV_PIX_FMT_YUVA422P12LE,
    //! @tsdocbegin
    //! planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), 12b alpha, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA444P12BE
    kYUVA444P12BE = AV_PIX_FMT_YUVA444P12BE,
    //! @tsdocbegin
    //! planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), 12b alpha, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kYUVA444P12LE
    kYUVA444P12LE = AV_PIX_FMT_YUVA444P12LE,
    //! @tsdocbegin
    //! planar YUV 4:4:4, 24bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V)
    //! @tsdocend
    //! TSDecl: @enumitem kNV24
    kNV24 = AV_PIX_FMT_NV24,
    //! @tsdocbegin
    //! as above, but U and V bytes are swapped
    //! @tsdocend
    //! TSDecl: @enumitem kNV42
    kNV42 = AV_PIX_FMT_NV42,
    //! @tsdocbegin
    //! packed YUV 4:2:2 like YUYV422, 20bpp, data in the high bits, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kY210BE
    kY210BE = AV_PIX_FMT_Y210BE,
    //! @tsdocbegin
    //! packed YUV 4:2:2 like YUYV422, 20bpp, data in the high bits, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kY210LE
    kY210LE = AV_PIX_FMT_Y210LE,
    //! @tsdocbegin
    //! packed RGB 10:10:10, 30bpp, (msb)2X 10R 10G 10B(lsb), little-endian, X=unused/undefined
    //! @tsdocend
    //! TSDecl: @enumitem kX2RGB10LE
    kX2RGB10LE = AV_PIX_FMT_X2RGB10LE,
    //! @tsdocbegin
    //! packed RGB 10:10:10, 30bpp, (msb)2X 10R 10G 10B(lsb), big-endian, X=unused/undefined
    //! @tsdocend
    //! TSDecl: @enumitem kX2RGB10BE
    kX2RGB10BE = AV_PIX_FMT_X2RGB10BE,
    //! @tsdocbegin
    //! packed BGR 10:10:10, 30bpp, (msb)2X 10B 10G 10R(lsb), little-endian, X=unused/undefined
    //! @tsdocend
    //! TSDecl: @enumitem kX2BGR10LE
    kX2BGR10LE = AV_PIX_FMT_X2BGR10LE,
    //! @tsdocbegin
    //! packed BGR 10:10:10, 30bpp, (msb)2X 10B 10G 10R(lsb), big-endian, X=unused/undefined
    //! @tsdocend
    //! TSDecl: @enumitem kX2BGR10BE
    kX2BGR10BE = AV_PIX_FMT_X2BGR10BE,
    //! @tsdocbegin
    //! interleaved chroma YUV 4:2:2, 20bpp, data in the high bits, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kP210BE
    kP210BE = AV_PIX_FMT_P210BE,
    //! @tsdocbegin
    //! interleaved chroma YUV 4:2:2, 20bpp, data in the high bits, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kP210LE
    kP210LE = AV_PIX_FMT_P210LE,
    //! @tsdocbegin
    //! interleaved chroma YUV 4:4:4, 30bpp, data in the high bits, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kP410BE
    kP410BE = AV_PIX_FMT_P410BE,
    //! @tsdocbegin
    //! interleaved chroma YUV 4:4:4, 30bpp, data in the high bits, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kP410LE
    kP410LE = AV_PIX_FMT_P410LE,
    //! @tsdocbegin
    //! interleaved chroma YUV 4:2:2, 32bpp, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kP216BE
    kP216BE = AV_PIX_FMT_P216BE,
    //! @tsdocbegin
    //! interleaved chroma YUV 4:2:2, 32bpp, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kP216LE
    kP216LE = AV_PIX_FMT_P216LE,
    //! @tsdocbegin
    //! interleaved chroma YUV 4:4:4, 48bpp, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kP416BE
    kP416BE = AV_PIX_FMT_P416BE,
    //! @tsdocbegin
    //! interleaved chroma YUV 4:4:4, 48bpp, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kP416LE
    kP416LE = AV_PIX_FMT_P416LE,
    //! @tsdocbegin
    //! packed VUYA 4:4:4, 32bpp, VUYAVUYA...
    //! @tsdocend
    //! TSDecl: @enumitem kVUYA
    kVUYA = AV_PIX_FMT_VUYA,
    //! @tsdocbegin
    //! IEEE-754 half precision packed RGBA 16:16:16:16, 64bpp, RGBARGBA..., big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kRGBAF16BE
    kRGBAF16BE = AV_PIX_FMT_RGBAF16BE,
    //! @tsdocbegin
    //! IEEE-754 half precision packed RGBA 16:16:16:16, 64bpp, RGBARGBA..., little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kRGBAF16LE
    kRGBAF16LE = AV_PIX_FMT_RGBAF16LE,
    //! @tsdocbegin
    //! packed VUYX 4:4:4, 32bpp, Variant of VUYA where alpha channel is left undefined
    //! @tsdocend
    //! TSDecl: @enumitem kVUYX
    kVUYX = AV_PIX_FMT_VUYX,
    //! @tsdocbegin
    //! like NV12, with 12bpp per component, data in the high bits, zeros in the low bits, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kP012LE
    kP012LE = AV_PIX_FMT_P012LE,
    //! @tsdocbegin
    //! like NV12, with 12bpp per component, data in the high bits, zeros in the low bits, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kP012BE
    kP012BE = AV_PIX_FMT_P012BE,
    //! @tsdocbegin
    //! packed YUV 4:2:2 like YUYV422, 24bpp, data in the high bits, zeros in the low bits, big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kY212BE
    kY212BE = AV_PIX_FMT_Y212BE,
    //! @tsdocbegin
    //! packed YUV 4:2:2 like YUYV422, 24bpp, data in the high bits, zeros in the low bits, little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kY212LE
    kY212LE = AV_PIX_FMT_Y212LE,
    //! @tsdocbegin
    //! packed XVYU 4:4:4, 32bpp, (msb)2X 10V 10Y 10U(lsb), big-endian, variant of Y410 where alpha channel is left undefined
    //! @tsdocend
    //! TSDecl: @enumitem kXV30BE
    kXV30BE = AV_PIX_FMT_XV30BE,
    //! @tsdocbegin
    //! packed XVYU 4:4:4, 32bpp, (msb)2X 10V 10Y 10U(lsb), little-endian, variant of Y410 where alpha channel is left undefined
    //! @tsdocend
    //! TSDecl: @enumitem kXV30LE
    kXV30LE = AV_PIX_FMT_XV30LE,
    //! @tsdocbegin
    //! packed XVYU 4:4:4, 48bpp, data in the high bits, zeros in the low bits, big-endian, variant of Y412 where alpha channel is left undefined
    //! @tsdocend
    //! TSDecl: @enumitem kXV36BE
    kXV36BE = AV_PIX_FMT_XV36BE,
    //! @tsdocbegin
    //! packed XVYU 4:4:4, 48bpp, data in the high bits, zeros in the low bits, little-endian, variant of Y412 where alpha channel is left undefined
    //! @tsdocend
    //! TSDecl: @enumitem kXV36LE
    kXV36LE = AV_PIX_FMT_XV36LE,
    //! @tsdocbegin
    //! IEEE-754 single precision packed RGB 32:32:32, 96bpp, RGBRGB..., big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kRGBF32BE
    kRGBF32BE = AV_PIX_FMT_RGBF32BE,
    //! @tsdocbegin
    //! IEEE-754 single precision packed RGB 32:32:32, 96bpp, RGBRGB..., little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kRGBF32LE
    kRGBF32LE = AV_PIX_FMT_RGBF32LE,
    //! @tsdocbegin
    //! IEEE-754 single precision packed RGBA 32:32:32:32, 128bpp, RGBARGBA..., big-endian
    //! @tsdocend
    //! TSDecl: @enumitem kRGBAF32BE
    kRGBAF32BE = AV_PIX_FMT_RGBAF32BE,
    //! @tsdocbegin
    //! IEEE-754 single precision packed RGBA 32:32:32:32, 128bpp, RGBARGBA..., little-endian
    //! @tsdocend
    //! TSDecl: @enumitem kRGBAF32LE
    kRGBAF32LE = AV_PIX_FMT_RGBAF32LE,
};
//! TSDecl: @end

//! TSDecl: @enum FieldOrder
enum class FieldOrder
{
    //! TSDecl: @enumitem Unknown
    kUnknown = AV_FIELD_UNKNOWN,
    //! TSDecl: @enumitem Progressive
    kProgressive = AV_FIELD_PROGRESSIVE,

    //! @tsdocbegin
    //! Top coded first, top displayed first
    //! @tsdocend
    //! TSDecl: @enumitem TT
    kTT = AV_FIELD_TT,

    //! @tsdocbegin
    //! Bottom coded first, bottom displayed first
    //! @tsdocend
    //! TSDecl: @enumitem BB
    kBB = AV_FIELD_BB,

    //! @tsdocbegin
    //! Top coded first, bottom displayed first
    //! @tsdocend
    //! TSDecl: @enumitem TB
    kTB = AV_FIELD_TB,

    //! @tsdocbegin
    //! Bottom coded first, top displayed first
    //! @tsdocend
    //! TSDecl: @enumitem BT
    kBT = AV_FIELD_BT
};
//! TSDecl: @end

//! @tsdocbegin
//! Chromaticity coordinates of the source primaries.
//! These values match the ones defined by ISO/IEC 23091-2_2019 subclause 8.1 and ITU-T H.273.
//! @tsdocend
//! TSDecl: @enum ColorPrimaries
enum class ColorPrimaries
{
    //! TSDecl: @enumitem Reserved0
    kReserved0 = AVCOL_PRI_RESERVED0,

    //! TSDecl: @enumitem Unspecified
    kUnspecified = AVCOL_PRI_UNSPECIFIED,

    //! TSDecl: @enumitem Reserved
    kReserved = AVCOL_PRI_RESERVED,

    //! @tsdocbegin
    //! also ITU-R BT1361 / IEC 61966-2-4 / SMPTE RP 177 Annex B
    //! @tsdocend
    //! TSDecl: @enumitem BT709
    kBT709 = AVCOL_PRI_BT709,
    //! @tsdocbegin
    //! also FCC Title 47 Code of Federal Regulations 73.682 (a)(20)
    //! @tsdocend
    //! TSDecl: @enumitem BT470M
    kBT470M = AVCOL_PRI_BT470M,
    //! @tsdocbegin
    //! also ITU-R BT601-6 625 / ITU-R BT1358 625 / ITU-R BT1700 625 PAL & SECAM
    //! @tsdocend
    //! TSDecl: @enumitem BT470BG
    kBT470BG = AVCOL_PRI_BT470BG,
    //! @tsdocbegin
    //! also ITU-R BT601-6 525 / ITU-R BT1358 525 / ITU-R BT1700 NTSC
    //! @tsdocend
    //! TSDecl: @enumitem SMPTE170M
    kSMPTE170M = AVCOL_PRI_SMPTE170M,
    //! @tsdocbegin
    //! identical to above, also called "SMPTE C" even though it uses D65
    //! @tsdocend
    //! TSDecl: @enumitem SMPTE240M
    kSMPTE240M = AVCOL_PRI_SMPTE240M,
    //! @tsdocbegin
    //! colour filters using Illuminant C
    //! @tsdocend
    //! TSDecl: @enumitem FILM
    kFILM = AVCOL_PRI_FILM,
    //! @tsdocbegin
    //! ITU-R BT2020
    //! @tsdocend
    //! TSDecl: @enumitem BT2020
    kBT2020 = AVCOL_PRI_BT2020,
    //! @tsdocbegin
    //! SMPTE ST 428-1 (CIE 1931 XYZ)
    //! @tsdocend
    //! TSDecl: @enumitem SMPTE428
    kSMPTE428 = AVCOL_PRI_SMPTE428,
    //! @tsdocbegin
    //! alias of SMPTE428,
    //! @tsdocend
    //! TSDecl: @enumitem SMPTEST428_1
    kSMPTEST428_1 = AVCOL_PRI_SMPTEST428_1,
    //! @tsdocbegin
    //! SMPTE ST 431-2 (2011) / DCI P3
    //! @tsdocend
    //! TSDecl: @enumitem SMPTE431
    kSMPTE431 = AVCOL_PRI_SMPTE431,
    //! @tsdocbegin
    //! SMPTE ST 432-1 (2010) / P3 D65 / Display P3
    //! @tsdocend
    //! TSDecl: @enumitem SMPTE432
    kSMPTE432 = AVCOL_PRI_SMPTE432,
    //! @tsdocbegin
    //! EBU Tech. 3213-E (nothing there) / one of JEDEC P22 group phosphors
    //! @tsdocend
    //! TSDecl: @enumitem EBU3213
    kEBU3213 = AVCOL_PRI_EBU3213,
    //! @tsdocbegin
    //! alias of EBU3213
    //! @tsdocend
    //! TSDecl: @enumitem JEDEC_P22
    kJEDEC_P22 = AVCOL_PRI_JEDEC_P22
};
//! TSDecl: @end

//! @tsdocbegin
//! Color Transfer Characteristic.
//! These values match the ones defined by ISO/IEC 23091-2_2019 subclause 8.2.
//! @tsdocend
//! TSDecl: @enum ColorTransferCharacteristic
enum class ColorTransferCharacteristic
{
    //! TSDecl: @enumitem Reserved0
    kReserved0 = AVCOL_TRC_RESERVED0,
    //! TSDecl: @enumitem Unspecified
    kUnspecified = AVCOL_TRC_UNSPECIFIED,
    //! TSDecl: @enumitem Reserved
    kReserved = AVCOL_TRC_RESERVED,
    //! @tsdocbegin
    //! also ITU-R BT1361
    //! @tsdocend
    //! TSDecl: @enumitem BT709
    kBT709 = AVCOL_TRC_BT709,
    //! @tsdocbegin
    //! also ITU-R BT470M / ITU-R BT1700 625 PAL & SECAM
    //! @tsdocend
    //! TSDecl: @enumitem GAMMA22
    kGAMMA22 = AVCOL_TRC_GAMMA22,
    //! @tsdocbegin
    //! also ITU-R BT470BG
    //! @tsdocend
    //! TSDecl: @enumitem GAMMA28
    kGAMMA28 = AVCOL_TRC_GAMMA28,
    //! @tsdocbegin
    //! also ITU-R BT601-6 525 or 625 / ITU-R BT1358 525 or 625 / ITU-R BT1700 NTSC
    //! @tsdocend
    //! TSDecl: @enumitem SMPTE170M
    kSMPTE170M = AVCOL_TRC_SMPTE170M,
    //! TSDecl: @enumitem SMPTE240M
    kSMPTE240M = AVCOL_TRC_SMPTE240M,
    //! @tsdocbegin
    //! "Linear transfer characteristics"
    //! @tsdocend
    //! TSDecl: @enumitem LINEAR
    kLINEAR = AVCOL_TRC_LINEAR,
    //! @tsdocbegin
    //! "Logarithmic transfer characteristic (100:1 range)"
    //! @tsdocend
    //! TSDecl: @enumitem LOG
    kLOG = AVCOL_TRC_LOG,
    //! @tsdocbegin
    //! "Logarithmic transfer characteristic (100 * Sqrt(10) : 1 range)"
    //! @tsdocend
    //! TSDecl: @enumitem LOG_SQRT
    kLOG_SQRT = AVCOL_TRC_LOG_SQRT,
    //! @tsdocbegin
    //! IEC 61966-2-4
    //! @tsdocend
    //! TSDecl: @enumitem IEC61966_2_4
    kIEC61966_2_4 = AVCOL_TRC_IEC61966_2_4,
    //! @tsdocbegin
    //! ITU-R BT1361 Extended Colour Gamut
    //! @tsdocend
    //! TSDecl: @enumitem BT1361_ECG
    kBT1361_ECG = AVCOL_TRC_BT1361_ECG,
    //! @tsdocbegin
    //! IEC 61966-2-1 (sRGB or sYCC)
    //! @tsdocend
    //! TSDecl: @enumitem IEC61966_2_1
    kIEC61966_2_1 = AVCOL_TRC_IEC61966_2_1,
    //! @tsdocbegin
    //! ITU-R BT2020 for 10-bit system
    //! @tsdocend
    //! TSDecl: @enumitem BT2020_10
    kBT2020_10 = AVCOL_TRC_BT2020_10,
    //! @tsdocbegin
    //! ITU-R BT2020 for 12-bit system
    //! @tsdocend
    //! TSDecl: @enumitem BT2020_12
    kBT2020_12 = AVCOL_TRC_BT2020_12,
    //! @tsdocbegin
    //! SMPTE ST 2084 for 10-, 12-, 14- and 16-bit systems
    //! @tsdocend
    //! TSDecl: @enumitem SMPTE2084
    kSMPTE2084 = AVCOL_TRC_SMPTE2084,
    //! @tsdocbegin
    //! alias of SMPTE2084,
    //! @tsdocend
    //! TSDecl: @enumitem SMPTEST2084
    kSMPTEST2084 = AVCOL_TRC_SMPTEST2084,
    //! @tsdocbegin
    //! SMPTE ST 428-1
    //! @tsdocend
    //! TSDecl: @enumitem SMPTE428
    kSMPTE428 = AVCOL_TRC_SMPTE428,
    //! @tsdocbegin
    //! alias of SMPTE428,
    //! @tsdocend
    //! TSDecl: @enumitem SMPTEST428_1
    kSMPTEST428_1 = AVCOL_TRC_SMPTEST428_1,
    //! @tsdocbegin
    //! ARIB STD-B67, known as "Hybrid log-gamma"
    //! @tsdocend
    //! TSDecl: @enumitem ARIB_STD_B67
    kARIB_STD_B67 = AVCOL_TRC_ARIB_STD_B67
};
//! TSDecl: @end

//! @tsdocbegin
//! YUV colorspace type.
//! These values match the ones defined by ISO/IEC 23091-2_2019 subclause 8.3.
//! @tsdocend
//! TSDecl: @enum ColorSpace
enum class ColorSpace
{
    //! @tsdocbegin
    //! order of coefficients is actually GBR, also IEC 61966-2-1 (sRGB), YZX and ST 428-1
    //! @tsdocend
    //! TSDecl: @enumitem RGB
    kRGB = AVCOL_SPC_RGB,
    //! @tsdocbegin
    //! also ITU-R BT1361 / IEC 61966-2-4 xvYCC709 / derived in SMPTE RP 177 Annex B
    //! @tsdocend
    //! TSDecl: @enumitem BT709
    kBT709 = AVCOL_SPC_BT709,
    //! TSDecl: @enumitem Unspecified
    kUnspecified = AVCOL_SPC_UNSPECIFIED,
    //! @tsdocbegin
    //! reserved for future use by ITU-T and ISO/IEC just like 15-255 are
    //! @tsdocend
    //! TSDecl: @enumitem Reserved
    kReserved = AVCOL_SPC_RESERVED,
    //! @tsdocbegin
    //! FCC Title 47 Code of Federal Regulations 73.682 (a)(20)
    //! @tsdocend
    //! TSDecl: @enumitem FCC
    kFCC = AVCOL_SPC_FCC,
    //! @tsdocbegin
    //! also ITU-R BT601-6 625 / ITU-R BT1358 625 / ITU-R BT1700 625 PAL & SECAM / IEC 61966-2-4 xvYCC601
    //! @tsdocend
    //! TSDecl: @enumitem BT470BG
    kBT470BG = AVCOL_SPC_BT470BG,
    //! @tsdocbegin
    //! also ITU-R BT601-6 525 / ITU-R BT1358 525 / ITU-R BT1700 NTSC / functionally identical to above
    //! @tsdocend
    //! TSDecl: @enumitem SMPTE170M
    kSMPTE170M = AVCOL_SPC_SMPTE170M,
    //! @tsdocbegin
    //! derived from 170M primaries and D65 white point, 170M is derived from BT470 System M's primaries
    //! @tsdocend
    //! TSDecl: @enumitem SMPTE240M
    kSMPTE240M = AVCOL_SPC_SMPTE240M,
    //! @tsdocbegin
    //! used by Dirac / VC-2 and H.264 FRext, see ITU-T SG16
    //! @tsdocend
    //! TSDecl: @enumitem YCGCO
    kYCGCO = AVCOL_SPC_YCGCO,
    //! @tsdocbegin
    //! alias of YCGCO,
    //! @tsdocend
    //! TSDecl: @enumitem YCOCG
    kYCOCG = AVCOL_SPC_YCOCG,
    //! @tsdocbegin
    //! ITU-R BT2020 non-constant luminance system
    //! @tsdocend
    //! TSDecl: @enumitem BT2020_NCL
    kBT2020_NCL = AVCOL_SPC_BT2020_NCL,
    //! @tsdocbegin
    //! ITU-R BT2020 constant luminance system
    //! @tsdocend
    //! TSDecl: @enumitem BT2020_CL
    kBT2020_CL = AVCOL_SPC_BT2020_CL,
    //! @tsdocbegin
    //! SMPTE 2085, Y'D'zD'x
    //! @tsdocend
    //! TSDecl: @enumitem SMPTE2085
    kSMPTE2085 = AVCOL_SPC_SMPTE2085,
    //! @tsdocbegin
    //! Chromaticity-derived non-constant luminance system
    //! @tsdocend
    //! TSDecl: @enumitem CHROMA_DERIVED_NCL
    kCHROMA_DERIVED_NCL = AVCOL_SPC_CHROMA_DERIVED_NCL,
    //! @tsdocbegin
    //! Chromaticity-derived constant luminance system
    //! @tsdocend
    //! TSDecl: @enumitem CHROMA_DERIVED_CL
    kCHROMA_DERIVED_CL = AVCOL_SPC_CHROMA_DERIVED_CL,
    //! @tsdocbegin
    //! ITU-R BT.2100-0, ICtCp
    //! @tsdocend
    //! TSDecl: @enumitem ICTCP
    kICTCP = AVCOL_SPC_ICTCP
};
//! TSDecl: @end

//! @tsdocbegin
//! Visual content value range.
//!
//! These values are based on definitions that can be found in multiple
//! specifications, such as ITU-T BT.709 (3.4 - Quantization of RGB, luminance
//! and colour-difference signals), ITU-T BT.2020 (Table 5 - Digital
//! Representation) as well as ITU-T BT.2100 (Table 9 - Digital 10- and 12-bit
//! integer representation). At the time of writing, the BT.2100 one is
//! recommended, as it also defines the full range representation.
//!
//! Common definitions:
//!   - For RGB and luma planes such as Y in YCbCr and I in ICtCp,
//!     'E' is the original value in range of 0.0 to 1.0.
//!   - For chroma planes such as Cb,Cr and Ct,Cp, 'E' is the original
//!     value in range of -0.5 to 0.5.
//!   - 'n' is the output bit depth.
//!   - For additional definitions such as rounding and clipping to valid n
//!     bit unsigned integer range, please refer to BT.2100 (Table 9).
//! @tsdocend
//! TSDecl: @enum ColorRange
enum class ColorRange
{
    //! TSDecl: @enumitem Unspecified
    kUnspecified = AVCOL_RANGE_UNSPECIFIED,
    
    //! @tsdocbegin
    //! Narrow or limited range content.
    //!
    //! - For luma planes:
    //!
    //!       (219 * E + 16) * 2^(n-8)
    //!
    //!   F.ex. the range of 16-235 for 8 bits
    //!
    //! - For chroma planes:
    //!
    //!       (224 * E + 128) * 2^(n-8)
    //!
    //!   F.ex. the range of 16-240 for 8 bits
    //! @tsdocend
    //! TSDecl: @enumitem MPEG
    kMPEG = AVCOL_RANGE_MPEG,
    
    //! @tsdocbegin
    //! Full range content.
    //!
    //! - For RGB and luma planes:
    //!
    //!       (2^n - 1) * E
    //!
    //!   F.ex. the range of 0-255 for 8 bits
    //!
    //! - For chroma planes:
    //!
    //!       (2^n - 1) * E + 2^(n - 1)
    //!
    //!   F.ex. the range of 1-255 for 8 bits
    //! @tsdocend
    //! TSDecl: @enumitem JPEG
    kJPEG = AVCOL_RANGE_JPEG
};
//! TSDecl: @end

//! @tsdocbegin
//! Location of chroma samples.
//!
//! Illustration showing the location of the first (top left) chroma sample of the
//! image, the left shows only luma, the right
//! shows the location of the chroma sample, the 2 could be imagined to overlay
//! each other but are drawn separately due to limitations of ASCII
//!
//!                 1st 2nd       1st 2nd horizontal luma sample positions
//!                  v   v         v   v
//!                  ______        ______
//! 1st luma line > |X   X ...    |3 4 X ...     X are luma samples,
//!                 |             |1 2           1-6 are possible chroma positions
//! 2nd luma line > |X   X ...    |5 6 X ...     0 is undefined/unknown position
//! @tsdocend
//! TSDecl: @enum ChromaLocation
enum class ChromaLocation
{
    //! TSDecl: @enumitem Unspecified
    kUnspecified = AVCHROMA_LOC_UNSPECIFIED,
    //! @tsdocbegin
    //! MPEG-2/4 4:2:0, H.264 default for 4:2:0
    //! @tsdocend
    //! TSDecl: @enumitem Left
    kLeft = AVCHROMA_LOC_LEFT,
    //! @tsdocbegin
    //! MPEG-1 4:2:0, JPEG 4:2:0, H.263 4:2:0
    //! @tsdocend
    //! TSDecl: @enumitem Center
    kCenter = AVCHROMA_LOC_CENTER,
    //! @tsdocbegin
    //! ITU-R 601, SMPTE 274M 296M S314M(DV 4:1:1), mpeg2 4:2:2
    //! @tsdocend
    //! TSDecl: @enumitem TopLeft
    kTopLeft = AVCHROMA_LOC_TOPLEFT,
    //! TSDecl: @enumitem Top
    kTop = AVCHROMA_LOC_TOP,
    //! TSDecl: @enumitem BottomLeft
    kBottomLeft = AVCHROMA_LOC_BOTTOMLEFT,
    //! TSDecl: @enumitem Bottom
    kBottom = AVCHROMA_LOC_BOTTOM
};
//! TSDecl: @end

//! @tsdocbegin
//! Audio sample formats
//!
//! - The data described by the sample format is always in native-endian order.
//!   Sample values can be expressed by native C types, hence the lack of a signed
//!   24-bit sample format even though it is a common raw audio data format.
//!
//! - The floating-point formats are based on full volume being in the range
//!   [-1.0, 1.0]. Any values outside this range are beyond full volume level.
//!
//! - The data layout is as follows:
//!   For planar sample formats, each audio channel is in a separate data plane,
//!   and linesize is the buffer size, in bytes, for a single plane. All data
//!   planes must be the same size. For packed sample formats, only the first data
//!   plane is used, and samples for each channel are interleaved. In this case,
//!   linesize is the buffer size, in bytes, for the 1 plane.
//! @tsdocend
//! TSDecl: @enum SampleFormat
enum class SampleFormat
{
    //! TSDecl: @enumitem None
    kNone = AV_SAMPLE_FMT_NONE,

    //! @tsdocbegin
    //! unsigned 8 bits
    //! @tsdocend
    //! TSDecl: @enumitem U8
    kU8 = AV_SAMPLE_FMT_U8,

    //! @tsdocbegin
    //! signed 16 bits
    //! @tsdocend
    //! TSDecl: @enumitem S16
    kS16 = AV_SAMPLE_FMT_S16,

    //! @tsdocbegin
    //! signed 32 bits
    //! @tsdocend
    //! TSDecl: @enumitem S32
    kS32 = AV_SAMPLE_FMT_S32,

    //! @tsdocbegin
    //! float (f32)
    //! @tsdocend
    //! TSDecl: @enumitem FLT
    kFLT = AV_SAMPLE_FMT_FLT,

    //! @tsdocbegin
    //! double (f64)
    //! @tsdocend
    //! TSDecl: @enumitem DBL
    kDBL = AV_SAMPLE_FMT_DBL,

    //! @tsdocbegin
    //! unsigned 8 bits, planar
    //! @tsdocend
    //! TSDecl: @enumitem U8P
    kU8P = AV_SAMPLE_FMT_U8P,

    //! @tsdocbegin
    //! signed 16 bits, planar
    //! @tsdocend
    //! TSDecl: @enumitem S16P
    kS16P = AV_SAMPLE_FMT_S16P,

    //! @tsdocbegin
    //! signed 32 bits, planar
    //! @tsdocend
    //! TSDecl: @enumitem S32P
    kS32P = AV_SAMPLE_FMT_S32P,

    //! @tsdocbegin
    //! float (f32), planar
    //! @tsdocend
    //! TSDecl: @enumitem FLTP
    kFLTP = AV_SAMPLE_FMT_FLTP,

    //! @tsdocbegin
    //! double (f64), planar
    //! @tsdocend
    //! TSDecl: @enumitem DBLP
    kDBLP = AV_SAMPLE_FMT_DBLP,

    //! @tsdocbegin
    //! signed 64 bits
    //! @tsdocend
    //! TSDecl: @enumitem S64
    kS64 = AV_SAMPLE_FMT_S64,

    //! @tsdocbegin
    //! signed 64 bits, planar
    //! @tsdocend
    //! TSDecl: @enumitem S64P
    kS64P = AV_SAMPLE_FMT_S64P
};
//! TSDecl: @end

//! TSDecl: @enum SeekFrameFlags
enum class SeekFrameFlags : uint32_t
{
    //! @tsdocbegin
    //! Seek backward
    //! @tsdocend
    //! TSDecl: @enumitem Backward
    kBackward = AVSEEK_FLAG_BACKWARD,

    //! @tsdocbegin
    //! Seeking based on position in bytes.
    //! @tsdocend
    //! TSDecl: @enumitem Byte
    kByte = AVSEEK_FLAG_BYTE,

    //! @tsdocbegin
    //! Seek to any frame, even non-keyframes
    //! @tsdocend
    //! TSDecl: @enumitem Any
    kAny = AVSEEK_FLAG_ANY,

    //! @tsdocbegin
    //! Seeking based on frame number
    //! @tsdocend
    //! TSDecl: @enumitem Frame
    kFrame = AVSEEK_FLAG_FRAME
};
//! TSDecl: @end

//! TSDecl: @enum Discard
enum class Discard
{
    //! @tsdocbegin
    //! discard nothing
    //! @tsdocend
    //! TSDecl: @enumitem None
    kNone = AVDISCARD_NONE,

    //! @tsdocbegin
    //! discard useless packets like 0 size packets in avi
    //! @tsdocend
    //! TSDecl: @enumitem Default
    kDefault = AVDISCARD_DEFAULT,

    //! @tsdocbegin
    //! discard all non reference
    //! @tsdocend
    //! TSDecl: @enumitem NonRef
    kNonRef = AVDISCARD_NONREF,

    //! @tsdocbegin
    //! discard all bidirectional frames
    //! @tsdocend
    //! TSDecl: @enumitem Bidir
    kBidir = AVDISCARD_BIDIR,

    //! @tsdocbegin
    //! discard all non intra frames
    //! @tsdocend
    //! TSDecl: @enumitem NonIntra
    kNonIntra = AVDISCARD_NONINTRA,

    //! @tsdocbegin
    //! discard all frames except keyframes
    //! @tsdocend
    //! TSDecl: @enumitem NonKey
    kNonKey = AVDISCARD_NONKEY,

    //! @tsdocbegin
    //! discard all
    //! @tsdocend
    //! TSDecl: @enumitem All
    kAll = AVDISCARD_ALL
};
//! TSDecl: @end

//! TSDecl: @enum PictureType
enum class PictureType
{
    //! @tsdocbegin
    //! Undefined
    //! @tsdocend
    //! TSDecl: @enumitem None
    kNone,

    //! @tsdocbegin
    //! Intra
    //! @tsdocend
    //! TSDecl: @enumitem I
    kI,

    //! @tsdocbegin
    //! Predicted
    //! @tsdocend
    //! TSDecl: @enumitem P
    kP,

    //! @tsdocbegin
    //! Bidirectional predicted
    //! @tsdocend
    //! TSDecl: @enumitem B
    kB,

    //! @tsdocbegin
    //! S(GMC)-VOP MPEG-4
    //! @tsdocend
    //! TSDecl: @enumitem S
    kS,

    //! @tsdocbegin
    //! Switching Infra
    //! @tsdocend
    //! TSDecl: @enumitem SI
    kSI,

    //! @tsdocbegin
    //! Switching Predicted
    //! @tsdocend
    //! TSDecl: @enumitem SP
    kSP,

    //! @tsdocbegin
    //! BI type
    //! @tsdocend
    //! TSDecl: @enumitem BI
    kBI
};
//! TSDecl: @end

//! TSDecl: @enum FrameFlags
enum class FrameFlags : uint32_t
{
    //! @tsdocbegin
    //! The frame data may be corrupted, e.g. due to decoding errors.
    //! @tsdocend
    //! TSDecl: @enumitem Corrupt
    kCorrupt = AV_FRAME_FLAG_CORRUPT,

    //! @tsdocbegin
    //! A flag to mark frames that are keyframes.
    //! @tsdocend
    //! TSDecl: @enumitem Key
    kKey = AV_FRAME_FLAG_KEY,

    //! @tsdocbegin
    //! A flag to mark the frames which need to be decoded, but shouldn't be output.
    //! @tsdocend
    //! TSDecl: @enumitem Discard
    kDiscard = AV_FRAME_FLAG_DISCARD,

    //! @tsdocbegin
    //! A flag to mark frames whose content is interlaced.
    //! @tsdocend
    //! TSDecl: @enumitem Interlaced
    kInterlaced = AV_FRAME_FLAG_INTERLACED,

    //! @tsdocbegin
    //! A flag to mark frames where the top field is displayed first if the content
    //! is interlaced.
    //! @tsdocend
    //! TSDecl: @enumitem TopFieldFirst
    kTopFieldFirst = AV_FRAME_FLAG_TOP_FIELD_FIRST
};
//! TSDecl: @end

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
#endif //COCOA_GALLIUM_BINDINGS_MULTIMEDIA_TYPES_H
