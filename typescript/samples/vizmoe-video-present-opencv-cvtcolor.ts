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

import * as utau from 'utau';
import * as std from 'core';
import * as gl from 'glamor';
import { Mat, OpenCVLib } from '../wasm/opencv/lib/opencv';
import { LoadFromProjectThirdParty } from '../wasm/wasm-loader-polyfill';
import { CloseRequestEvent, ToplevelWindow } from '../vizmoe/render/toplevel-window';
import { CompositeRenderNode, PaintRenderNode } from '../vizmoe/render/render-node';
import { DrawContextSubmitter, SubmittedEvent } from '../vizmoe/render/draw-context-submitter';
import { Rect } from '../vizmoe/render/rectangle';

const cv = await LoadFromProjectThirdParty<OpenCVLib>('opencv_js.wasm', 'opencv.js');

class RenderContext {
    private readonly fDecoder: utau.AVStreamDecoder;
    private readonly fAudioSinkStream: utau.AudioSinkStream;
    private readonly fDispatcher: utau.MediaFramePresentDispatcher;
    private readonly fVInfo: utau.AVDecoderStreamInfo;

    private readonly fRootNode: CompositeRenderNode;
    private readonly fPaintNode: PaintRenderNode;

    private fSubmitter: DrawContextSubmitter;

    private readonly fYUVMat: Mat;
    private fPendingRead: boolean;
    private fDisposed: boolean;

    constructor(file: string) {
        this.fDecoder = utau.AVStreamDecoder.MakeFromFile(file, { useHWDecoding: false });
        if (!this.fDecoder.hasVideoStream || !this.fDecoder.hasAudioStream) {
            throw Error('Media file must have audio and video stream');
        }
        this.fVInfo = this.fDecoder.getStreamInfo(utau.Constants.STREAM_SELECTOR_VIDEO);

        if (this.fVInfo.pixelFormat != utau.Constants.PIXEL_FORMAT_YUV420P) {
            throw Error('Only support YUV420P video');
        }

        const device = utau.AudioDevice.ConnectPipeWire();
        this.fAudioSinkStream = device.createSinkStream('Cocoa/Vizmoe Video Present');
        device.unref();

        const aInfo = this.fDecoder.getStreamInfo(utau.Constants.STREAM_SELECTOR_AUDIO);
        this.fAudioSinkStream.connect(
            aInfo.sampleFormat,
            aInfo.channelMode,
            aInfo.sampleRate,
            true
        );

        this.fDispatcher = new utau.MediaFramePresentDispatcher(
            this.fDecoder,
            this.fAudioSinkStream
        );

        this.fDispatcher.onPresentVideoBuffer = (buffer) => {
            this.onVideoPresent(buffer);
        };

        this.fRootNode = new CompositeRenderNode();
        this.fPaintNode = new PaintRenderNode();
        this.fRootNode.appendChild(this.fPaintNode);

        this.fYUVMat = cv.Mat.zeros(this.fVInfo.height * 1.5, this.fVInfo.width, cv.CV_8UC1);
        this.fSubmitter = null;
        this.fPendingRead = false;
        this.fDisposed = false;
    }

    private onVideoPresent(buffer: utau.VideoBuffer): void {
        if (!this.fSubmitter) {
            return;
        }

        if (this.fPendingRead) {
            return;
        }

        const height = buffer.height, width = buffer.width;

        // Size of the luma plane
        const lumaBytes = width * height;

        // Width and height of the chroma plane
        const crw = width / 2, crh = height / 2;
        const crbytes = crw * crh;

        const yuv = this.fYUVMat;

        // Read YUV components into OpenCV buffer.
        // This is an expensive operation so the three components are read
        // in parallel for better performance.
        this.fPendingRead = true;
        Promise.all([
            buffer.readComponentAsync(
                utau.Constants.PIXEL_COMPONENT_SELECTOR_LUMA,
                yuv.data.subarray(0, lumaBytes),
                width, height, 0, 0, width
            ),

            buffer.readComponentAsync(
                utau.Constants.PIXEL_COMPONENT_SELECTOR_CHROMA_U,
                yuv.data.subarray(lumaBytes, lumaBytes + crbytes),
                crw, crh, 0, 0, crw
            ),

            buffer.readComponentAsync(
                utau.Constants.PIXEL_COMPONENT_SELECTOR_CHROMA_V,
                yuv.data.subarray(lumaBytes + crbytes, lumaBytes + 2 * crbytes),
                crw, crh, 0, 0, crw
            )
        ]).then(() => {
            this.fPendingRead = false;

            if (this.fDisposed) {
                return;
            }

            // Convert the YUV frame into an RGBA bitmap, which is drawable on the window surface.
            const rgba = new cv.Mat();
            cv.cvtColor(yuv, rgba, cv.COLOR_YUV420p2RGBA);

            const bitmap = gl.CkBitmap.MakeFromBuffer(
                rgba.data,
                width,
                height,
                width * 4,
                gl.Constants.COLOR_TYPE_BGRA8888,
                gl.Constants.ALPHA_TYPE_OPAQUE
            );
            bitmap.setImmutable();

            this.fPaintNode.update(Rect.MakeWH(width, height), C => {
                C.drawImage(bitmap.asImage(), 0, 0, gl.Constants.SAMPLING_FILTER_LINEAR, null);
            });

            if (!this.fSubmitter.submit(this.fRootNode, rgba, false).has()) {
                rgba.delete();
            }
        });
    }

    public get width(): number {
        return this.fVInfo.width;
    }

    public get height(): number {
        return this.fVInfo.height;
    }

    public pause(): void {
        this.fDispatcher.pause();
    }

    public play(): void {
        this.fDispatcher.play();
    }

    public setSubmitter(submitter: DrawContextSubmitter): void {
        this.fSubmitter = submitter;
    }

    public dispose(): void {
        this.fDispatcher.pause();
        this.fDispatcher.dispose();
        this.fAudioSinkStream.disconnect();
        this.fAudioSinkStream.dispose();

        this.fYUVMat.delete();

        this.fDisposed = true;
    }
}


const ctx = new RenderContext(std.args[0]);

gl.RenderHost.Initialize({name: 'Video Present', major: 1, minor: 0, patch: 0});

gl.RenderHost.Connect().then((display) => {
    return ToplevelWindow.Create(display, ctx.width, ctx.height, false);
}).then((window) => {
    window.addEventListener(CloseRequestEvent, async () => {
        ctx.dispose();

        const display = window.display;
        await window.close();
        await display.close();
        gl.RenderHost.Dispose();
    });

    window.drawContext.submitter.addEventListener(SubmittedEvent, event => {
        (event.closure as Mat).delete();
    });

    ctx.setSubmitter(window.drawContext.submitter);
    ctx.play();
});
