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
}

export const Constants: Constants;

export type ABufferEventListenerCallback = (id: number) => void;
export interface ABufferEventListener {
    playing?: ABufferEventListenerCallback;
    cancelled?: ABufferEventListenerCallback;
    consumed?: ABufferEventListenerCallback;
}

export class AudioSinkContext {
    static Initialize(): void;
    static Dispose(callFromListener: boolean): void;
    static Enqueue(buffer: AudioBuffer): number;
    static AddBufferEventListener(listener: ABufferEventListener): number;
    static RemoveBufferEventListener(listenerId: number);
}

export class AudioBuffer {
    readonly sampleFormat: SampleFormat;
    readonly channelMode: ChannelMode;
    readonly sampleRate: number;
    readonly samplesCount: number;
}

export interface AudioInBufferParameters {
    name: string;
    sampleFormat: SampleFormat;
    channelMode: ChannelMode;
    sampleRate: number;
}

export interface AudioOutBufferParameters {
    name: string;
    sampleFormats?: Array<SampleFormat>;
    channelModes?: Array<ChannelMode>;
    sampleRates?: Array<number>;
}

export interface AudioDAGNamedInOutBuffer {
    name: string;
    buffer: AudioBuffer;
}

export class AudioFilterDAG {
    static MakeFromDSL(dsl: string,
                       inparams: Array<AudioInBufferParameters>,
                       outparams: Array<AudioOutBufferParameters>): AudioFilterDAG;

    filter(inBuffers: Array<AudioDAGNamedInOutBuffer>): Array<AudioDAGNamedInOutBuffer>;
}

export interface AVDecoderOptions {
    disableAudio?: boolean;
    disableVideo?: boolean;
    audioCodecName?: string;
    videoCodecName?: string;
}

export interface AVDecoderStreamInfo {
    timeBase: Rational;
    duration: number;
    sampleFormat?: SampleFormat;
    channelMode?: ChannelMode;
    sampleRate?: number;
}

export interface AVDecodeBuffer {
    type: DecodeBufferType;
    audio?: AudioBuffer;
}

export class AVStreamDecoder {
    readonly hasAudioStream: boolean;
    readonly hasVideoStream: boolean;

    static MakeFromFile(path: string, options: AVDecoderOptions): AVStreamDecoder;

    getStreamInfo(selector: StreamSelector): AVDecoderStreamInfo;
    decodeNextFrame(): AVDecodeBuffer;
}
