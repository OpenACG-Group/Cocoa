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

export type DAGReceiveStatus = number;
export type SampleFormat = number;
export type ChannelMode = number;
export type StreamSelector = number;
export type DecodeBufferType = number;
export type MediaType = number;
export type PixelFormat = number;
export type PixelComponentSelector = number;
export type VideoFrameType = number;

export interface Rational {
    num: number;
    denom: number;
}

export interface Constants {
    DAG_RECEIVE_STATUS_OK: DAGReceiveStatus;
    DAG_RECEIVE_STATUS_ERROR: DAGReceiveStatus;
    DAG_RECEIVE_STATUS_AGAIN: DAGReceiveStatus;
    DAG_RECEIVE_STATUS_EOF: DAGReceiveStatus;

    SAMPLE_FORMAT_UNKNOWN: SampleFormat;
    SAMPLE_FORMAT_U8: SampleFormat;
    SAMPLE_FORMAT_S16: SampleFormat;
    SAMPLE_FORMAT_S32: SampleFormat;
    SAMPLE_FORMAT_F32: SampleFormat;
    SAMPLE_FORMAT_F64: SampleFormat;
    SAMPLE_FORMAT_U8P: SampleFormat;
    SAMPLE_FORMAT_S16P: SampleFormat;
    SAMPLE_FORMAT_S32P: SampleFormat;
    SAMPLE_FORMAT_F32P: SampleFormat;
    SAMPLE_FORMAT_F64P: SampleFormat;

    CH_MODE_MONO: ChannelMode;
    CH_MODE_STEREO: ChannelMode;

    PIXEL_FORMAT_NONE: PixelFormat;
    PIXEL_FORMAT_YUV420P: PixelFormat;
    PIXEL_FORMAT_YUYV422: PixelFormat;
    PIXEL_FORMAT_RGB24: PixelFormat;
    PIXEL_FORMAT_BGR24: PixelFormat;
    PIXEL_FORMAT_YUV422P: PixelFormat;
    PIXEL_FORMAT_YUV444P: PixelFormat;
    PIXEL_FORMAT_YUV410P: PixelFormat;
    PIXEL_FORMAT_YUV411P: PixelFormat;
    PIXEL_FORMAT_GRAY8: PixelFormat;
    PIXEL_FORMAT_MONOWHITE: PixelFormat;
    PIXEL_FORMAT_MONOBLACK: PixelFormat;
    PIXEL_FORMAT_PAL8: PixelFormat;
    PIXEL_FORMAT_YUVJ420P: PixelFormat;
    PIXEL_FORMAT_YUVJ422P: PixelFormat;
    PIXEL_FORMAT_YUVJ444P: PixelFormat;
    PIXEL_FORMAT_UYVY422: PixelFormat;
    PIXEL_FORMAT_UYYVYY411: PixelFormat;
    PIXEL_FORMAT_BGR8: PixelFormat;
    PIXEL_FORMAT_BGR4: PixelFormat;
    PIXEL_FORMAT_BGR4_BYTE: PixelFormat;
    PIXEL_FORMAT_RGB8: PixelFormat;
    PIXEL_FORMAT_RGB4: PixelFormat;
    PIXEL_FORMAT_RGB4_BYTE: PixelFormat;
    PIXEL_FORMAT_NV12: PixelFormat;
    PIXEL_FORMAT_NV21: PixelFormat;
    PIXEL_FORMAT_ARGB: PixelFormat;
    PIXEL_FORMAT_RGBA: PixelFormat;
    PIXEL_FORMAT_ABGR: PixelFormat;
    PIXEL_FORMAT_BGRA: PixelFormat;
    PIXEL_FORMAT_GRAY16: PixelFormat;
    PIXEL_FORMAT_YUV440P: PixelFormat;
    PIXEL_FORMAT_YUVJ440P: PixelFormat;
    PIXEL_FORMAT_YUVA420P: PixelFormat;
    PIXEL_FORMAT_RGB48: PixelFormat;
    PIXEL_FORMAT_RGB565: PixelFormat;
    PIXEL_FORMAT_RGB555: PixelFormat;
    PIXEL_FORMAT_BGR565: PixelFormat;
    PIXEL_FORMAT_BGR555: PixelFormat;
    PIXEL_FORMAT_VAAPI: PixelFormat;
    PIXEL_FORMAT_YUV420P16: PixelFormat;
    PIXEL_FORMAT_YUV422P16: PixelFormat;
    PIXEL_FORMAT_YUV444P16: PixelFormat;
    PIXEL_FORMAT_DXVA2_VLD: PixelFormat;
    PIXEL_FORMAT_RGB444: PixelFormat;
    PIXEL_FORMAT_BGR444: PixelFormat;
    PIXEL_FORMAT_YA8: PixelFormat;
    PIXEL_FORMAT_BGR48: PixelFormat;
    PIXEL_FORMAT_YUV420P9: PixelFormat;
    PIXEL_FORMAT_YUV420P10: PixelFormat;
    PIXEL_FORMAT_YUV422P10: PixelFormat;
    PIXEL_FORMAT_YUV444P9: PixelFormat;
    PIXEL_FORMAT_YUV444P10: PixelFormat;
    PIXEL_FORMAT_YUV422P9: PixelFormat;
    PIXEL_FORMAT_GBRP: PixelFormat;
    PIXEL_FORMAT_GBRP9: PixelFormat;
    PIXEL_FORMAT_GBRP10: PixelFormat;
    PIXEL_FORMAT_GBRP16: PixelFormat;
    PIXEL_FORMAT_YUVA422P: PixelFormat;
    PIXEL_FORMAT_YUVA444P: PixelFormat;
    PIXEL_FORMAT_YUVA420P9: PixelFormat;
    PIXEL_FORMAT_YUVA422P9: PixelFormat;
    PIXEL_FORMAT_YUVA444P9: PixelFormat;
    PIXEL_FORMAT_YUVA420P10: PixelFormat;
    PIXEL_FORMAT_YUVA422P10: PixelFormat;
    PIXEL_FORMAT_YUVA444P10: PixelFormat;
    PIXEL_FORMAT_YUVA420P16: PixelFormat;
    PIXEL_FORMAT_YUVA422P16: PixelFormat;
    PIXEL_FORMAT_YUVA444P16: PixelFormat;
    PIXEL_FORMAT_VDPAU: PixelFormat;
    PIXEL_FORMAT_XYZ12: PixelFormat;
    PIXEL_FORMAT_NV16: PixelFormat;
    PIXEL_FORMAT_NV20: PixelFormat;
    PIXEL_FORMAT_RGBA64: PixelFormat;
    PIXEL_FORMAT_BGRA64: PixelFormat;
    PIXEL_FORMAT_YVYU422: PixelFormat;
    PIXEL_FORMAT_YA16: PixelFormat;
    PIXEL_FORMAT_GBRAP: PixelFormat;
    PIXEL_FORMAT_GBRAP16: PixelFormat;
    PIXEL_FORMAT_QSV: PixelFormat;
    PIXEL_FORMAT_MMAL: PixelFormat;
    PIXEL_FORMAT_D3D11VA_VLD: PixelFormat;
    PIXEL_FORMAT_CUDA: PixelFormat;
    PIXEL_FORMAT_0RGB: PixelFormat;
    PIXEL_FORMAT_RGB0: PixelFormat;
    PIXEL_FORMAT_0BGR: PixelFormat;
    PIXEL_FORMAT_BGR0: PixelFormat;
    PIXEL_FORMAT_YUV420P12: PixelFormat;
    PIXEL_FORMAT_YUV420P14: PixelFormat;
    PIXEL_FORMAT_YUV422P12: PixelFormat;
    PIXEL_FORMAT_YUV422P14: PixelFormat;
    PIXEL_FORMAT_YUV444P12: PixelFormat;
    PIXEL_FORMAT_YUV444P14: PixelFormat;
    PIXEL_FORMAT_GBRP12: PixelFormat;
    PIXEL_FORMAT_GBRP14: PixelFormat;
    PIXEL_FORMAT_YUVJ411P: PixelFormat;
    PIXEL_FORMAT_BAYER_BGGR8: PixelFormat;
    PIXEL_FORMAT_BAYER_RGGB8: PixelFormat;
    PIXEL_FORMAT_BAYER_GBRG8: PixelFormat;
    PIXEL_FORMAT_BAYER_GRBG8: PixelFormat;
    PIXEL_FORMAT_BAYER_BGGR16: PixelFormat;
    PIXEL_FORMAT_BAYER_RGGB16: PixelFormat;
    PIXEL_FORMAT_BAYER_GBRG16: PixelFormat;
    PIXEL_FORMAT_BAYER_GRBG16: PixelFormat;
    PIXEL_FORMAT_YUV440P10: PixelFormat;
    PIXEL_FORMAT_YUV440P12: PixelFormat;
    PIXEL_FORMAT_AYUV64: PixelFormat;
    PIXEL_FORMAT_VIDEOTOOLBOX: PixelFormat;
    PIXEL_FORMAT_P010: PixelFormat;
    PIXEL_FORMAT_GBRAP12: PixelFormat;
    PIXEL_FORMAT_GBRAP10: PixelFormat;
    PIXEL_FORMAT_MEDIACODEC: PixelFormat;
    PIXEL_FORMAT_GRAY12: PixelFormat;
    PIXEL_FORMAT_GRAY10: PixelFormat;
    PIXEL_FORMAT_P016: PixelFormat;
    PIXEL_FORMAT_D3D11: PixelFormat;
    PIXEL_FORMAT_GRAY9: PixelFormat;
    PIXEL_FORMAT_GBRPF32: PixelFormat;
    PIXEL_FORMAT_GBRAPF32: PixelFormat;
    PIXEL_FORMAT_DRM_PRIME: PixelFormat;
    PIXEL_FORMAT_OPENCL: PixelFormat;
    PIXEL_FORMAT_GRAY14: PixelFormat;
    PIXEL_FORMAT_GRAYF32: PixelFormat;
    PIXEL_FORMAT_YUVA422P12: PixelFormat;
    PIXEL_FORMAT_YUVA444P12: PixelFormat;
    PIXEL_FORMAT_NV24: PixelFormat;
    PIXEL_FORMAT_NV42: PixelFormat;
    PIXEL_FORMAT_VULKAN: PixelFormat;
    PIXEL_FORMAT_Y210: PixelFormat;
    PIXEL_FORMAT_X2RGB10: PixelFormat;
    PIXEL_FORMAT_X2BGR10: PixelFormat;
    PIXEL_FORMAT_P210: PixelFormat;
    PIXEL_FORMAT_P410: PixelFormat;
    PIXEL_FORMAT_P216: PixelFormat;
    PIXEL_FORMAT_P416: PixelFormat;
    PIXEL_FORMAT_VUYA: PixelFormat;
    PIXEL_FORMAT_RGBAF16: PixelFormat;
    PIXEL_FORMAT_VUYX: PixelFormat;
    PIXEL_FORMAT_P012: PixelFormat;
    PIXEL_FORMAT_Y212: PixelFormat;
    PIXEL_FORMAT_XV30: PixelFormat;
    PIXEL_FORMAT_XV36: PixelFormat;
    PIXEL_FORMAT_RGBF32: PixelFormat;
    PIXEL_FORMAT_RGBAF32: PixelFormat;

    PIXEL_COMPONENT_SELECTOR_LUMA: PixelComponentSelector;
    PIXEL_COMPONENT_SELECTOR_CHROMA_U: PixelComponentSelector;
    PIXEL_COMPONENT_SELECTOR_CHROMA_V: PixelComponentSelector;
    PIXEL_COMPONENT_SELECTOR_R: PixelComponentSelector;
    PIXEL_COMPONENT_SELECTOR_G: PixelComponentSelector;
    PIXEL_COMPONENT_SELECTOR_B: PixelComponentSelector;
    PIXEL_COMPONENT_SELECTOR_ALPHA: PixelComponentSelector;

    STREAM_SELECTOR_VIDEO: StreamSelector;
    STREAM_SELECTOR_AUDIO: StreamSelector;

    DECODE_BUFFER_AUDIO: DecodeBufferType;
    DECODE_BUFFER_VIDEO: DecodeBufferType;
    DECODE_BUFFER_EOF: DecodeBufferType;
    DECODE_BUFFER_NULL: DecodeBufferType;

    MEDIA_TYPE_AUDIO: MediaType;
    MEDIA_TYPE_VIDEO: MediaType;

    VIDEO_FRAME_TYPE_NONE: VideoFrameType;
    VIDEO_FRAME_TYPE_I: VideoFrameType;
    VIDEO_FRAME_TYPE_B: VideoFrameType;
    VIDEO_FRAME_TYPE_P: VideoFrameType;
    VIDEO_FRAME_TYPE_S: VideoFrameType;
    VIDEO_FRAME_TYPE_BI: VideoFrameType;
    VIDEO_FRAME_TYPE_SI: VideoFrameType;
    VIDEO_FRAME_TYPE_SP: VideoFrameType;
}

export const Constants: Constants;

export function getCurrentTimestampMs(): number;

export interface PixelFormatDescriptor {
    name: string;
    components: Array<{
        plane: number;
        step: number;
        offset: number;
        shift: number;
        depth: number;
    }>;
    hasPalette: boolean;
    isHWAccel: boolean;
    isPlanar: boolean;
    isRGBLike: boolean;
    isBayer: boolean;
    hasAlpha: boolean;
    isFloat: boolean;
    planes: number;
    bitsPerPixel: number;
    chromaWidthRShift: number;
    chromaHeightRShift: number;
}
export function getPixelFormatDescriptor(fmt: PixelFormat): PixelFormatDescriptor;

export class AudioDevice {
    static ConnectPipeWire(): AudioDevice;

    unref(): void;
    createSinkStream(name: string): AudioSinkStream;
}

export class AudioSinkStream {
    public volume: number;
    public onVolumeChanged: (volume: number) => void;

    dispose(): void;
    connect(sampleFormat: SampleFormat, channelMode: ChannelMode, sampleRate: number, realTime: boolean): void;
    disconnect(): void;
    enqueue(buffer: AudioBuffer): void;
    getCurrentDelayInUs(): number;
}

export interface AVGenericBuffer {
    readonly pts: bigint;
    readonly duration: bigint;
}

export class AudioBuffer implements AVGenericBuffer {
    readonly pts: bigint;
    readonly duration: bigint;
    readonly sampleFormat: SampleFormat;
    readonly channelMode: ChannelMode;
    readonly sampleRate: number;
    readonly samplesCount: number;

    read(plane: number, sampleCount: number, sampleOffset: number,
         dstBytesOffset: number, dst: ArrayBuffer): number;

    readChannel(ch: number, sampleCount: number, sampleOffset: number,
                dstBytesOffset: number, dst: ArrayBuffer): number;

    dispose(): void;
    clone(): void;
}

/**
 * `VideoBuffer` represents an image object stored in host memory (CPU memory)
 * or device memory (e.g. GPU memory). Many objects, like `AVStreamDecoder`
 * and `AVFilterDAG`, can produce or consume VideoBuffers.
 *
 * The backing store of video buffer is reference-counted, and each instance
 * of `VideoBuffer` should be treated as a reference to its underlying memory.
 * Calling `VideoBuffer.dispose()` will decrease the reference count of its
 * underlying memory buffer, making the instance become a dangling buffer.
 * If a `VideoBuffer` instance becomes a dangling buffer, user must not use
 * it (call any methods or access any properties except `disposed`).
 *
 * Once the reference count of underlying buffer decreases to zero, it will be
 * released or reused for other frames.
 */
export class VideoBuffer implements AVGenericBuffer {
    readonly disposed: boolean;
    readonly pts: bigint;
    readonly duration: bigint;
    readonly width: number;
    readonly height: number;
    readonly hwframe: boolean;
    readonly frameType: VideoFrameType;
    readonly format: PixelFormat;
    readonly formatName: string;

    /**
     * Copy the specified component of pixels in a rectangular area
     * `(srcX, srcY, sliceW, sliceH)` into `dst` buffer.
     * The rectangular area `(srcX, srcY, sliceW, sliceH)` is considered to be in
     * the component's coordinate system. For example, for a `1920x1080` image in
     * `YUV420P` format, it has `1920x1080` coordinate space for luma component,
     * but only `960x540` coordinate space for chroma (U, V) components.
     */
    readComponent(component: PixelComponentSelector,
                  dst: Uint8Array | Uint16Array | Uint32Array | Float32Array,
                  sliceW: number,
                  sliceH: number,
                  srcX: number,
                  srcY: number,
                  dstStrideInElements: number): void;

    readComponentAsync(component: PixelComponentSelector,
                       dst: Uint8Array | Uint16Array | Uint32Array | Float32Array,
                       sliceW: number,
                       sliceH: number,
                       srcX: number,
                       srcY: number,
                       dstStrideInElements: number): Promise<void>;

    readGrayscale(dst: Uint8Array,
                  sliceW: number,
                  sliceH: number,
                  srcX: number,
                  srcY: number,
                  dstStride: number): void;

    transferHardwareFrameDataTo(expectFormat: PixelFormat): VideoBuffer;
    queryHardwareTransferableFormats(): Array<PixelFormat>;

    /**
     * Decrease the reference count the underlying buffer, and make the object
     * become a dangling buffer.
     */
    dispose(): void;

    /**
     * Create a new `VideoBuffer` instance which shares the same underlying
     * buffer with this object. This operation increases the reference count
     * of the underlying buffer. Returned object is completely equivalent
     * to the original object, but they are different JavaScript instances.
     */
    clone(): VideoBuffer;
}

export interface InBufferParameters {
    name: string;
    mediaType: MediaType;

    sampleFormat?: SampleFormat;
    channelMode?: ChannelMode;
    sampleRate?: number;

    pixelFormat?: number;
    hwFramesContext?: HWFramesContextRef;
    width?: number;
    height?: number;
    timeBase?: Rational;
    SAR?: Rational;
}

export interface OutBufferParameters {
    name: string;
    mediaType: MediaType;
}

interface DAGOutputInfo {
    status: DAGReceiveStatus;
    name?: string;
    mediaType?: MediaType;
    audio?: AudioBuffer;
    video?: VideoBuffer;
}

export class AVFilterDAG {
    static MakeFromDSL(dsl: string, inparams: Array<InBufferParameters>,
                       outparams: Array<OutBufferParameters>): AVFilterDAG;

    sendFrame(name: string, frame: AudioBuffer | VideoBuffer): void;
    tryReceiveFrame(name: string): DAGOutputInfo;
}

export class HWFramesContextRef {
    private constructor();

    dispose(): void;
    clone(): HWFramesContextRef;
}

export interface AVDecoderOptions {
    disableAudio?: boolean;
    disableVideo?: boolean;
    useHWDecoding?: boolean;
    audioCodecName?: string;
    videoCodecName?: string;
}

export interface AVDecoderStreamInfo {
    timeBase: Rational;
    duration: number;

    sampleFormat?: SampleFormat;
    channelMode?: ChannelMode;
    sampleRate?: number;

    pixelFormat?: number;
    width?: number;
    height?: number;
    SAR?: Rational;
}

export interface AVDecodeBuffer {
    type: DecodeBufferType;

    audioBuffer?: AudioBuffer;
    videoBuffer?: VideoBuffer;
}

export class AVStreamDecoder {
    readonly hasAudioStream: boolean;
    readonly hasVideoStream: boolean;

    static MakeFromFile(path: string, options: AVDecoderOptions): AVStreamDecoder;

    getStreamInfo(selector: StreamSelector): AVDecoderStreamInfo;
    decodeNextFrame(): AVDecodeBuffer;
    seekStreamTo(selector: StreamSelector, timestamp: number): void;
    flushDecoderBuffers(selector: StreamSelector): void;
    refHWFramesContext(): null | HWFramesContextRef;
}

export class MediaFramePresentDispatcher {
    onPresentVideoBuffer: (buffer: VideoBuffer, ptsInSeconds: number) => void;
    onAudioPresentNotify: (buffer: AudioBuffer, ptsInSeconds: number) => void;
    onErrorOrEOF: () => void;

    constructor(decoder: AVStreamDecoder, audioSinkStream: AudioSinkStream);

    play(): void;
    pause(): void;
    seekTo(tsSeconds: number): void;
    dispose(): void;
}
