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
export declare enum PNGFilterFlag {
    Zero,
    None,
    Sub,
    Up,
    Avg,
    Paeth,
    All,
}
export declare enum JPEGAlphaOption {
    Ignore,
    BlendOnBlack,
}
export declare enum JPEGDownsample {
    YUV420,
    YUV422,
    YUV444,
}
export declare enum WebpCompression {
    Lossy,
    Lossless,
}
export interface PNGEncoderOptions {
    filterFlags?: u32;
    zlibLevel?: i32;
    comments?: Uint8Array[];
}
export interface JPEGEncoderOptions {
    quality?: i32;
    downsample?: JPEGDownsample;
    alphaOption?: JPEGAlphaOption;
    xmpMetadata?: Uint8Array;
}
export interface WebpEncoderOptions {
    quality?: f32;
    compression?: WebpCompression;
}
export interface WebpFrame {
    pixmap: _renderer.Pixmap;
    duration: i32;
}
export class PNGEncoder {
    private constructor();
    static EncodeImage(ctx: (_renderer.GpuDirectContext | null), image: _renderer.Image, options: PNGEncoderOptions): ArrayBuffer;
    static EncodePixmap(pixmap: _renderer.Pixmap, options: PNGEncoderOptions): ArrayBuffer;
}
export class JPEGEncoder {
    private constructor();
    static EncodeImage(ctx: (_renderer.GpuDirectContext | null), image: _renderer.Image, options: JPEGEncoderOptions): ArrayBuffer;
    static EncodePixmap(pixmap: _renderer.Pixmap, options: JPEGEncoderOptions): ArrayBuffer;
}
export class WebpEncoder {
    private constructor();
    static EncodeImage(ctx: (_renderer.GpuDirectContext | null), image: _renderer.Image, options: WebpEncoderOptions): ArrayBuffer;
    static EncodePixmap(pixmap: _renderer.Pixmap, options: WebpEncoderOptions): ArrayBuffer;
    static EncodeAnimated(frames: WebpFrame[], options: WebpEncoderOptions): ArrayBuffer;
}
