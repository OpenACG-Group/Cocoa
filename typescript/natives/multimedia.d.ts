import * as _event from 'event';
import * as _renderer from 'renderer';
type i8 = number;
type u8 = number;
type i16 = number;
type u16 = number;
type i32 = number;
type u32 = number;
type i64 = number;
type u64 = number;
type f32 = number;
type f64 = number;
export declare enum CodecType {
    Encoder,
    Decoder,
}
export declare enum CodecStatus {
    Success,
    /**
     * Input is not accepted in the current state. User must read output first,
     * and once all output is read, the input should be resent, and the call will
     * not fail with this state.
     */
    NeedConsumeOutput,
    /** Output is not available in this state. User must try to send new input. */
    NeedFeedInput,
    /**
     * The codec has been flushed, and neither new input can be sent to it, nor
     * new output can be received from it.
     */
    EOF,
    /**
     * Invalid use of codec (e.g. use a encoder as a decoder).
     * See detailed information in the comment of related methods.
     */
    Invalid,
    /** Failed to allocate memory; failed to add input to internal queue; or similar. */
    NoMemory,
    Error,
}
export declare enum FilterGraphReceiveStatus {
    Success,
    /**
     * No frames are available at this point; more input frames must
     * be sent to the filter graph to get more output.
     */
    NeedInput,
    /** There will be no more output frames on this sink. */
    EOF,
}
export declare enum FrameSchedulerStatus {
    Success,
    /**
     * Number of frames in the queue exceeds the watermark, and the new frame
     * cannot be enqueued until frames in the queue has been consumed.
     */
    Full,
}
/** Controls the behaviour of frame scheduler when `pause()` is called. */
export declare enum FrameSchedulerPausePolicy {
    /** Keep the frames in the queue, and pause the internal timer. */
    Conserve,
    /**
     * Discard all the frames in the queue, not including pending frames from `enqueuePromise()`,
     * and pause the internal timer.
     */
    Discard,
    /**
     * Discard all the frames in the queue, including pending frames from `enqueuePromise()`,
     * and pause the internal timer. The pending promises will be rejected.
     */
    DiscardAndReject,
}
export declare enum SeekWhence {
    Set,
    Cur,
    End,
}
export declare enum MediaIOFlags {
    Read,
    Write,
    ReadWrite,
    Direct,
    NonBlock,
}
export declare enum ScaleResampler {
    Nearest,
    Bilinear,
    Bicubic,
}
export declare enum PacketFlags {
    Key,
    Corrupt,
    /**
     * Flag is used to discard packets which are required to maintain valid
     * decoder state but are not required for output and should be dropped
     * after decoding.
     */
    Discard,
    /**
     * Flag is used to indicate packets that contain frames that can
     * be discarded by the decoder.  I.e. Non-reference frames.
     */
    Disposable,
}
export declare enum MediaType {
    Unknown,
    Video,
    Audio,
    Subtitle,
    Attachment,
    Data,
}
export declare enum StreamDisposition {
    Default,
    Dub,
    Original,
    Comment,
    Lyrics,
    Karaoke,
    Forced,
    HearingImpaired,
    VisualImpaired,
    CleanEffects,
    AttachedPic,
    TimedThumbnails,
    NonDiegetic,
    Captions,
    Descriptions,
    Metadata,
    Dependent,
    StillImage,
}
export declare enum PixelFormat {
    kNone,
    /** planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples) */
    kYUV420P,
    /** packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr */
    kYUYV422,
    /** packed RGB 8:8:8, 24bpp, RGBRGB... */
    kRGB24,
    /** packed RGB 8:8:8, 24bpp, BGRBGR... */
    kBGR24,
    /** planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples) */
    kYUV422P,
    /** planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples) */
    kYUV444P,
    /** planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples) */
    kYUV410P,
    /** planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples) */
    kYUV411P,
    /**        Y        ,  8bpp */
    kGRAY8,
    /**        Y        ,  1bpp, 0 is white, 1 is black, in each byte pixels are ordered from the msb to the lsb */
    kMONOWHITE,
    /**        Y        ,  1bpp, 0 is black, 1 is white, in each byte pixels are ordered from the msb to the lsb */
    kMONOBLACK,
    /** 8 bits with AV_PIX_FMT_RGB32 palette */
    kPAL8,
    /** packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1 */
    kUYVY422,
    /** packed YUV 4:1:1, 12bpp, Cb Y0 Y1 Cr Y2 Y3 */
    kUYYVYY411,
    /** packed RGB 3:3:2,  8bpp, (msb)2B 3G 3R(lsb) */
    kBGR8,
    /** packed RGB 1:2:1 bitstream,  4bpp, (msb)1B 2G 1R(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits */
    kBGR4,
    /** packed RGB 1:2:1,  8bpp, (msb)1B 2G 1R(lsb) */
    kBGR4_BYTE,
    /** packed RGB 3:3:2,  8bpp, (msb)2R 3G 3B(lsb) */
    kRGB8,
    /** packed RGB 1:2:1 bitstream,  4bpp, (msb)1R 2G 1B(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits */
    kRGB4,
    /** packed RGB 1:2:1,  8bpp, (msb)1R 2G 1B(lsb) */
    kRGB4_BYTE,
    /** planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V) */
    kNV12,
    /** as above, but U and V bytes are swapped */
    kNV21,
    /** packed ARGB 8:8:8:8, 32bpp, ARGBARGB... */
    kARGB,
    /** packed RGBA 8:8:8:8, 32bpp, RGBARGBA... */
    kRGBA,
    /** packed ABGR 8:8:8:8, 32bpp, ABGRABGR... */
    kABGR,
    /** packed BGRA 8:8:8:8, 32bpp, BGRABGRA... */
    kBGRA,
    /**        Y        , 16bpp, big-endian */
    kGRAY16BE,
    /**        Y        , 16bpp, little-endian */
    kGRAY16LE,
    /** planar YUV 4:4:0 (1 Cr & Cb sample per 1x2 Y samples) */
    kYUV440P,
    /** planar YUV 4:2:0, 20bpp, (1 Cr & Cb sample per 2x2 Y & A samples) */
    kYUVA420P,
    /** packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as big-endian */
    kRGB48BE,
    /** packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as little-endian */
    kRGB48LE,
    /** packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), big-endian */
    kRGB565BE,
    /** packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), little-endian */
    kRGB565LE,
    /** packed RGB 5:5:5, 16bpp, (msb)1X 5R 5G 5B(lsb), big-endian   , X=unused/undefined */
    kRGB555BE,
    /** packed RGB 5:5:5, 16bpp, (msb)1X 5R 5G 5B(lsb), little-endian, X=unused/undefined */
    kRGB555LE,
    /** packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), big-endian */
    kBGR565BE,
    /** packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), little-endian */
    kBGR565LE,
    /** packed BGR 5:5:5, 16bpp, (msb)1X 5B 5G 5R(lsb), big-endian   , X=unused/undefined */
    kBGR555BE,
    /** packed BGR 5:5:5, 16bpp, (msb)1X 5B 5G 5R(lsb), little-endian, X=unused/undefined */
    kBGR555LE,
    /** planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian */
    kYUV420P16LE,
    /** planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian */
    kYUV420P16BE,
    /** planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian */
    kYUV422P16LE,
    /** planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian */
    kYUV422P16BE,
    /** planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian */
    kYUV444P16LE,
    /** planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian */
    kYUV444P16BE,
    /** HW decoding through DXVA2, Picture.data[3] contains a LPDIRECT3DSURFACE9 pointer */
    kDXVA2_VLD,
    /** packed RGB 4:4:4, 16bpp, (msb)4X 4R 4G 4B(lsb), little-endian, X=unused/undefined */
    kRGB444LE,
    /** packed RGB 4:4:4, 16bpp, (msb)4X 4R 4G 4B(lsb), big-endian,    X=unused/undefined */
    kRGB444BE,
    /** packed BGR 4:4:4, 16bpp, (msb)4X 4B 4G 4R(lsb), little-endian, X=unused/undefined */
    kBGR444LE,
    /** packed BGR 4:4:4, 16bpp, (msb)4X 4B 4G 4R(lsb), big-endian,    X=unused/undefined */
    kBGR444BE,
    /** 8 bits gray, 8 bits alpha */
    kYA8,
    /** alias for kYA8 */
    kY400A,
    /** alias for kYA8 */
    kGRAY8A,
    /** packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as big-endian */
    kBGR48BE,
    /** packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as little-endian */
    kBGR48LE,
    /** planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian */
    kYUV420P9BE,
    /** planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian */
    kYUV420P9LE,
    /** planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian */
    kYUV420P10BE,
    /** planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian */
    kYUV420P10LE,
    /** planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian */
    kYUV422P10BE,
    /** planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian */
    kYUV422P10LE,
    /** planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian */
    kYUV444P9BE,
    /** planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian */
    kYUV444P9LE,
    /** planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian */
    kYUV444P10BE,
    /** planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian */
    kYUV444P10LE,
    /** planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian */
    kYUV422P9BE,
    /** planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian */
    kYUV422P9LE,
    /** planar GBR 4:4:4 24bpp */
    kGBRP,
    /** alias for kGBRP */
    kGBR24P,
    /** planar GBR 4:4:4 27bpp, big-endian */
    kGBRP9BE,
    /** planar GBR 4:4:4 27bpp, little-endian */
    kGBRP9LE,
    /** planar GBR 4:4:4 30bpp, big-endian */
    kGBRP10BE,
    /** planar GBR 4:4:4 30bpp, little-endian */
    kGBRP10LE,
    /** planar GBR 4:4:4 48bpp, big-endian */
    kGBRP16BE,
    /** planar GBR 4:4:4 48bpp, little-endian */
    kGBRP16LE,
    /** planar YUV 4:2:2 24bpp, (1 Cr & Cb sample per 2x1 Y & A samples) */
    kYUVA422P,
    /** planar YUV 4:4:4 32bpp, (1 Cr & Cb sample per 1x1 Y & A samples) */
    kYUVA444P,
    /** planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), big-endian */
    kYUVA420P9BE,
    /** planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), little-endian */
    kYUVA420P9LE,
    /** planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), big-endian */
    kYUVA422P9BE,
    /** planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), little-endian */
    kYUVA422P9LE,
    /** planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), big-endian */
    kYUVA444P9BE,
    /** planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), little-endian */
    kYUVA444P9LE,
    /** planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian) */
    kYUVA420P10BE,
    /** planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian) */
    kYUVA420P10LE,
    /** planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian) */
    kYUVA422P10BE,
    /** planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian) */
    kYUVA422P10LE,
    /** planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian) */
    kYUVA444P10BE,
    /** planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian) */
    kYUVA444P10LE,
    /** planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian) */
    kYUVA420P16BE,
    /** planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian) */
    kYUVA420P16LE,
    /** planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian) */
    kYUVA422P16BE,
    /** planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian) */
    kYUVA422P16LE,
    /** planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian) */
    kYUVA444P16BE,
    /** planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian) */
    kYUVA444P16LE,
    /** packed XYZ 4:4:4, 36 bpp, (msb) 12X, 12Y, 12Z (lsb), the 2-byte value for each X/Y/Z is stored as little-endian, the 4 lower bits are set to 0 */
    kXYZ12LE,
    /** packed XYZ 4:4:4, 36 bpp, (msb) 12X, 12Y, 12Z (lsb), the 2-byte value for each X/Y/Z is stored as big-endian, the 4 lower bits are set to 0 */
    kXYZ12BE,
    /** interleaved chroma YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples) */
    kNV16,
    /** interleaved chroma YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian */
    kNV20LE,
    /** interleaved chroma YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian */
    kNV20BE,
    /** packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian */
    kRGBA64BE,
    /** packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian */
    kRGBA64LE,
    /** packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian */
    kBGRA64BE,
    /** packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian */
    kBGRA64LE,
    /** packed YUV 4:2:2, 16bpp, Y0 Cr Y1 Cb */
    kYVYU422,
    /** 16 bits gray, 16 bits alpha (big-endian) */
    kYA16BE,
    /** 16 bits gray, 16 bits alpha (little-endian) */
    kYA16LE,
    /** planar GBRA 4:4:4:4 32bpp */
    kGBRAP,
    /** planar GBRA 4:4:4:4 64bpp, big-endian */
    kGBRAP16BE,
    /** planar GBRA 4:4:4:4 64bpp, little-endian */
    kGBRAP16LE,
    /** packed RGB 8:8:8, 32bpp, XRGBXRGB...   X=unused/undefined */
    k0RGB,
    /** packed RGB 8:8:8, 32bpp, RGBXRGBX...   X=unused/undefined */
    kRGB0,
    /** packed BGR 8:8:8, 32bpp, XBGRXBGR...   X=unused/undefined */
    k0BGR,
    /** packed BGR 8:8:8, 32bpp, BGRXBGRX...   X=unused/undefined */
    kBGR0,
    /** planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian */
    kYUV420P12BE,
    /** planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian */
    kYUV420P12LE,
    /** planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian */
    kYUV420P14BE,
    /** planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian */
    kYUV420P14LE,
    /** planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian */
    kYUV422P12BE,
    /** planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian */
    kYUV422P12LE,
    /** planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian */
    kYUV422P14BE,
    /** planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian */
    kYUV422P14LE,
    /** planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian */
    kYUV444P12BE,
    /** planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian */
    kYUV444P12LE,
    /** planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian */
    kYUV444P14BE,
    /** planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian */
    kYUV444P14LE,
    /** planar GBR 4:4:4 36bpp, big-endian */
    kGBRP12BE,
    /** planar GBR 4:4:4 36bpp, little-endian */
    kGBRP12LE,
    /** planar GBR 4:4:4 42bpp, big-endian */
    kGBRP14BE,
    /** planar GBR 4:4:4 42bpp, little-endian */
    kGBRP14LE,
    /** bayer, BGBG..(odd line), GRGR..(even line), 8-bit samples */
    kBAYER_BGGR8,
    /** bayer, RGRG..(odd line), GBGB..(even line), 8-bit samples */
    kBAYER_RGGB8,
    /** bayer, GBGB..(odd line), RGRG..(even line), 8-bit samples */
    kBAYER_GBRG8,
    /** bayer, GRGR..(odd line), BGBG..(even line), 8-bit samples */
    kBAYER_GRBG8,
    /** bayer, BGBG..(odd line), GRGR..(even line), 16-bit samples, little-endian */
    kBAYER_BGGR16LE,
    /** bayer, BGBG..(odd line), GRGR..(even line), 16-bit samples, big-endian */
    kBAYER_BGGR16BE,
    /** bayer, RGRG..(odd line), GBGB..(even line), 16-bit samples, little-endian */
    kBAYER_RGGB16LE,
    /** bayer, RGRG..(odd line), GBGB..(even line), 16-bit samples, big-endian */
    kBAYER_RGGB16BE,
    /** bayer, GBGB..(odd line), RGRG..(even line), 16-bit samples, little-endian */
    kBAYER_GBRG16LE,
    /** bayer, GBGB..(odd line), RGRG..(even line), 16-bit samples, big-endian */
    kBAYER_GBRG16BE,
    /** bayer, GRGR..(odd line), BGBG..(even line), 16-bit samples, little-endian */
    kBAYER_GRBG16LE,
    /** bayer, GRGR..(odd line), BGBG..(even line), 16-bit samples, big-endian */
    kBAYER_GRBG16BE,
    /** planar YUV 4:4:0,20bpp, (1 Cr & Cb sample per 1x2 Y samples), little-endian */
    kYUV440P10LE,
    /** planar YUV 4:4:0,20bpp, (1 Cr & Cb sample per 1x2 Y samples), big-endian */
    kYUV440P10BE,
    /** planar YUV 4:4:0,24bpp, (1 Cr & Cb sample per 1x2 Y samples), little-endian */
    kYUV440P12LE,
    /** planar YUV 4:4:0,24bpp, (1 Cr & Cb sample per 1x2 Y samples), big-endian */
    kYUV440P12BE,
    /** packed AYUV 4:4:4,64bpp (1 Cr & Cb sample per 1x1 Y & A samples), little-endian */
    kAYUV64LE,
    /** packed AYUV 4:4:4,64bpp (1 Cr & Cb sample per 1x1 Y & A samples), big-endian */
    kAYUV64BE,
    /** like NV12, with 10bpp per component, data in the high bits, zeros in the low bits, little-endian */
    kP010LE,
    /** like NV12, with 10bpp per component, data in the high bits, zeros in the low bits, big-endian */
    kP010BE,
    /** planar GBR 4:4:4:4 48bpp, big-endian */
    kGBRAP12BE,
    /** planar GBR 4:4:4:4 48bpp, little-endian */
    kGBRAP12LE,
    /** planar GBR 4:4:4:4 40bpp, big-endian */
    kGBRAP10BE,
    /** planar GBR 4:4:4:4 40bpp, little-endian */
    kGBRAP10LE,
    /**        Y        , 12bpp, big-endian */
    kGRAY12BE,
    /**        Y        , 12bpp, little-endian */
    kGRAY12LE,
    /**        Y        , 10bpp, big-endian */
    kGRAY10BE,
    /**        Y        , 10bpp, little-endian */
    kGRAY10LE,
    /** like NV12, with 16bpp per component, little-endian */
    kP016LE,
    /** like NV12, with 16bpp per component, big-endian */
    kP016BE,
    /**        Y        , 9bpp, big-endian */
    kGRAY9BE,
    /**        Y        , 9bpp, little-endian */
    kGRAY9LE,
    /** IEEE-754 single precision planar GBR 4:4:4,     96bpp, big-endian */
    kGBRPF32BE,
    /** IEEE-754 single precision planar GBR 4:4:4,     96bpp, little-endian */
    kGBRPF32LE,
    /** IEEE-754 single precision planar GBRA 4:4:4:4, 128bpp, big-endian */
    kGBRAPF32BE,
    /** IEEE-754 single precision planar GBRA 4:4:4:4, 128bpp, little-endian */
    kGBRAPF32LE,
    /**        Y        , 14bpp, big-endian */
    kGRAY14BE,
    /**        Y        , 14bpp, little-endian */
    kGRAY14LE,
    /** IEEE-754 single precision Y, 32bpp, big-endian */
    kGRAYF32BE,
    /** IEEE-754 single precision Y, 32bpp, little-endian */
    kGRAYF32LE,
    /** planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), 12b alpha, big-endian */
    kYUVA422P12BE,
    /** planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), 12b alpha, little-endian */
    kYUVA422P12LE,
    /** planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), 12b alpha, big-endian */
    kYUVA444P12BE,
    /** planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), 12b alpha, little-endian */
    kYUVA444P12LE,
    /** planar YUV 4:4:4, 24bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V) */
    kNV24,
    /** as above, but U and V bytes are swapped */
    kNV42,
    /** packed YUV 4:2:2 like YUYV422, 20bpp, data in the high bits, big-endian */
    kY210BE,
    /** packed YUV 4:2:2 like YUYV422, 20bpp, data in the high bits, little-endian */
    kY210LE,
    /** packed RGB 10:10:10, 30bpp, (msb)2X 10R 10G 10B(lsb), little-endian, X=unused/undefined */
    kX2RGB10LE,
    /** packed RGB 10:10:10, 30bpp, (msb)2X 10R 10G 10B(lsb), big-endian, X=unused/undefined */
    kX2RGB10BE,
    /** packed BGR 10:10:10, 30bpp, (msb)2X 10B 10G 10R(lsb), little-endian, X=unused/undefined */
    kX2BGR10LE,
    /** packed BGR 10:10:10, 30bpp, (msb)2X 10B 10G 10R(lsb), big-endian, X=unused/undefined */
    kX2BGR10BE,
    /** interleaved chroma YUV 4:2:2, 20bpp, data in the high bits, big-endian */
    kP210BE,
    /** interleaved chroma YUV 4:2:2, 20bpp, data in the high bits, little-endian */
    kP210LE,
    /** interleaved chroma YUV 4:4:4, 30bpp, data in the high bits, big-endian */
    kP410BE,
    /** interleaved chroma YUV 4:4:4, 30bpp, data in the high bits, little-endian */
    kP410LE,
    /** interleaved chroma YUV 4:2:2, 32bpp, big-endian */
    kP216BE,
    /** interleaved chroma YUV 4:2:2, 32bpp, little-endian */
    kP216LE,
    /** interleaved chroma YUV 4:4:4, 48bpp, big-endian */
    kP416BE,
    /** interleaved chroma YUV 4:4:4, 48bpp, little-endian */
    kP416LE,
    /** packed VUYA 4:4:4, 32bpp, VUYAVUYA... */
    kVUYA,
    /** IEEE-754 half precision packed RGBA 16:16:16:16, 64bpp, RGBARGBA..., big-endian */
    kRGBAF16BE,
    /** IEEE-754 half precision packed RGBA 16:16:16:16, 64bpp, RGBARGBA..., little-endian */
    kRGBAF16LE,
    /** packed VUYX 4:4:4, 32bpp, Variant of VUYA where alpha channel is left undefined */
    kVUYX,
    /** like NV12, with 12bpp per component, data in the high bits, zeros in the low bits, little-endian */
    kP012LE,
    /** like NV12, with 12bpp per component, data in the high bits, zeros in the low bits, big-endian */
    kP012BE,
    /** packed YUV 4:2:2 like YUYV422, 24bpp, data in the high bits, zeros in the low bits, big-endian */
    kY212BE,
    /** packed YUV 4:2:2 like YUYV422, 24bpp, data in the high bits, zeros in the low bits, little-endian */
    kY212LE,
    /** packed XVYU 4:4:4, 32bpp, (msb)2X 10V 10Y 10U(lsb), big-endian, variant of Y410 where alpha channel is left undefined */
    kXV30BE,
    /** packed XVYU 4:4:4, 32bpp, (msb)2X 10V 10Y 10U(lsb), little-endian, variant of Y410 where alpha channel is left undefined */
    kXV30LE,
    /** packed XVYU 4:4:4, 48bpp, data in the high bits, zeros in the low bits, big-endian, variant of Y412 where alpha channel is left undefined */
    kXV36BE,
    /** packed XVYU 4:4:4, 48bpp, data in the high bits, zeros in the low bits, little-endian, variant of Y412 where alpha channel is left undefined */
    kXV36LE,
    /** IEEE-754 single precision packed RGB 32:32:32, 96bpp, RGBRGB..., big-endian */
    kRGBF32BE,
    /** IEEE-754 single precision packed RGB 32:32:32, 96bpp, RGBRGB..., little-endian */
    kRGBF32LE,
    /** IEEE-754 single precision packed RGBA 32:32:32:32, 128bpp, RGBARGBA..., big-endian */
    kRGBAF32BE,
    /** IEEE-754 single precision packed RGBA 32:32:32:32, 128bpp, RGBARGBA..., little-endian */
    kRGBAF32LE,
}
export declare enum FieldOrder {
    Unknown,
    Progressive,
    /** Top coded first, top displayed first */
    TT,
    /** Bottom coded first, bottom displayed first */
    BB,
    /** Top coded first, bottom displayed first */
    TB,
    /** Bottom coded first, top displayed first */
    BT,
}
/**
 * Chromaticity coordinates of the source primaries.
 * These values match the ones defined by ISO/IEC 23091-2_2019 subclause 8.1 and ITU-T H.273.
 */
export declare enum ColorPrimaries {
    Reserved0,
    Unspecified,
    Reserved,
    /** also ITU-R BT1361 / IEC 61966-2-4 / SMPTE RP 177 Annex B */
    BT709,
    /** also FCC Title 47 Code of Federal Regulations 73.682 (a)(20) */
    BT470M,
    /** also ITU-R BT601-6 625 / ITU-R BT1358 625 / ITU-R BT1700 625 PAL & SECAM */
    BT470BG,
    /** also ITU-R BT601-6 525 / ITU-R BT1358 525 / ITU-R BT1700 NTSC */
    SMPTE170M,
    /** identical to above, also called "SMPTE C" even though it uses D65 */
    SMPTE240M,
    /** colour filters using Illuminant C */
    FILM,
    /** ITU-R BT2020 */
    BT2020,
    /** SMPTE ST 428-1 (CIE 1931 XYZ) */
    SMPTE428,
    /** alias of SMPTE428, */
    SMPTEST428_1,
    /** SMPTE ST 431-2 (2011) / DCI P3 */
    SMPTE431,
    /** SMPTE ST 432-1 (2010) / P3 D65 / Display P3 */
    SMPTE432,
    /** EBU Tech. 3213-E (nothing there) / one of JEDEC P22 group phosphors */
    EBU3213,
    /** alias of EBU3213 */
    JEDEC_P22,
}
/**
 * Color Transfer Characteristic.
 * These values match the ones defined by ISO/IEC 23091-2_2019 subclause 8.2.
 */
export declare enum ColorTransferCharacteristic {
    Reserved0,
    Unspecified,
    Reserved,
    /** also ITU-R BT1361 */
    BT709,
    /** also ITU-R BT470M / ITU-R BT1700 625 PAL & SECAM */
    GAMMA22,
    /** also ITU-R BT470BG */
    GAMMA28,
    /** also ITU-R BT601-6 525 or 625 / ITU-R BT1358 525 or 625 / ITU-R BT1700 NTSC */
    SMPTE170M,
    SMPTE240M,
    /** "Linear transfer characteristics" */
    LINEAR,
    /** "Logarithmic transfer characteristic (100:1 range)" */
    LOG,
    /** "Logarithmic transfer characteristic (100 * Sqrt(10) : 1 range)" */
    LOG_SQRT,
    /** IEC 61966-2-4 */
    IEC61966_2_4,
    /** ITU-R BT1361 Extended Colour Gamut */
    BT1361_ECG,
    /** IEC 61966-2-1 (sRGB or sYCC) */
    IEC61966_2_1,
    /** ITU-R BT2020 for 10-bit system */
    BT2020_10,
    /** ITU-R BT2020 for 12-bit system */
    BT2020_12,
    /** SMPTE ST 2084 for 10-, 12-, 14- and 16-bit systems */
    SMPTE2084,
    /** alias of SMPTE2084, */
    SMPTEST2084,
    /** SMPTE ST 428-1 */
    SMPTE428,
    /** alias of SMPTE428, */
    SMPTEST428_1,
    /** ARIB STD-B67, known as "Hybrid log-gamma" */
    ARIB_STD_B67,
}
/**
 * YUV colorspace type.
 * These values match the ones defined by ISO/IEC 23091-2_2019 subclause 8.3.
 */
export declare enum ColorSpace {
    /** order of coefficients is actually GBR, also IEC 61966-2-1 (sRGB), YZX and ST 428-1 */
    RGB,
    /** also ITU-R BT1361 / IEC 61966-2-4 xvYCC709 / derived in SMPTE RP 177 Annex B */
    BT709,
    Unspecified,
    /** reserved for future use by ITU-T and ISO/IEC just like 15-255 are */
    Reserved,
    /** FCC Title 47 Code of Federal Regulations 73.682 (a)(20) */
    FCC,
    /** also ITU-R BT601-6 625 / ITU-R BT1358 625 / ITU-R BT1700 625 PAL & SECAM / IEC 61966-2-4 xvYCC601 */
    BT470BG,
    /** also ITU-R BT601-6 525 / ITU-R BT1358 525 / ITU-R BT1700 NTSC / functionally identical to above */
    SMPTE170M,
    /** derived from 170M primaries and D65 white point, 170M is derived from BT470 System M's primaries */
    SMPTE240M,
    /** used by Dirac / VC-2 and H.264 FRext, see ITU-T SG16 */
    YCGCO,
    /** alias of YCGCO, */
    YCOCG,
    /** ITU-R BT2020 non-constant luminance system */
    BT2020_NCL,
    /** ITU-R BT2020 constant luminance system */
    BT2020_CL,
    /** SMPTE 2085, Y'D'zD'x */
    SMPTE2085,
    /** Chromaticity-derived non-constant luminance system */
    CHROMA_DERIVED_NCL,
    /** Chromaticity-derived constant luminance system */
    CHROMA_DERIVED_CL,
    /** ITU-R BT.2100-0, ICtCp */
    ICTCP,
}
/**
 * Visual content value range.
 * 
 * These values are based on definitions that can be found in multiple
 * specifications, such as ITU-T BT.709 (3.4 - Quantization of RGB, luminance
 * and colour-difference signals), ITU-T BT.2020 (Table 5 - Digital
 * Representation) as well as ITU-T BT.2100 (Table 9 - Digital 10- and 12-bit
 * integer representation). At the time of writing, the BT.2100 one is
 * recommended, as it also defines the full range representation.
 * 
 * Common definitions:
 *   - For RGB and luma planes such as Y in YCbCr and I in ICtCp,
 *     'E' is the original value in range of 0.0 to 1.0.
 *   - For chroma planes such as Cb,Cr and Ct,Cp, 'E' is the original
 *     value in range of -0.5 to 0.5.
 *   - 'n' is the output bit depth.
 *   - For additional definitions such as rounding and clipping to valid n
 *     bit unsigned integer range, please refer to BT.2100 (Table 9).
 */
export declare enum ColorRange {
    Unspecified,
    /**
     * Narrow or limited range content.
     * 
     * - For luma planes:
     * 
     *       (219 * E + 16) * 2^(n-8)
     * 
     *   F.ex. the range of 16-235 for 8 bits
     * 
     * - For chroma planes:
     * 
     *       (224 * E + 128) * 2^(n-8)
     * 
     *   F.ex. the range of 16-240 for 8 bits
     */
    MPEG,
    /**
     * Full range content.
     * 
     * - For RGB and luma planes:
     * 
     *       (2^n - 1) * E
     * 
     *   F.ex. the range of 0-255 for 8 bits
     * 
     * - For chroma planes:
     * 
     *       (2^n - 1) * E + 2^(n - 1)
     * 
     *   F.ex. the range of 1-255 for 8 bits
     */
    JPEG,
}
/**
 * Location of chroma samples.
 * 
 * Illustration showing the location of the first (top left) chroma sample of the
 * image, the left shows only luma, the right
 * shows the location of the chroma sample, the 2 could be imagined to overlay
 * each other but are drawn separately due to limitations of ASCII
 * 
 *                 1st 2nd       1st 2nd horizontal luma sample positions
 *                  v   v         v   v
 *                  ______        ______
 * 1st luma line > |X   X ...    |3 4 X ...     X are luma samples,
 *                 |             |1 2           1-6 are possible chroma positions
 * 2nd luma line > |X   X ...    |5 6 X ...     0 is undefined/unknown position
 */
export declare enum ChromaLocation {
    Unspecified,
    /** MPEG-2/4 4:2:0, H.264 default for 4:2:0 */
    Left,
    /** MPEG-1 4:2:0, JPEG 4:2:0, H.263 4:2:0 */
    Center,
    /** ITU-R 601, SMPTE 274M 296M S314M(DV 4:1:1), mpeg2 4:2:2 */
    TopLeft,
    Top,
    BottomLeft,
    Bottom,
}
/**
 * Audio sample formats
 * 
 * - The data described by the sample format is always in native-endian order.
 *   Sample values can be expressed by native C types, hence the lack of a signed
 *   24-bit sample format even though it is a common raw audio data format.
 * 
 * - The floating-point formats are based on full volume being in the range
 *   [-1.0, 1.0]. Any values outside this range are beyond full volume level.
 * 
 * - The data layout is as follows:
 *   For planar sample formats, each audio channel is in a separate data plane,
 *   and linesize is the buffer size, in bytes, for a single plane. All data
 *   planes must be the same size. For packed sample formats, only the first data
 *   plane is used, and samples for each channel are interleaved. In this case,
 *   linesize is the buffer size, in bytes, for the 1 plane.
 */
export declare enum SampleFormat {
    None,
    /** unsigned 8 bits */
    U8,
    /** signed 16 bits */
    S16,
    /** signed 32 bits */
    S32,
    /** float (f32) */
    FLT,
    /** double (f64) */
    DBL,
    /** unsigned 8 bits, planar */
    U8P,
    /** signed 16 bits, planar */
    S16P,
    /** signed 32 bits, planar */
    S32P,
    /** float (f32), planar */
    FLTP,
    /** double (f64), planar */
    DBLP,
    /** signed 64 bits */
    S64,
    /** signed 64 bits, planar */
    S64P,
}
export declare enum SeekFrameFlags {
    /** Seek backward */
    Backward,
    /** Seeking based on position in bytes. */
    Byte,
    /** Seek to any frame, even non-keyframes */
    Any,
    /** Seeking based on frame number */
    Frame,
}
export declare enum Discard {
    /** discard nothing */
    None,
    /** discard useless packets like 0 size packets in avi */
    Default,
    /** discard all non reference */
    NonRef,
    /** discard all bidirectional frames */
    Bidir,
    /** discard all non intra frames */
    NonIntra,
    /** discard all frames except keyframes */
    NonKey,
    /** discard all */
    All,
}
export declare enum PictureType {
    /** Undefined */
    None,
    /** Intra */
    I,
    /** Predicted */
    P,
    /** Bidirectional predicted */
    B,
    /** S(GMC)-VOP MPEG-4 */
    S,
    /** Switching Infra */
    SI,
    /** Switching Predicted */
    SP,
    /** BI type */
    BI,
}
export declare enum FrameFlags {
    /** The frame data may be corrupted, e.g. due to decoding errors. */
    Corrupt,
    /** A flag to mark frames that are keyframes. */
    Key,
    /** A flag to mark the frames which need to be decoded, but shouldn't be output. */
    Discard,
    /** A flag to mark frames whose content is interlaced. */
    Interlaced,
    /**
     * A flag to mark frames where the top field is displayed first if the content
     * is interlaced.
     */
    TopFieldFirst,
}
export interface FilterGraphSinkProperties {
    type: MediaType;
    timeBase: (Rational | null);
    /**
     * The following properties are video only.
     * Note that for video sinks, the following properties are never absent,
     * but can be `null` when unavailable.
     */
    pixelFormat?: PixelFormat;
    frameRate?: (Rational | null);
    width?: i32;
    height?: i32;
    SAR?: (Rational | null);
    colorSpace?: ColorSpace;
    colorRange?: ColorRange;
    hwFramesCtx?: (null | HWFramesContext);
    /** The following properties are audio only. */
    sampleFormat?: SampleFormat;
    channelLayout?: AChannelLayout;
    sampleRate?: i32;
}
/** An interface for specifying custom options to the demuxer, affecting its behaviour. */
export interface DemuxerOptions {
    /**
     * A comma ',' separated string of allowed container formats.
     * If absent all are allowed.
     */
    formatWhitelist?: string;
    /**
     * A comma ',' separated string of allowed decoders.
     * If absent all are allowed.
     */
    codecWhitelist?: string;
}
/**
 * Info contained in the format container (mp4, mov, etc.).
 * This provides similar information to what FFmpeg's `ffprobe` command prints.
 * Note that details of each stream are not included.
 */
export interface FormatContainerInfo {
    formatName: string;
    formatLongName: string;
    streamCount: u32;
    duration: i64;
    totalStreamBitRate: i64;
    metadata: Map<string, string>;
}
/** Media stream info read from the media file, filled by `FormatDemuxer`. */
export interface DemuxStreamInfo {
    index: i32;
    type: MediaType;
    /**
     * This is the fundamental unit of time (in seconds) in terms of which
     * frame timestamps are represented.
     */
    timeBase: Rational;
    /**
     * PTS of the first frame of the stream presentation order,
     * in stream time base. May be absent if unknown.
     */
    startTime?: i64;
    /**
     * Duration of the stream, in stream time base.
     * If a source file does not specify a duration, but does specify a bitrate,
     * this value will be estimated from bitrate and file size.
     */
    duration: i64;
    /** Stream disposition - a combination of `StreamDisposition.*` flags. */
    disposition: i32;
    metadata: Map<string, string>;
    /** Video stream info. Only available when `type` is `MediaType.Video`. */
    videoInfo?: DemuxStreamVideoInfo;
    /** Audio stream info. Only available when `type` is `MediaType.Audio`. */
    audioInfo?: DemuxStreamAudioInfo;
}
export interface DemuxStreamVideoInfo {
    /** Sample aspect ratio, may be absent if unknown or undefined. */
    SAR?: Rational;
    avgFrameRate: Rational;
    /**
     * Real base framerate of the stream.
     * This is the lowest framerate with which all timestamps can be
     * represented accurately (it is the least common multiple of all
     * framerates in the stream). Note, this value is just a guess!
     * For example, if the time base is 1/90000 and all frames have either
     * approximately 3600 or 1800 timer ticks, then `framerate` will be 50/1.
     */
    framerate: Rational;
    format: PixelFormat;
    /** The average bitrate of the encoded data (in bits per second). */
    bitrate: i64;
    width: i32;
    height: i32;
    /**
     * Additional, detailed, colorspace characteristics.
     * These characteristics are useful for color conversion or running other
     * image-processing algorithms.
     */
    colorRange: ColorRange;
    colorPrimaries: ColorPrimaries;
    colorTrc: ColorTransferCharacteristic;
    colorSpace: ColorSpace;
    chromaLocation: ChromaLocation;
    /** Number of delayed frames. */
    delayFrames: i32;
}
export interface DemuxStreamAudioInfo {
    format: SampleFormat;
    channelLayout: AChannelLayout;
    sampleRate: i32;
    /**
     * The amount of padding (in samples) inserted by the encoder at the beginning of
     * the audio. I.e. this number of leading decoded samples must be discarded by the
     * caller to get the original audio without leading padding.
     */
    initialPadding: i32;
    /**
     * The amount of padding (in samples) appended by the encoder to the end of the audio.
     * I.e. this number of decoded samples must be discarded by the caller from the end of
     * the stream to get the original audio without any trailing padding.
     */
    trailingPadding: i32;
    /** Number of samples to skip after a discontinuity. */
    seekPreroll: i32;
}
export interface FrameSpecification {
    /**
     * Video only. The pixel format of the frame. Maybe `PixelFormat.kNone`
     * if the format is not set, unknown, or the frame is a hardware frame.
     * 
     * set spec: optional;
     * get spec: guaranteed for video frames.
     */
    pixelFormat?: PixelFormat;
    /**
     * Audio only. The sample format of the frame. Maybe `SampleFormat.None`
     * if the format is not set or unknown.
     * 
     * set spec: optional;
     * get spec: guaranteed for audio frames.
     */
    sampleFormat?: SampleFormat;
    /**
     * Video only. Width and height in pixels.
     * 
     * set spec: optional;
     * get spec: guaranteed for video frames.
     */
    width?: i32;
    height?: i32;
    /**
     * Audio only. Audio samples (per channel) described by the frame.
     * 
     * set spec: optional;
     * get spec: guaranteed for audio frames.
     */
    nbSamples?: i32;
    /**
     * Video only. Picture type of the frame.
     * 
     * set spec: optional;
     * get spec: guaranteed for video frames.
     */
    pictureType?: PictureType;
    /**
     * Video only. Sample aspect ratio.
     * 
     * set spec: optional;
     * get spec: guaranteed for video frames, `0/1` of unknown.
     */
    SAR?: Rational;
    /**
     * Presentation timestamp, in the stream timebase.
     * 
     * set spec: optional;
     * get spec: may be absent if unknown.
     */
    pts?: i64;
    /**
     * Audio only. Audio sample rate in Hz.
     * 
     * set spec: optional.
     * get spec: guaranteed for audio frames.
     */
    sampleRate?: i32;
    /**
     * Frame flags. See `FrameFlags` for more details.
     * 
     * set spec: optional;
     * get spec: guaranteed.
     */
    flags?: FrameFlags;
    /**
     * Video only. Color characteristics.
     * 
     * set spec: optional;
     * get spec: guaranteed for video frames.
     */
    colorRange?: ColorRange;
    colorPrimaries?: ColorPrimaries;
    colorTrc?: ColorTransferCharacteristic;
    colorSpace?: ColorSpace;
    chromaLocation?: ChromaLocation;
    /**
     * Audio only. Audio channel layout.
     * 
     * set spec: optional;
     * get spec: guaranteed for audio frames.
     */
    channelLayout?: AChannelLayout;
    /**
     * Presentation duration the frame lasts.
     * 
     * set spec: optional;
     * get spec: may be absent if unknown.
     */
    duration?: i64;
    /**
     * Video only. Hardware frames context.
     * 
     * set spec: not acceptable;
     * get spec: only absent if not a hardware frame.
     */
    hwFramesCtx?: HWFramesContext;
    /**
     * Frame timestamp estimated using various heuristics, in stream time base.
     * 
     * set spec: not acceptable;
     * get spec: guaranteed.
     */
    bestEffortTimestamp?: i64;
    /**
     * Video only. The number of pixels to discard from the the top/bottom/left/right
     * border of the frame to obtain the sub-rectangle of the frame intended for
     * presentation.
     * 
     * set spec: optional;
     * get spec: guaranteed for video frames.
     */
    cropTop?: u64;
    cropBottom?: u64;
    cropLeft?: u64;
    cropRight?: u64;
}
export interface FrameSchedulerQueueOptions {
    /**
     * Whether to trigger a `present` event when a frame in the queue is represented.
     * Default is false.
     */
    emitsPresentEvent?: boolean;
    /**
     * Whether to feedback a `Frame` instance in the `present` event. The provided
     * instance clones the original instance passed to `enqueue()` or `enqueuePromise()`.
     * Default is false.
     */
    requiresFrameFeedback?: boolean;
}
export interface HWFrameTransferFormatsInfo {
    /** Possible source formats when transfer the data to the frame. */
    src: PixelFormat[];
    /** Possible destination formats when transfer the data from the frame. */
    dst: PixelFormat[];
}
export interface HWFramesConstraints {
    formats: PixelFormat[];
    /** The minimum size of frames. Zero if not known. */
    minWidth: i32;
    minHeight: i32;
    /** The maximum size of frames. Infinity if not known / no limit. */
    maxWidth: i32;
    maxHeight: i32;
}
export interface MediaIOBackend {
    /**
     * Specify the type of the context. Creates a write stream if true;
     * otherwise, creates a read stream.
     */
    writable?: boolean;
    bufferSizeHint?: i32;
    /**
     * A callback to read some data to refill the given buffer `buffer`, may be absent
     * if the `MediaIOContext` will not be used as a readable stream.
     * Returns the number of bytes read, 0 if EOF, and any value < 0 indicates an error.
     * Exceptions are swallowed.
     * 
     * Note that `buffer` is ONLY available in the scope of your callback. Once your
     * callback returns, `buffer` will be detached. Never pass it to other places.
     */
    onReadPacket?: ((buffer: ArrayBuffer) => i32);
    /**
     * A callback to write data in the given buffer `buffer` to destination, may be absent
     * if the `writable` property is absent or false.
     * Returns the number of bytes written, any value < 0 indicates an error.
     * Exceptions are swallowed.
     * 
     * Note that `buffer` is ONLY available in the scope of your callback. Once your
     * callback returns, `buffer` will be detached. Never pass it to other places.
     */
    onWritePacket?: ((buffer: ArrayBuffer) => i32);
    /**
     * A callback to seek to specified byte position, may be absent if the underlying implementation
     * is not seekable.
     * Returns the position measured in bytes from the beginning of the stream, any value < 0
     * indicates an error. Exceptions are swallowed.
     */
    onSeek?: ((offset: bigint, whence: SeekWhence) => bigint);
}
export interface FrameIterationResult {
    mediaType: MediaType;
    frame: Frame;
}
export interface FrameIterationOptions {
    /** Called only once when creating a codec context for an audio or video stream. */
    setAdditionalCodecOptions?: ((streamInfo: DemuxStreamInfo, builder: CodecContextBuilder) => void);
}
export interface FrameMakeFromEncodedOptions {
    /**
     * Force to decode the input into Y'CbCr (YUV) format. Otherwise, always decode into
     * RGB format. Default is false.
     * 
     * If enabled, option `reinterpretToSRGB` will be enabled, and any other options are ignored.
     */
    forceDecodeToYCbCr?: boolean;
    /**
     * Whatever the actual color space is, set the result's color space info (in `FrameSpecification`)
     * to sRGB, including color primaries and gamma. It does not change the behaviour of `*ColorSpace`
     * option: color space conversion still happens, and pixels should be in the expected color space.
     * But the color space info in frame specification is set to sRGB.
     */
    reinterpretToSRGB?: boolean;
    /** Convert the result to be linear gamma. Default is false. */
    toLinearGamma?: boolean;
    /** Convert the result to be sRGB color space. Default is false. */
    toSRGBColorSpace?: boolean;
    /**
     * Set the pixel format that the caller expects. The result is guaranteed to be in that format.
     * If not set, decoder will choose the most appropriate format.
     */
    format?: PixelFormat;
}
/**
 * An immutable object holding information about the channel layout of audio data.
 * 
 * A channel layout here is defined as a set of channels ordered in a specific
 * way (unless the channel order is unspecified), in which case an `AChannelLayout`
 * carries only the channel count).
 * All orders may be treated as if they were unspecified by ignoring everything
 * but the channel count.
 * 
 * Channel names (C=center, R=right, L=left, Rc=right of center, Lc=left of center, Sur=surround):
 *   FrontL FrontR FrontC
 *   LowFreq
 *   BackL BackR FrontLc FrontRc BackC
 *   SideL SideR
 *   TopC TopFrontL TopFrontC TopFrontR TopBackL TopBackC TopBackR
 *   L R (for stereo downmix)
 *   WideL WideR
 *   SurDirectL SurDirectR
 *   LowFreq2
 *   TopSideL TopSideR
 *   BottomFrontC BottomFrontL BottomFrontR
 * 
 * Note that these channel names are case-insensitive.
 * 
 */
export class AChannelLayout {
    private constructor();
    readonly channels: i32;
    /**
     * An ordered list of strings that describes each channel.
     * If the channel order is unspecified, returns an empty array.
     */
    readonly orderedChannels: string[];
    /**
     * Creates a channel layout from a specific predefined layout `name`.
     * Valid layout names are (C=center, R=right, L=left, Rc=right of center, Lc=left of center):
     *   - mono:           [1ch ]
     *   - stereo:         [2ch ] FrontL FrontR
     *   - 2.1:            [3ch ] stereo LowFreq
     *   - 2-1:            [3ch ] stereo BackC
     *   - surround:       [3ch ] stereo FrontC
     *   - 3.1:            [4ch ] surround LowFreq
     *   - 4.0:            [4ch ] surround BackC
     *   - 4.1:            [5ch ] 4.0 LowFreq
     *   - 2-2:            [4ch ] stereo SideL SideR
     *   - quad:           [4ch ] stereo BackL BackR
     *   - 5.0:            [5ch ] surround SideL SideR
     *   - 5.1:            [6ch ] 5.0 LowFreq
     *   - 5.0-back:       [5ch ] surround BackL BackR
     *   - 5.1-back:       [6ch ] 5.0-back LowFreq
     *   - 6.0:            [6ch ] 5.0 BackC
     *   - 6.0-front:      [6ch ] 2-2 FrontLc FrontRc
     *   - hexagonal:      [6ch ] 5.0-back BackC
     *   - 6.1:            [7ch ] 5.1 BackC
     *   - 6.1-back:       [7ch ] 5.1-back BackC
     *   - 6.1-front:      [7ch ] 6.0-front LowFreq
     *   - 7.0:            [7ch ] 5.0 BackL BackR
     *   - 7.0-front:      [7ch ] 5.0 FrontLc FrontRc
     *   - 7.1:            [8ch ] 5.1 BackL BackR
     *   - 7.1-wide:       [8ch ] 5.1 FrontLc FrontRc
     *   - 7.1-wide-back:  [8ch ] 5.1-back FrontLc FrontRc
     *   - 7.1-top-back:   [8ch ] 5.1-back TopFrontL TopFrontR
     *   - octagonal:      [8ch ] 5.0 BackL BackC BackR
     *   - cube:           [8ch ] quad TopFrontL TopFrontR TopBackL TopBackR
     *   - hexadecagonal:  [16ch] octagonal WideL WideR TopBackL TopBackR TopBackC TopFrontL TopFrontR
     *   - stereo-downmix: [2ch ] L R
     *   - 22.2:           [24ch] 5.1-back FrontLc FrontRc BackC LowFreq2 SideL SideR
     *                            TopFrontL TopFrontR TopFrontC TopC TopBackL TopBackR
     *                            TopSideL TopSideR TopBackC BottomFrontC BottomFrontL
     *                            BottomFrontR
     * 
     */
    static Predefined(name: string): AChannelLayout;
    clone(): AChannelLayout;
    equalTo(other: AChannelLayout): boolean;
    /**
     * Find out what channels from a given set are present in the channel layout,
     * without regard for their positions.
     */
    intersect(ch: string[]): string[];
}
/**
 * A stream that allows writing audio frames to the audio service.
 * 
 * `AudioSinkStream` is an event emitter of the following events:
 *   @event volume-changed(volumes: f32[]): when volume has been changed by system;
 *                                          volumes are in the current channel order.
 * 
 *   @event empty-queue(lastFrameId: bigint): when the last frame has been consumed and
 *                                            the queue becomes empty. `lastFrameId` is
 *                                            the return value of the last `enqueue()`.
 */
export class AudioSinkStream extends _event.EventEmitterBase {
    private constructor();
    dispose(): void;
    /**
     * Pushes a frame into the presentation queue of the sink stream, and the frame
     * will be played as soon as possible. Timestamp of the frame is ignored.
     * Returns a frame ID on success, which can be used when handling `empty-queue` event.
     * 
     * The frame must be an audio frame, and its format, sample rate, and channel
     * layout must be strictly identical to what were used to create the sink stream.
     * Otherwise, throws an exception on failure.
     * 
     * Contents of frame will not be touched, and it is dispose-safe.
     */
    enqueue(frame: Frame): bigint;
    /**
     * Returns the current delay of system audio, in microseconds.
     * Useful for realtime audio to make sure the frame will be played at a correct time.
     */
    getDelayInUs(): f64;
    /**
     * Set volumes for each channel. `volumes.length` must be identical to
     * the number of channels in the current channel layout; otherwise it does nothing.
     * A `volume-changed` event will be triggered when the volumes are updated.
     */
    setVolumes(volumes: f64[]): void;
}
/**
 * An active connection to the system's audio service backend (e.g. PipeWire on Linux).
 * Once a connection is established via `Connect()` method.
 * 
 * The connection itself is reference counted - when all the users of the connection
 * (they can be audio streams created by the connection) are disposed.
 */
export class AudioStreamService {
    private constructor();
    /**
     * Establishes a new connection to the system's audio service. On success,
     * it prevents the event loop from exiting.
     * Throws an exception on failure.
     */
    static Connect(): AudioStreamService;
    /**
     * Disposes this instance. It does NOT mean to release the connection, but just
     * decreases the refcount of the connection and invalidates this instance.
     * Other users (streams created by the connection) may still need the connection,
     * when all of them are disposed, the connection will be released.
     */
    dispose(): void;
    /**
     * Create a sink (output) audio stream. The new stream increases the refcount of
     * the connection.
     * Throws an exception on failure.
     * 
     * @param name     Can be any string describing the stream. May be showed by the
     *                 desktop environment to indicate your application. Will not be
     *                 parsed by any program.
     * @param format   Sample format, the created stream only accepts this format.
     * @param sampleRate Sample rate in Hz, the created stream only accepts this sample rate.
     * @param channelLayout Channel layout, the created stream only accepts this layout.
     * @param realtime Whether the stream is a realtime.
     */
    createSinkStream(name: string, format: SampleFormat, sampleRate: i32, channelLayout: AChannelLayout, realtime: boolean): AudioSinkStream;
}
/**
 * Helps find a proper decoder/encoder, and create a `CodecContext` instance
 * from the given parameters. Cannot be reused.
 * 
 * To build a codec context, you first should call `setXXX()` to provide a set of
 * parameters required by codec. Finally, call `detach()` to get the created context
 * and simultaneously dispose the builder.
 */
export class CodecContextBuilder {
    constructor(type: CodecType);
    setCodecParameters(params: CodecParameters): CodecContextBuilder;
    setPacketTimebase(timebase: Rational): CodecContextBuilder;
    setCodecOption(name: string, value: string): CodecContextBuilder;
    setEncoderHWFramesContext(hwFramesCtx: HWFramesContext): CodecContextBuilder;
    setDecoderHWDeviceContext(device: HWDeviceContext): CodecContextBuilder;
    detach(): CodecContext;
}
export class CodecContext {
    private constructor();
    dispose(): void;
    /**
     * Supply raw packet data as input to a decoder, and then `receiveFrame()`
     * should be used to receive the decoded pixel data.
     * 
     * The provided packet is fully consumed, and if it contains multiple frames
     * (e.g. some audio codecs), will require you to call `receiveFrame()` multiple
     * times afterwards before you can send a new packet. It can be `null` or an
     * empty packet; in this case, it is considered a flush packet, which signals the
     * end of the stream. Sending the first flush packet will return success.
     * Subsequent ones are unnecessary and will return EOF. If the decoder still
     * has frames buffered, it will return them after sending a flush packet.
     * 
     * Possible status returned:
     *   Success
     *   NeedConsumeOutput
     *   EOF                   - the decoder has been flushed, and no new packets
     *                           can be sent to it (also returned if more than 1 flush
     *                           packet is sent)
     *   Invalid               - it is an encoder, or requires flush
     *   NoMemory
     *   Error
     * 
     */
    sendPacket(packet: (Packet | null)): CodecStatus;
    /**
     * Return decoded output data from a decoder or encoder
     * (when the `CodecFlags.ReconFrame` flag is used).
     * 
     * If `reuse` argument is null, a new frame instance will be created and returned;
     * otherwise, we try to reuse the provided frame to store data, and returns the same
     * instance. If the provided frame is not reusable, throws an exception.
     * 
     * Possible status returned:
     *   Success
     *   NeedFeedInput
     *   EOF                   - the codec has been fully flushed, and there will be
     *                           no more output frames
     *   Invalid               - it is an encoder without the `CodecFlags.ReconFrame`
     *                           flag enabled
     *   Error
     */
    receiveFrame(reuse: (null | Frame)): [CodecStatus, Frame];
    /**
     * Supply a raw video or audio frame to the encoder. Use `receivePacket()` to retrieve
     * buffered output packets.
     * 
     * The provided frame can be `null`, in which case it is considered a flush packet.
     * This signals the end of the stream. If the encoder still has packets buffered,
     * it will return them after this call. Once flushing mode has been entered, additional
     * flush packets are ignored, and sending frames will return EOF.
     * 
     * For audio, if `CodecFlags.CapVariableFrameSize` is set, then each frame can have
     * any number of samples. If it is not set, the number of samples must equal to
     * `CodecContext.frameSize` for all frames except the last. The final frame may be
     * smaller than we required.
     * 
     * Possible status returned:
     *   Success
     *   NeedConsumeOutput
     *   EOF                   - the encoder has been flushed, and no new frames can be
     *                           sent to it
     *   Invalid               - it is a decoder, or requires flush
     *   NoMemory
     *   Error
     */
    sendFrame(frame: (Frame | null)): CodecStatus;
    /**
     * Read encoded data from the encoder.
     * 
     * If `reuse` argument is null, a new packet instance will be created and returned;
     * otherwise, we try to reuse the provided packet to store data, and returns the same
     * instance. If the provided packet is not reusable, throws an exception.
     * 
     * Possible status returned:
     *   Success
     *   NeedFeedInput
     *   EOF                   - the encoder has been fully flushed, and there will be no
     *                           more output packets
     *   Invalid               - it is a decoder
     *   Error
     */
    receivePacket(reuse: (null | Packet)): [CodecStatus, Packet];
}
/**
 * `CodecParameters` describes a set of parameters (properties) of an encoded stream.
 * These parameters are used to instantiate a codec context.
 */
export class CodecParameters {
    /**
     * Create an instance with all the parameters set to default values,
     * e.g. unknown or invalid or 0. User is supposed to fill the parameters
     * before using this instance to create codec context.
     */
    constructor();
    /**
     * Search a codec ID by its name, always lowercase, with simple fuzzy search.
     * Returns a map of matched codec name and ID.
     */
    static SearchCodecID(name: string): Map<string, u32>;
    /**
     * Get the media type of the given codec ID.
     * Codec ID can be queried by name through `SearchCodecID()` method.
     */
    static GetCodecType(id: u32): MediaType;
    /**
     * Returns codec bits per sample, or zero if unknown.
     * Codec ID can be queried by name through `SearchCodecID()` method.
     */
    static GetCodecBitsPerSample(id: u32): i32;
    clone(): CodecParameters;
    setCodecType(type: MediaType): CodecParameters;
    /** Codec ID can be queried from `SearchCodecID()` method. */
    setCodecID(id: u32): CodecParameters;
    /**
     * Extra binary data needed for initializing the decoder, codec-dependent.
     * It copies the original data, not storing the given array buffer.
     */
    copyExtraData(data: Uint8Array): CodecParameters;
    setPixelFormat(format: PixelFormat): CodecParameters;
    setSampleFormat(format: SampleFormat): CodecParameters;
    /** The average bitrate of the encoded data, in bits per second. */
    setBitRate(br: i64): CodecParameters;
    /**
     * The number of bits per sample in the codedwords.
     * 
     * This is basically the bitrate per sample. It is mandatory for a bunch of
     * formats to actually decode them. It's the number of bits for one sample in
     * the actual coded bitstream.
     * 
     * This could be for example 4 for ADPCM, for PCM formats this matches `setBitsPerRawSample()`.
     * Can be 0.
     */
    setBitsPerCodedSample(bpcs: i32): CodecParameters;
    /**
     * This is the number of valid bits in each output sample. If the
     * sample format has more bits, the least significant bits are additional
     * padding bits, which are always 0. Use right shifts to reduce the sample
     * to its actual size. For example, audio formats with 24 bit samples will
     * have bits_per_raw_sample set to 24, and format set to `SampleFormat.S32`.
     * To get the original sample use "(i32)sample >> 8".
     * 
     * For ADPCM this might be 12 or 16 or similar.
     * Can be 0.
     */
    setBitsPerRawSample(bprs: i32): CodecParameters;
    /** Codec-specific bitstream restrictions that the stream conforms to. */
    setProfileLevel(profile: i32, level: i32): CodecParameters;
    /** Video only. The dimensions of the video frame in pixels. */
    setDimensions(width: i32, height: i32): CodecParameters;
    /**
     * Video only. The aspect ratio which a single pixel should have
     * when displayed. When the aspect ratio is unknown or undefined, the numerator
     * should be set to 0 (the denominator may have any value).
     */
    setSampleAspectRatio(sar: Rational): CodecParameters;
    /** Video only. The order of the fields in interlaced video. */
    setFieldOrder(fo: FieldOrder): CodecParameters;
    /** Video only. Additional colorspace characteristics. */
    setColorSpace(range: ColorRange, primaries: ColorPrimaries, trc: ColorTransferCharacteristic, space: ColorSpace, chromaLoc: ChromaLocation): CodecParameters;
    /** Video only. Number of delayed frame. */
    setVideoDelay(delay: i32): CodecParameters;
    /** Audio only. The number of audio samples per second. */
    setSampleRate(sr: i32): CodecParameters;
    /**
     * Audio only. The number of bytes per coded audio frame, required by some
     * formats. Corresponds to nBlockAlign in WACEFORMATEX.
     */
    setBlockAlign(ba: i32): CodecParameters;
    /** Audio only. Audio frame size, if known. Required by some formats to be static. */
    setFrameSize(fs: i32): CodecParameters;
    /**
     * Audio only. The amount of padding (in samples) inserted by the encoder at
     * the beginning of the audio. I.e. this number of leading decoded samples
     * must be discarded by the caller to get the original audio without leading
     * padding.
     */
    setInitialPadding(pad: i32): CodecParameters;
    /**
     * Audio only. The amount of padding (in samples) appended by the encoder to
     * the end of the audio. I.e. this number of decoded samples must be
     * discarded by the caller from the end of the stream to get the original
     * audio without any trailing padding.
     */
    setTrailingPadding(pad: i32): CodecParameters;
    /** Audio only. Number of samples to skip after a discontinuity. */
    setSeekPreroll(preroll: i32): CodecParameters;
    /** Audio only. The channel layout and number of channels. */
    setChannelLayout(layout: AChannelLayout): CodecParameters;
}
/** Helper class for creating a `FilterGraph` instance. */
export class FilterGraphBuilder {
    constructor();
    setThreads(num: i32): FilterGraphBuilder;
    /**
     * Set the FFmpeg graph description of the target filter graph. It describes the
     * nodes (filters) and links in the graph. For detailed information, see:
     *     https://ffmpeg.org/ffmpeg-filters.html
     * 
     * This function just declares the topology of the graph, and the user should
     * call `add{Audio,Video}{Src,Sink}()` methods to create the corresponding input
     * and output pads that are specified in the description. For example:
     * \code
     *     builder.setGraph('[inp] afade [outp]');
     *     builder.addAudioSrc('inp', ...);
     *     builder.addAudioSink('outp', ...);
     * \endcode
     * 
     * This function can be called multiple times, and the newer value replaces the
     * older value. But the input/output pads that have added remain unchanged.
     */
    setGraph(dsl: string): FilterGraphBuilder;
    /**
     * Adds an audio input pad in the graph. ID of the pad is what you specify in the
     * graph description. Other arguments constrains the format and properties of
     * acceptable frames on this pad. Frames not satisfying all the constraints cannot
     * be pushed into the filter graph through this pad.
     */
    addAudioSrc(padId: string, format: SampleFormat, timebase: (Rational | null), sampleRate: i32, channelLayout: AChannelLayout): FilterGraphBuilder;
    /**
     * Adds an video input pad in the graph. ID of the pad is what you specify in the
     * graph description. Other arguments constrains the format and properties of
     * acceptable frames on this pad. Frames not satisfying all the constraints cannot
     * be pushed into the filter graph through this pad.
     * 
     * If `hwctx` is passed, `format` must be `PixelFormat.kNone`, and an appropriate
     * format will be selected automatically.
     */
    addVideoSrc(padId: string, format: PixelFormat, timebase: (Rational | null), width: i32, height: i32, sar: Rational, colorSpace: ColorSpace, colorRange: ColorRange, hwctx: (HWFramesContext | null)): FilterGraphBuilder;
    /**
     * Adds an audio output pad in the graph. ID of the pad is what you specify in the
     * graph description.
     */
    addAudioSink(padId: string): FilterGraphBuilder;
    /**
     * Adds an video output pad in the graph. ID of the pad is what you specify in the
     * graph description.
     */
    addVideoSink(padId: string): FilterGraphBuilder;
    /**
     * Checks the configuration and creates a filter graph instance.
     * The builder itself is disposed after return.
     * Throws an exception on failure.
     */
    build(): FilterGraph;
}
/**
 * `FilterGraph` is a high-performance pipeline for audio/video processing.
 * Detailed information about filter graph: https://ffmpeg.org/ffmpeg-filters.html
 * 
 * The user should create a filter graph by using `FilterGraphBuilder`.
 */
export class FilterGraph {
    private constructor();
    dispose(): void;
    /** Get the properties of frames received at output pad `pad`. */
    getSinkProperties(pad: string): FilterGraphSinkProperties;
    /**
     * Pushes a frame into the graph. If `frame` is `null`, it indicates the EOF signal.
     * Throws an exception on failure.
     */
    sendFrame(pad: string, frame: (Frame | null)): void;
    /**
     * Receives a frame from the graph. If `reuse` points to a `Frame` instance in reusable
     * state, reuses that instance to receive the result, and returns the identical instance.
     * Otherwise, returns a newly created `Frame` instance.
     * 
     * If the returned status is not `Success`, `FilterGraphReceiveStatus.frame` is `null`,
     * and `reuse` is not touched.
     * 
     * Throws an exception on failure.
     */
    receiveFrame(pad: string, reuse: (Frame | null)): [FilterGraphReceiveStatus, Frame];
    /**
     * Send a command `cmd` with arguments `args` to the `target` filter in the graph.
     * `target` can be the filter's name or ID (see FFmpeg documentation).
     * 
     * If `propagate` is true, command is sent to all the filters matching `target`;
     * otherwise, it is only sent to the first matched filter.
     * 
     * Throws an exception on failure.
     */
    sendCommand(target: string, cmd: string, args: string, propagate: boolean): void;
}
/**
 * Reads data from a given `MediaIOContext` context, treating the data
 * as multimedia file formats (e.g. mp4, mp3, mov, etc.), then parse and extract
 * media contents from the data.
 */
export class FormatDemuxer {
    private constructor();
    readonly formatInfo: FormatContainerInfo;
    /**
     * Creates a demuxer that reads data from the given `MediaIOContext`, and returns
     * the created `FormatDemuxer`. Context must be readable.
     * The given `MediaIOContext` will be locked until the demuxer is disposed.
     * Throws an exception when context is not readable, or an error occurs.
     */
    static Make(context: MediaIOContext, options: DemuxerOptions): FormatDemuxer;
    dispose(): void;
    /**
     * Find the "best" stream in the file.
     * The best stream is determined according to various heuristics as the most
     * likely to be what the user expects. For example, among multiple video streams,
     * it selects the one that has the highest resolution.
     * Returns the index if a stream is found; otherwise, returns -1 if not found or
     * found but there is no corresponding decoder.
     */
    findBestStream(type: MediaType): i32;
    /**
     * Stream specifier describes some constraints to filter streams.
     * This method tests every stream contained in the media file, and returns stream indices
     * that satisfy all the constraints.
     * For the syntax of stream specifier, see FFmpeg's documentation:
     *   https://ffmpeg.org/ffmpeg.html#Stream-specifiers-1
     * 
     * Throws an exception if specifier is invalid.
     */
    matchStreamSpecifier(specifier: string): i32[];
    /**
     * Get stream information by its index.
     * Each call returns a newly created object, and user can use the returned object freely.
     * The object is not cached (each call returns a newly created object).
     * Throws an exception if `index` is invalid.
     */
    getStreamInfo(index: i32): DemuxStreamInfo;
    /**
     * Controls which frame in a specified stream will not be demuxed.
     * Throws an exception if `index` is invalid.
     */
    setStreamDiscard(index: i32, discard: Discard): void;
    /**
     * Returns codec parameters of a specified stream.
     * Never modify the returned parameters, and use it to instantiate a corresponding decoder.
     * The object is not cached (each call returns a newly created object).
     * Throws an exception if `index` is invalid.
     */
    getCodecParameters(index: i32): CodecParameters;
    /**
     * Seek to the keyframe at timestamp.
     * Throws an exception on failure.
     * 
     * @param index     Stream index. If -1 is provided, a default stream is selected,
     *                  and timestamp is automatically converted from `TIME_BASE` units
     *                  to the stream specific time base.
     * @param timestamp Timestamp in stream's time base units or, if index is -1,
     *                  in `TIME_BASE` units.
     * @param flags     Flags which select direction and seeking mode.
     */
    seekFrame(index: i32, timestamp: i64, flags: SeekFrameFlags): void;
    /**
     * Start playing a network-based stream (e.g. RTSP stream) at the current position.
     * Throws an exception on failure.
     */
    readPlay(): void;
    /**
     * Pause a network-based stream (e.g. RTSP stream). Use `readPlay()` to resume it.
     * Throws an exception on failure.
     */
    readPause(): void;
    /**
     * Return the next frame of a stream.
     * This function returns what is stored in the file, and does not validate
     * that what is there are valid frames for the decoder. It will split what is
     * stored in the file into frames and return one for each call. It will not
     * omit invalid data between valid frames so as to give the decoder the maximum
     * information possible for decoding.
     * 
     * If `reuse` argument is null, a new packet instance will be created and returned;
     * otherwise, we try to reuse the provided packet to store data, and returns the same
     * instance. If the provided packet is not reusable, throws an exception.
     * 
     * If EOF, returns `null`; throws an exception if an error occurs.
     * In both case `reuse` keeps untouched.
     * 
     * For video, the packet contains exactly one frame.
     * For audio, it contains an integer number of frames if each frame has
     * a known fixed size (e.g. PCM or ADPCM data). If the audio frames have
     * a variable size (e.g. MPEG audio), then it contains one frame.
     * 
     * `packet.pts`, `packet.dts` and `packet.duration` are always set to correct
     * values in stream's timebase units (and guessed if the format cannot provide them).
     * `packet.pts` can be `null` if the video format has B-frames, so it is better to
     * rely on dts if you do not decompress the payload.
     */
    readFrame(reuse: (Packet | null)): Packet;
}
/**
 * A reference to a underlying buffer that stores the decoded (raw) audio or video data.
 * One frame could contain multiple separated buffers, which are called planes, and
 * how to interpret the planes and data in them depends on the format of the frame.
 * 
 * Like `Packet` object, the underlying buffer of `Frame` is reference counted. Each
 * instance of `Frame` should be treated as a reference to its underlying buffer, and
 * when there is no reference to a underlying buffer, it will be freed. However, note
 * that not all the references to a certain underlying buffer are comes from `Frame`
 * instances, which means it may be referenced implicitly by internal code that the user
 * cannot touch. You should NEVER assume when a underlying buffer will be freed.
 * 
 * If a frame is disposed via `disposeReusable()`, it will become a reusable frame.
 * That means to delete the reference of the current underlying buffer, but remain other
 * allocated memory as more as possible, and then refill the instance with another
 * underlying buffer. It is useful to decrease the overhead caused by frequent object
 * allocation.
 */
export class Frame {
    private constructor();
    /**
     * Make a `Frame` instance and set its properties to specified values, without
     * allocating any underlying buffers. It creates an instance in the "reusable"
     * state, just like the state when `disposeReusable()` is called on an allocated
     * normal frame.
     * 
     * Absent property in `spec` will be its default value.
     */
    static MakeUnallocated(spec: FrameSpecification): Frame;
    /**
     * Equivalent to creating an instance by `MakeUnallocated(spec)`, and then allocate the
     * memory by `allocate()` method.
     */
    static MakeAllocated(spec: FrameSpecification): Frame;
    /**
     * Allocate new underlying buffer(s) for audio or video data.
     * The frame must be in "reusable" state, which means it has been disposed via
     * `disposeReusable(spec)`, or created via `MakeUnallocated(spec)`.
     * 
     * Either `spec.pixelFormat` or `spec.sampleFormat` must be set, and:
     *   - if `spec.pixelFormat` is set, `spec.width`, `spec.height` must be set;
     *   - if `spec.sampleFormat` is set, `spec.nbSamples`, `spec.channelLayout` must be set.
     * 
     * If the frame has been allocated, fails.
     * Throws an exception on failure. In that case, the frame is not touched.
     * 
     * Note that this method only allocates normal memory. For frames which are expected
     * to carry hw buffers, allocate them using `HWFramesContext.getBuffer()` instead.
     */
    allocate(): void;
    /**
     * Create a new `Frame` instance that shares the same underlying buffer.
     * This operation increases the refcount.
     */
    clone(): Frame;
    /**
     * Delete the reference to the underlying buffer and invalidate the `Frame`
     * instance. This operation decreases the refcount.
     */
    dispose(): void;
    /**
     * Like `dispose()`, but the `Frame` instance will be reusable, and can be reused
     * by functions like `CodecContext.receiveFrame(frame)`. This operation also reset the
     * frame properties to what the `spec` specifies, if it is provided.
     * This operation decreases the refcount.
     * Throws an exception if `spec` is invalid. In that case, frame is disposed, reusable,
     * but all the properties are set to default values.
     */
    disposeReusable(spec: FrameSpecification): void;
    /**
     * Get the frame specification. Depending on the media type of the frame, some properties
     * are filled while other fields are not. If a property is unavailable, it will be absent.
     * 
     * See `FrameSpecification` for more details.
     * 
     * This function does not cache the value. Returns a new object each call.
     */
    specification(type: MediaType): FrameSpecification;
    /**
     * Updates the specified frame properties. Properties not assigned in `spec` will not change.
     * This function could cause fatal errors if wrong properties are set. Do NOT use it unless
     * you know what are you doing.
     */
    updateSpecification(spec: FrameSpecification): void;
}
/** Helper class for reading/writing the underlying buffer of `Frame` instance. */
export class PixelFrameView {
    /**
     * Create the instance from a specified `frame` object.
     * The frame must be a video frame, otherwise it is undefined behaviour.
     * The new instance accesses the underlying buffer through the provided `frame` object,
     * when the frame is disposed, all its views will become unavailable (throws an exception
     * when the user attempts to access them).
     * 
     * Note that creating a view from a hardware frame is not allowed, and an exception will
     * be thrown in that case.
     */
    constructor(frame: Frame);
    /**
     * Returns true if the frame is writable.
     * A frame is writable, if and only if each of the underlying buffers has only one reference,
     * namely the one stored in this frame.
     */
    isWritable(): boolean;
    /**
     * Ensure that the frame data is writable, avoiding data copy if possible.
     * 
     * Do nothing if the frame is writable, allocate new buffers and copy the data
     * if it is not.
     * Throws an exception on failure.
     */
    makeWritable(): void;
    /**
     * Copy image data in `srcBuffers` to the frame storage. Length of `srcBuffers` and `srcRowBytes`
     * must be the same and match the number of planes of the frame. The destination frame must be
     * writable (`Frame.isWritable()` returns true). Neither format conversion nor scaling will be
     * performed, so the format and dimensions of src buffers must match the frame's specification.
     * 
     * Throws an exception on failure.
     */
    writeImageFrom(srcBuffers: Uint8Array[], srcRowBytes: i32[]): void;
    /**
     * Copy pixels in `src_rect` of `plane` to `dst` buffer.
     * 
     * Note that `src_rect` is supposed to have the plane's coordinates. E.g. if a 1920x1080 frame is
     * YUV420 format with Y, U, V three planes, then the rect representing the whole Y plane is
     * `RectXYWH(0, 0, 1920, 1080)`. However, for U or V plane, it is `RectXYWH(0, 0, 960, 540)` instead.
     * 
     * Fails if `src_rect` contains pixels out of the plane, or if the `dst` buffer has an insufficient
     * size to receive the pixels.
     * 
     * Throws an exception on failure.
     */
    readPlaneRectTo(plane: i32, srcRect: _renderer.Rect, dst: Uint8Array, dstRowBytes: i32): void;
    /**
     * Creates a `renderer.ColorSpace` according to the color characteristics of the frame.
     * 
     * Color characteristics are stored in the frame specification that the user can retrieve
     * by `Frame.specification(mediaType)`. It includes several properties:
     *   - `colorSpace` defines the YUV colorspace (i.e. a transformation between YUV and RGB);
     *   - `colorPrimaries` defines the three primary colors and a white point (reference white);
     *   - `colorTrc` defines a gamma transfer function;
     *   - `colorRange` defines the visual content value range.
     * 
     * Among these properties, `colorPrimaries` and `colorTrc` sufficiently describe a color space
     * (note that property `colorSpace` ONLY specifies the YUV <=> RGB transformation, instead of
     * a real color space).
     * 
     * This method uses `colorPrimaries` and `colorTrc` to create a `ColorSpace` object that can
     * be used in `renderer` module. For the source frame, supported color primaries include BT709,
     * SMPTE432, BT2020, SMPTE170M, SMPTE240M,  BT470M, BT470BG (BT610), SMPTE431, EBU3213, and
     * supported trc include BT709, GAMMA22, GAMMA28, SMPTE170M, SMPTE240M, LINEAR,  IEC61966-2-1 (sRGB),
     * BT2020.
     * 
     * Returns the created `ColorSpace` object on success, otherwise, returns `fallback`. If `fallback`
     * is `null`, returns `null`.
     */
    resolveColorSpace(fallback: (_renderer.ColorSpace | null)): (_renderer.ColorSpace | null);
    /**
     * Copies the pixels into the specified Pixmap `dst`, with scaling and color conversion if
     * necessary. The resampling options is specified by `sampling` (if `null`, bilinear is used).
     * 
     * If `dst` does not specify a color space (i.e. `dst.imageInfo.refColorSpace()` returns `null`),
     * this method uses the result of `resolveColorSpace(sRGB)` as the destination color space.
     * If `dst` specifies a color space, only sRGB is supported, otherwise, it throws an exception.
     * Returns a `renderer.ColorSpace` object that indicates the color space of pixels transferred
     * into `dst`.
     * 
     * Pixels may be copied more than once during conversion. This operation will not change the
     * result of `PixelFrameView.isWritable()`.
     * 
     * Throws an exception if the pixmap is invalid, or the conversion is impossible.
     */
    blitToPixmap(srcRegion: _renderer.Rect, dstRegion: _renderer.Rect, resampler: ScaleResampler, dst: _renderer.Pixmap): _renderer.ColorSpace;
    /**
     * Wraps the frame to a `renderer.Image` instance without memory copy. The result image shares
     * the same underlying buffer with the frame. Note that this operation makes a writable frame
     * not writable, since new buffer references are created.
     * 
     * It requires valid color characteristics to be set (color primaries, range, and trc), i.e.
     * those properties must not be `UNSPECIFIED`, otherwise it fails.
     * 
     * Not all formats and color spaces support this, and throws an exception on failure.
     */
    wrapToImage(): _renderer.Image;
    /**
     * Applies the cropping settings which is specified in frame specification. Returns a new `Frame`
     * instance that shares the same underlying buffer, but its width, height, and buffer views has
     * been adjusted according to the cropping settings. All cropping settings will be set to 0 in
     * the returned `Frame` instance.
     * 
     * In all cases, the cropping boundaries will be rounded to the inherent alignment of the pixel
     * format.
     * 
     * Specially, if all the cropping settings are 0, it acts like `Frame.clone()`.
     * Throws an exception on failure.
     */
    applyCropping(): Frame;
}
/** Helper class to build a `FrameScheduler` instance. See `FrameScheduler` for more details. */
export class FrameSchedulerBuilder {
    constructor();
    /**
     * Add a queue into the scheduler, with specified media type, watermark, and timebase.
     * Values in `options` controls some detailed behaviours of the scheduler, see
     * `FrameSchedulerQueueOptions` for more details.
     * 
     * For a certain media type, there is only one queue can be added. If the method
     * is called more than once with the same media type, the newer parameters will be used.
     * But for `options`, only present fields (fields which are not absent) will overwrite
     * the older value.
     * 
     * @param type         Media type of the queue, uniquely identifies a queue.
     * @param watermark    The maximum number of frames that could be stored in the queue.
     * @param timebase     Timebase of the queue, which all frames' pts are assumed to be in.
     * @param options      Detailed options of the queue.
     */
    addQueue(type: MediaType, watermark: i32, timebase: Rational, options: FrameSchedulerQueueOptions): FrameSchedulerBuilder;
    /**
     * Set an audio sink for the audio queue (if we have).
     * If set, an audio sink stream will be created from the provided `service`, with specified
     * `format`, `sampleRate`, and `channelLayout`. That audio sink stream will be used to
     * receive expired audio frames. When the scheduler is disposed, the stream will be disposed
     * together.
     * 
     * Although the user can require a frame feedback (by queue option `requiresFrameFeedback`)
     * for audio queue, and enqueue that frame into the user created `AudioSinkStream` instance
     * to play it, setting an audio sink directly by `setAudioSinkCreationInfo()` can reduce audio
     * delay and provide better performance.
     */
    setAudioSinkCreationInfo(service: AudioStreamService, format: SampleFormat, sampleRate: i32, channelLayout: AChannelLayout): FrameSchedulerBuilder;
    /**
     * Creates a scheduler instance using the specified configurations,
     * then disposes the builder.
     * Throws an exception for invalid configuration.
     */
    build(): FrameScheduler;
}
/**
 * `FrameScheduler` receives a sequence of frames that has timestamp, and will notify the
 * user by emitting events when a frame has expired. It guarantees that the time is accurate
 * enough for multimedia playing.
 * 
 * `FrameScheduler` maintains a set of queues, which are identified by media type. A certain
 * queue only accepts frames of a certain media type. Each queue has a limitation of the maximum
 * number of stored frames, which is called "watermark". If the limitation is exceeded, the queue
 * will not accept any frames until the head frame in the queue expires and leaves the queue.
 * 
 * `FrameScheduler` is an event emitter of the following events:
 *  @event empty-queue(type: MediaType):
 *    when the queue identified by media type `type` becomes empty (the last frame has been consumed).
 * 
 *  @event present(type: MediaType, ptsInSeconds: i32, frame: Frame | null):
 *    when a frame `frame` with timestamp `ptsInSeconds` has expired. Only appears when the
 *    `emitsPresentEvent` option is set, and `frame` is non-null only when the `requiresFrameFeedback`
 *    option is set. If `frame` is non-null, it is a different instance from the frame that the user
 *    has enqueued, but refers to the same underlying buffer, just like a cloned instance.
 * 
 */
export class FrameScheduler extends _event.EventEmitterBase {
    private constructor();
    /**
     * Dispatch a frame to a certain queue according to `mediaType`.
     * It clones the frame - `frame` is dispose-safe after calling the method.
     * 
     * Note that the timestamp of frames served in the same queue MUST be monotonically increasing.
     * If a frame has an older (smaller) timestamp than the previous frame, it will be dropped without
     * any notification. Specifically, for the scheduler, the concept of "now" always follows the
     * latest expired frame's timestamp, any frames older than "now" will be dropped.
     */
    enqueue(mediaType: MediaType, frame: Frame): FrameSchedulerStatus;
    /**
     * Like `enqueue()`, but never returns `FrameSchedulerStatus.Full`.
     * Instead, if the queue is not full, the promise will be fulfilled immediately;
     * otherwise, if the queue is full, the promise will be fulfilled when the enqueue
     * has free space and the frame has been enqueued.
     */
    enqueuePromise(mediaType: MediaType, frame: Frame): Promise<void>;
    /**
     * Enters the pause state, in which the internal timer is stopped.
     * `policy` controls how the frames remaining in the queue are handled.
     */
    pause(policy: FrameSchedulerPausePolicy): void;
    /** Resume from the pause state. */
    resume(): void;
    dispose(): void;
}
/**
 * This class describes a set or pool of "hardware" frames (i.e. those with
 * data not located in normal system memory). All the frames in the pool are
 * assumed to be allocated in the same way and interchangeable.
 */
export class HWFramesContext {
    private constructor();
    /**
     * Returns the constraints of hardware frames on current platform.
     * These constraints apply on creating the `HWFramesContext` via `HWFrameContext.Make()`.
     * This method may cause the initialization of device context.
     */
    static GetPlatformConstraints(device: HWDeviceContext): HWFramesConstraints;
    /**
     * Creates an instance of `HWFramesContext`. The underlying device, backend, and
     * all the related low-level resources are selected, created, and managed by Cocoa
     * internally.
     * 
     * This method may cause the initialization of device context.
     * Once the first `HWFramesContext` is created, the corresponding device
     * context has been initialized globally. When another `HWFramesContext` need to be
     * created, it will use the same underlying device context, and share common resources.
     * 
     * The `format` argument specifies the actual data layout in the device memory,
     * and choosing which format depends on your purpose, but for most cases, using NV12 format
     * will always produce the correct result.
     * 
     * Throws an exception on failure.
     * 
     * @param width, height        Dimensions of frames that are created from the context.
     * @param format               Pixel format, it specifies the actual data layout
     *                             of frames in the device memory.
     * @param initialPoolSize      An integer >= 0, specifies the initial size of the pool.
     *                             Some devices do not support dynamic pool size, and in that
     *                             case, this is also the maximum size of the pool.
     */
    static Make(device: HWDeviceContext, width: i32, height: i32, format: PixelFormat, initialPoolSize: i32): HWFramesContext;
    /**
     * Copy data to or from a hw frame. At least one of dst/src must be hw frame.
     * 
     * Supposing the return value of `queryTransferFormats()` is `formats`, then:
     *  - if `src` is a hw frame, the format of `dst` (if set) must use one of the `formats.dst`;
     *  - if `dst` is a hw frame, the format of `src` must use one of the `formats.src`.
     * 
     * If `dst` is a frame in "reusable" state, allocates memory for it. In that case, if
     * a format is set in `FrameSpecification`, this format will be used, otherwise the first
     * acceptable format will be chosen. The dimensions (if set) must matches the dimensions
     * of `src`, since not all devices support transferring a sub-rectangle of the whole surface.
     * 
     * If `dst` has been allocated, writes contents into the `dst` frame directly.
     * 
     * Throws a exception on failure. In that case, `dst` is not touched.
     */
    static Transfer(dst: Frame, src: Frame): void;
    /**
     * Allocate a new frame attached to the HWFramesContext.
     * If `reuse` is not null, the provided `Frame` instance will be reused, refilled
     * with newly allocated buffers. In that case, returns the original `reuse` instance.
     * Otherwise, a new `Frame` instance is created and returned.
     * Throws an exception on failure.
     * 
     * Note that if `reuse` is not null, it must be in "reusable" state, with the default
     * specification (i.e. created by `Frame.MakeUnallocated({})`, or disposed by
     * `Frame.disposeReusable({})`).
     */
    getBuffer(reuse: (Frame | null)): Frame;
    /** Returns true if the underlying context is identical. */
    equalTo(other: HWFramesContext): boolean;
    /**
     * Get a list of possible formats usable when transfer the data from/to frame created
     * from this `HWFramesContext`.
     * Throws an exception on failure.
     */
    queryTransferFormats(): HWFrameTransferFormatsInfo;
}
/**
 * `MediaIOContext` is a standard interface for other multimedia interfaces to
 * read and write binary data as bytestream. This API is synchronous.
 * The context is either readable or writable, never duplex.
 * 
 * The context can be backed by the native implementation or user's implementation
 * in JavaScript. Once the context is occupied by a consumer (like `AVDecoder`) or
 * producer (like `AVEncoder`), it becomes locked, and any operation from JavaScript,
 * except `dispose()` and `isLocked()`, will throw an exception, until the consumer
 * or producer unlock it.
 */
export class MediaIOContext {
    private constructor();
    readonly writable: boolean;
    readonly readable: boolean;
    /**
     * Gets the size of stream in bytes. For write streams, size is updated each
     * time a successful writeout ends up further position-wise.
     * Throws an exception if the size is unmeasurable.
     */
    readonly sizeInBytes: bigint;
    /**
     * Returns all the supported protocols that can be used in `OpenURL()`.
     * 
     * @param outputProto  Get output protocols if true; otherwise, get input protocols.
     */
    static GetSupportedProtocols(outputProto: boolean): string[];
    /**
     * Opens a context for accessing the resource indicated by the given URL.
     * URL may be a local file, a network media, or something else supported.
     * Note that if URL is opened in read+write mode, the context ONLY can be used to write data.
     * 
     * @param url          URL of the resource
     * @param flags        Flags to enable or disable some features
     * @param options      A dictionary filled with protocol-private options.
     *                     Invalid options are ignored.
     * @param interruptCb  An interrupt callback, may be null, to be used at the protocols level.
     *                     During blocking operations, if the callback returns true, the blocking
     *                     operation will be aborted.
     */
    static OpenURL(url: string, flags: MediaIOFlags, options: (null | Map<string, string>), interruptCb: (null | (() => boolean))): MediaIOContext;
    /**
     * Opens a context backed by a dynamic (resizable) memory stream, write-only.
     * Written data will be stored in a memory buffer.
     * Both `dispose()` and `disposeMemoryDynamic()` can be used to release the context,
     * but the latter returns an `ArrayBuffer` that contains the written data.
     */
    static OpenDynamicMemory(): MediaIOContext;
    /**
     * Open a context backed by a user-implemented backend.
     * See `MediaIOBackend` for more details.
     */
    static OpenFrom(backend: MediaIOBackend): MediaIOContext;
    isLocked(): boolean;
    /**
     * Free the IO context and all the resources associated with it.
     * Once the context is disposed, any operation will throw an exception.
     * If the context is locked, disposing of underlying resource will be delayed until
     * the context is unlocked, but the `MediaIOContext` instance still behaves like a
     * disposed instance.
     */
    dispose(): void;
    /**
     * Like `dispose()`, free the IO context and all the resources associated with it.
     * But it is for context opened by `OpenDynamicMemory()` only, throwing an exception
     * when called on other type of contexts.
     * 
     * Unlike `dispose()`, this must NOT be called on a locked context.
     * 
     * @returns An `ArrayBuffer` that contains the written data.
     */
    disposeMemoryDynamic(): ArrayBuffer;
    /**
     * Write specified data into the stream.
     * Throws an exception when the context is not writable, or an IO error occurred.
     * Length of `src` must be not larger than INT32_MAX.
     */
    write(src: Uint8Array): void;
    /**
     * Read no more than `dst.byteLength` bytes into `dst` buffer.
     * Always refill `dst` completely unless the stream reaches end (EOF).
     * Throws an exception when the context is not readable, or an IO error occurred.
     * Length of `dst` must be not larger than INT32_MAX.
     * 
     * @returns Number of bytes read.
     */
    read(dst: Uint8Array): i32;
    /**
     * Read no more than `dst.byteLength` bytes into `dst` buffer.
     * Unlike `read()`, this method allows reading fewer bytes than requested,
     * even though the stream does not reach its end. The missing bytes can be read
     * in the next call, and at least 1 byte is read in each call.
     * Useful to reduce latency in certain cases.
     * Length of `dst` must be not larger than INT32_MAX.
     * 
     * @returns Number of bytes read.
     */
    readPartial(dst: Uint8Array): i32;
    /**
     * Sets the current position of stream to the specified `offset`.
     * Throws if an error occurs.
     * 
     * @returns The new position from the beginning of stream, measured in bytes.
     */
    seek(offset: bigint, whence: SeekWhence): bigint;
    /**
     * Force flushing of buffered data.
     * 
     * For write streams, force the buffered data to be immediately written to the output,
     * without waiting to fill the internal buffer.
     * 
     * For read streams, discard all currently buffered data, and advance the
     * reported file position to that of the underlying stream. This does not
     * read new data, and does not perform any seeks.
     */
    flush(): void;
}
/**
 * `Packet` is a reference of a underlying binary buffer, which stores compressed
 * media data. It is typically exported by demuxers and then passed as input to
 * decoders, or received as output from encoders and then passed to muxers.
 * 
 * The underlying buffer is reference-counted, each instance of `Packet` should
 * be considered as a reference to a buffer. However, side data in the packet are
 * not shared among instances. They will be copied when a packet is cloned, and will
 * be freed when a packet is disposed.
 * 
 * For video, it should typically contain one compressed frame. For audio it may
 * contain several compressed frames. Encoders are allowed to output empty packets,
 * with no compressed data, containing only side data. (e.g. to update some stream
 * parameters at the end of encoding).
 */
export class Packet {
    private constructor();
    /**
     * Presentation timestamp in the stream's timebase units; the time at which
     * the decompressed packet will be presented to the user.
     * Can be `null` if it is not stored in the file.
     * pts MUST be larger or equal to dts as presentation cannot happen before
     * decompression, unless one wants to view hex dumps. Some formats misuse
     * the terms dts and pts/cts to mean something different. Such timestamps
     * must be converted to true pts/dts before they are stored in packet.
     */
    readonly pts: (null | i64);
    /**
     * Decompression timestamp in the stream's timebase units; the time at which
     * the packet is decompressed.
     * Can be `null` if it is not stored in the file.
     */
    readonly dts: (null | i64);
    /**
     * Duration of this packet in the stream's timebase units, `null` if unknown,
     * never 0. Equals `next_pts - this_pts` in presentation order.
     */
    readonly duration: (null | i64);
    readonly streamIndex: i32;
    /** A combination of `PacketFlags.*` values. */
    readonly flags: i32;
    /**
     * Create a new instance that shares the same underlying buffer, and copies
     * side data (if we have).
     * This operation increases the refcount of the underlying buffer.
     */
    clone(): Packet;
    /**
     * Destroy the instance and reference (including side data).
     * This operation decreases the refcount of the underlying buffer.
     */
    dispose(): void;
    /**
     * Only deletes the reference to underlying buffer and frees side data.
     * Like `dispose()`, the instance will become invalid, but the difference is
     * that it can be reused by functions like `FormatDemuxer.readFrame(packet)`.
     */
    disposeReusable(): void;
}
/**
 * Immutable object, represents a rational number x (x ∈ ℚ) accurately, where x = num / den.
 * Basic arithmetic calculations are supported.
 * 
 * While rational numbers can be expressed as floating-point numbers, the conversion process
 * is a lossy one, so are floating-point operations. On the other hand, the nature of multimedia
 * demands highly accurate calculation of timestamps. This set of rational number utilities
 * serves as a generic interface for manipulating rational numbers as pairs of numerators and
 * denominators.
 */
export class Rational {
    /**
     * Constructs a rational number from a pair of specified numerator and denominator.
     * Both `num` and `den` must be integers, and `den` must not be zero.
     * Note that the given fraction will be reduced (simplified):
     *     new Rational(20, 10) => 2:1
     *     new Rational(-20, -10) => 2:1
     *     new Rational(0, 100) => 0:1
     *     ...
     * 
     * If the fraction is negative, sign is carried by the numerator.
     */
    constructor(num: i32, den: i32);
    readonly num: i32;
    readonly den: i32;
    toString(delimiter: string): string;
    add(r: Rational): Rational;
    sub(r: Rational): Rational;
    mul(r: Rational): Rational;
    scale(x: f64): f64;
    inv(): Rational;
    div(r: Rational): Rational;
    toFloat64(): f64;
}
/**
 * Represents a hardware device for video decoding or encoding. Instances of `HWDeviceContext`
 * are references to the underlying device handle, and the underlying device resources will be
 * disposed when there are no references to it.
 * 
 * After the `HWDeviceContext` is passed to consumers, like `CodecContext` or `HWFramesContext`,
 * they will create references to the underlying device handle internally, and it is safe to
 * dispose the `HWDeviceContext` instance itself.
 * 
 * To create a new `HWDeviceContext`:
 *   - call `HWDeviceContext.MakeVulkan()` to create a new context. This method creates a new
 *     separated Vulkan logical device.
 *   - obtain an instance from `present.Surface.getVideoDecodeCompatibleDevice()`. This method
 *     returns a deocde-only context that shares the same logical device with the corresponding
 *     `present.Surface` surface. It makes the zerocopy texture sharing available on that surface.
 */
export class HWDeviceContext {
    private constructor();
    /**
     * Creates a new context using a separated Vulkan logical device.
     * Throws an exception on failure.
     */
    static MakeVulkan(): HWDeviceContext;
    /** Dispose this reference to the underlying device handle. */
    dispose(): void;
}
/** Create `Frame` instances from various sources. */
export class PixelFrameFactory {
    private constructor();
    /**
     * Make a `Frame` instance whose underlying buffers are filled with pixels decoded from the
     * given data. Input data could be encoded image formats, including JPG, PNG, Webp, GIF, etc.
     * 
     * Throws an exception on failure.
     */
    static FromEncoded(data: Uint8Array, options: FrameMakeFromEncodedOptions): Frame;
    /**
     * Make a `Frame` instance by copying pixels in `image`. It is impossible to safely create a `Frame`
     * from `renderer.Image` without copy.
     * 
     * Detecting the accurate color space info from `Image` is hard, since it only stores a transform
     * matrix and a gamma curve in numeric representation. So user must provide color space info manually.
     */
    static FromImage(image: _renderer.Image, colorPrimaries: ColorPrimaries, colorTrc: ColorTransferCharacteristic): Frame;
}
/**
 * A helper function that reads packets from the specified demuxer, decodes them automatically,
 * and returns decoded frames in each iteration. It has the same effect to manually creating
 * codec contexts and sending, receiving frames, but more convenient for simple situations.
 * 
 * The function itself is not a generator function, but it returns an iterable `Generator` object,
 * which is equivalent to the result of calling a generator function.
 * 
 * Note that the yielded `Frame` instance keeps valid until the next iteration.
 */
export function IterateMediaFrames(demuxer: FormatDemuxer, options: FrameIterationOptions): Generator<FrameIterationResult, FrameIterationResult>;
