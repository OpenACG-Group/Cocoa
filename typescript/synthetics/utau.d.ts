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

export type SampleFormat = number;
export type ChannelMode = number;
export type StreamSelector = number;
export type DecodeBufferType = number;
export type MediaType = number;

export interface Rational {
    num: number;
    denom: number;
}

export interface Constants {
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

    STREAM_SELECTOR_VIDEO: StreamSelector;
    STREAM_SELECTOR_AUDIO: StreamSelector;

    DECODE_BUFFER_AUDIO: DecodeBufferType;
    DECODE_BUFFER_VIDEO: DecodeBufferType;
    DECODE_BUFFER_EOF: DecodeBufferType;
    DECODE_BUFFER_NULL: DecodeBufferType;

    MEDIA_TYPE_AUDIO: MediaType;
    MEDIA_TYPE_VIDEO: MediaType;
}

export const Constants: Constants;

export function getCurrentTimestampMs(): number;

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
    readonly pts: number;
}

export class AudioBuffer implements AVGenericBuffer {
    readonly pts: number;
    readonly sampleFormat: SampleFormat;
    readonly channelMode: ChannelMode;
    readonly sampleRate: number;
    readonly samplesCount: number;

    dispose(): void;
}

export class VideoBuffer implements AVGenericBuffer {
    readonly pts: number;

    dispose(): void;
}

export interface InBufferParameters {
    name: string;
    mediaType: MediaType;

    sampleFormat?: SampleFormat;
    channelMode?: ChannelMode;
    sampleRate?: number;

    pixelFormat?: number;
    hwFrameContextFrom?: VideoBuffer;
    width?: number;
    height?: number;
    timeBase?: Rational;
    SAR?: Rational;
}

export interface OutBufferParameters {
    name: string;
    mediaType: MediaType;
}

export interface DAGNamedInOutBuffer {
    name: string;
    mediaType: MediaType;

    audioBuffer?: AudioBuffer;
    videoBuffer?: VideoBuffer;
}

export class AVFilterDAG {
    static MakeFromDSL(dsl: string, inparams: Array<InBufferParameters>,
                       outparams: Array<OutBufferParameters>): AVFilterDAG;

    filter(inBuffers: Array<DAGNamedInOutBuffer>): Array<DAGNamedInOutBuffer>;
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
}

export class MediaFramePresentDispatcher {
    onPresentVideoBuffer: (buffer: VideoBuffer, ptsInSeconds: number) => void;
    onErrorOrEOF: () => void;

    constructor(decoder: AVStreamDecoder, audioSinkStream: AudioSinkStream);

    play(): void;
    pause(): void;
    seekTo(tsSeconds: number): void;
    dispose(): void;
}
