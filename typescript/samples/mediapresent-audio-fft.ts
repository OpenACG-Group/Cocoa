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

import * as std from 'core';
import * as utau from 'utau';
import * as gl from 'glamor';
import { FFT } from './fft';
import { CloseRequestEvent, ToplevelWindow } from "../vizmoe/render/toplevel-window";
import { CompositeRenderNode, PaintRenderNode } from "../vizmoe/render/render-node";
import { Rect } from "../vizmoe/render/rectangle";
import { PaintEvent } from '../vizmoe/render/draw-context';
import { DrawContextSubmitter } from "../vizmoe/render/draw-context-submitter";
import { Vector2f } from "../vizmoe/render/vector";

const COVER_RADIUS = 150;        // pixels
const COVER_ANGULAR_SPEED = 0.1; // rad/s

function slurp(a: number, b: number, t: number) {
    return (a - b) * t + a;
}

function roundToNextPow2(int: number): number {
    int--;
    int |= int >> 1;
    int |= int >> 2;
    int |= int >> 4;
    int |= int >> 8;
    int |= int >> 16;
    int++;
    return int;
}

interface FFTResultTriple {
    amplitude: number;
    freq: number;
    phase: number;
}

class AudioFFTContext {
    private readonly fDecoder: utau.AVStreamDecoder;
    private readonly fResampler: utau.AVFilterDAG;

    constructor(decoder: utau.AVStreamDecoder) {
        this.fDecoder = decoder;

        const audioInfo = decoder.getStreamInfo(utau.Constants.STREAM_SELECTOR_AUDIO);

        // Resample the audio stream to a single channel, float64 representation
        this.fResampler = utau.AVFilterDAG.MakeFromDSL(
            '[in] aformat=sample_fmts=flt:channel_layouts=mono [out]',
            [{
                name: 'in',
                mediaType: utau.Constants.MEDIA_TYPE_AUDIO,
                sampleFormat: audioInfo.sampleFormat,
                channelMode: audioInfo.channelMode,
                sampleRate: audioInfo.sampleRate
            }],
            [{
                name: 'out',
                mediaType: utau.Constants.MEDIA_TYPE_AUDIO
            }]
        );
    }

    private resolveFFTResultTriples(fftResult: ArrayLike<number>, q: number): FFTResultTriple[]
    {
        const L = fftResult.length / 2;

        const triples: FFTResultTriple[] = [];
        const terms = q * L;
        for (let i = 0; i < terms; i++) {
            // to reorder the frequencies a little nicer, we pick from the front and back alternatively
            const j = i % 2 == 0 ? i / 2 : L - ((i+1) / 2);
            const x = fftResult[2 * j];
            const y = fftResult[2 * j + 1];
            const freq = ((j + L / 2) % L) - L / 2;

            const amp = Math.sqrt(x * x + y * y) / L;
            triples.push({
                freq: freq,
                amplitude: amp,
                phase: Math.atan2(y, x),
            });
        }

        return triples;
    }

    private resampleToProperFFTSize(input: Float32Array): Float32Array {
        const numSamples = roundToNextPow2(input.length);
        const inputCount = input.length;

        const output = new Float32Array(numSamples);

        for (let i = 0; i < numSamples; i ++) {
            const position = inputCount * (i / numSamples);
            const index = Math.floor(position);
            const nextIndex = (index + 1) % inputCount;
            const amt = position - index;
            output[i] = slurp(input[index], input[nextIndex], amt);
        }

        return output;
    }

    public transform(buffer: utau.AudioBuffer): FFTResultTriple[] {
        this.fResampler.sendFrame('in', buffer);
        const out = this.fResampler.tryReceiveFrame('out');
        if (out.status != utau.Constants.DAG_RECEIVE_STATUS_OK) {
            std.print('Failed to resample audio\n');
        }

        const resampled = out.audio;

        const samplesBuffer = new Float32Array(resampled.samplesCount);
        resampled.read(0, resampled.samplesCount, 0, 0, samplesBuffer.buffer);
        resampled.dispose();

        const fftInput = this.resampleToProperFFTSize(samplesBuffer);
        const fft = new FFT(fftInput.length);
        const fftResult = fft.createComplexArray();
        fft.realTransform(fftResult, fftInput);

        return this.resolveFFTResultTriples(fftResult, 0.4)
                   .sort((a, b) => a.freq - b.freq);
    }
}

class RenderContext {
    private readonly fWidth: number;
    private readonly fHeight: number;
    private readonly fRootNode: CompositeRenderNode;
    private readonly fPaintNode: PaintRenderNode;
    private readonly fCoverCompositeNode: CompositeRenderNode;
    private readonly fCoverImageNode: PaintRenderNode;
    private readonly fBGNode: PaintRenderNode;
    private fMaxFreq: number;
    private fMinFreq: number;

    constructor(width: number, height: number) {
        this.fWidth = width;
        this.fHeight = height;

        this.fRootNode = new CompositeRenderNode();

        this.fBGNode = new PaintRenderNode();
        this.fRootNode.appendChild(this.fBGNode);

        this.fCoverCompositeNode = new CompositeRenderNode();
        this.fRootNode.appendChild(this.fCoverCompositeNode);
        this.fCoverImageNode = new PaintRenderNode();
        this.fCoverCompositeNode.appendChild(this.fCoverImageNode);
        this.fCoverCompositeNode.attributes.setRotate(0, new Vector2f(width / 2, height / 2));

        this.fPaintNode = new PaintRenderNode();
        this.fRootNode.appendChild(this.fPaintNode);

        this.fMinFreq = Infinity;
        this.fMaxFreq = -Infinity;
    }

    public renderCoverImage(image: gl.CkImage): void {
        this.fBGNode.update(Rect.MakeWH(this.fWidth, this.fHeight), C => {
            const paint = new gl.CkPaint();
            paint.setImageFilter(gl.CkImageFilter.MakeFromDSL(
                'blur(10, 10, _, _)', {}
            ));
            C.drawImageRect(
                image,
                [0, 0, image.width, image.height],
                [0, 0, this.fWidth, this.fHeight],
                gl.Constants.SAMPLING_CUBIC_MITCHELL,
                paint,
                gl.Constants.CANVAS_SRC_RECT_CONSTRAINT_FAST
            );
        });

        const R = COVER_RADIUS;

        const bounds = Rect.MakeXYWH(
            this.fWidth / 2 - R,
            this.fHeight / 2 - R,
            R * 2,
            R * 2
        );

        this.fCoverImageNode.update(bounds, C => {
            const clipPath = new gl.CkPath();
            clipPath.addCircle(R, R, R, gl.Constants.PATH_DIRECTION_CW);
            C.clipPath(clipPath, gl.Constants.CLIP_OP_INTERSECT, true);

            const d = R * 2;
            const scalar = Math.min(d / image.width, d / image.height);
            const iW = image.width * scalar, iH = image.height * scalar;
            const imageRect = Rect.MakeXYWH(
                (bounds.width - iW) / 2,
                (bounds.height - iH) / 2,
                iW,
                iH
            );

            C.clear([1, 1, 1, 1]);
            C.drawImageRect(
                image,
                [0, 0, image.width, image.height],
                imageRect.toGLType(),
                gl.Constants.SAMPLING_FILTER_LINEAR,
                null,
                gl.Constants.CANVAS_SRC_RECT_CONSTRAINT_FAST
            );
        });
    }

    private resolveMaxMinFreq(triples: FFTResultTriple[]): void {
        const curMin = triples[0].freq, curMax = triples[triples.length - 1].freq;
        if (curMin < this.fMinFreq) {
            this.fMinFreq = curMin;
        }
        if (curMax > this.fMaxFreq) {
            this.fMaxFreq = curMax;
        }
    }

    public render(triples: FFTResultTriple[]): void {
        if (triples.length == 0) {
            return;
        }

        this.resolveMaxMinFreq(triples);

        const center = new Vector2f(this.fWidth / 2, this.fHeight / 2);

        const rad = (Date.now() / 1e3 * COVER_ANGULAR_SPEED) % (Math.PI * 2);
        this.fCoverCompositeNode.attributes.setRotate(rad, center);

        const bounds = Rect.MakeWH(this.fWidth, this.fHeight);

        const dTheta = Math.PI / 128;
        const R = COVER_RADIUS + 10;

        const path = new gl.CkPath();
        for (let theta = 0; theta < Math.PI * 2; theta += dTheta) {
            // const a = Math.round((triple.freq - this.fMinFreq) / (this.fMaxFreq - this.fMinFreq));
            let n = Math.round(triples.length * (theta / (Math.PI * 2)));
            if (n >= triples.length) {
                n = triples.length - 1;
            }
            const h = triples[n].amplitude * 1000;

            path.moveTo(
                center.x + R * Math.sin(theta),
                center.y - R * Math.cos(theta)
            );
            path.lineTo(
                center.x + (R + h) * Math.sin(theta),
                center.y - (R + h) * Math.cos(theta)
            );
        }

        const paint = new gl.CkPaint();
        paint.setColor4f([0, 0, 1, 1]);
        paint.setAntiAlias(true);
        paint.setStyle(gl.Constants.PAINT_STYLE_STROKE);
        paint.setStrokeCap(gl.Constants.PAINT_CAP_ROUND);
        paint.setStrokeWidth(3);

        this.fPaintNode.update(bounds, C => {
            C.rotate(-rad, center.x, center.y);
            C.drawPath(path, paint);
        });
    }

    public submit(submitter: DrawContextSubmitter): void {
        submitter.submit(this.fRootNode, undefined, false);
    }
}

function extractMediaCoverImage(file: string): gl.CkImage {
    const decoder = utau.AVStreamDecoder.MakeFromFile(file, { disableAudio: true });
    if (!decoder.hasVideoStream) {
        return null;
    }

    let result = decoder.decodeNextFrame();
    while (result.type != utau.Constants.DECODE_BUFFER_VIDEO) {
        if (result.type == utau.Constants.DECODE_BUFFER_EOF ||
            result.type == utau.Constants.DECODE_BUFFER_NULL) {
            return null;
        }
        result = decoder.decodeNextFrame();
    }

    const image = gl.CkImage.MakeFromVideoBuffer(result.videoBuffer);
    result.videoBuffer.dispose();

    return image;
}

const AUDIO_FILE = std.args[0];

async function main(): Promise<void> {
    gl.RenderHost.Initialize({name: 'AudioFFT', major: 1, minor: 0, patch: 0});

    const display = await gl.RenderHost.Connect();
    const window = await ToplevelWindow.Create(display, 800, 600, true);

    await window.setTitle('[Cocoa/Vizmoe] Audio Realtime FFT');

    const ctx = new RenderContext(window.width, window.height);

    ctx.renderCoverImage(extractMediaCoverImage(AUDIO_FILE));

    // Prepare audio decoder
    const decoder = utau.AVStreamDecoder.MakeFromFile(AUDIO_FILE, {disableVideo: true});
    const audioStreamInfo = decoder.getStreamInfo(utau.Constants.STREAM_SELECTOR_AUDIO);

    // Prepare audio playback stream
    const audioDevice = utau.AudioDevice.ConnectPipeWire();
    const audioSinkStream = audioDevice.createSinkStream('Cocoa/Vizmoe AudioFFT');
    audioDevice.unref();
    audioSinkStream.connect(
        audioStreamInfo.sampleFormat,
        audioStreamInfo.channelMode,
        audioStreamInfo.sampleRate,
        true
    );

    // Prepare playback dispatcher
    const dispatcher = new utau.MediaFramePresentDispatcher(decoder, audioSinkStream);

    const fftContext = new AudioFFTContext(decoder);

    dispatcher.onAudioPresentNotify = (buffer) => {
        ctx.render(fftContext.transform(buffer));
        ctx.submit(window.drawContext.submitter);
    };

    dispatcher.onErrorOrEOF = async () => {
        dispatcher.dispose();
        audioSinkStream.dispose();
        await window.close();
        await display.close();
        gl.RenderHost.Dispose();
    };

    // Go!
    dispatcher.play();

    window.addEventListener(CloseRequestEvent, async event => {
        dispatcher.pause();
        dispatcher.dispose();
        audioSinkStream.dispose();

        await event.window.close();
        await display.close();
        gl.RenderHost.Dispose();
    });
}

await main();
