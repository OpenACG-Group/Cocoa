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

import * as gl from 'glamor';
import * as utau from 'utau';
import * as std from 'core';
import { CloseRequestEvent, ToplevelWindow } from "../vizmoe/render/toplevel-window";
import { CompositeRenderNode, VideoTextureRenderNode } from '../vizmoe/render/render-node';
import { DrawContextSubmitter } from "../vizmoe/render/draw-context-submitter";
import * as GL from "glamor";

class RenderContext {
    private readonly fDecoder: utau.AVStreamDecoder;
    private readonly fAudioSinkStream: utau.AudioSinkStream;
    private readonly fDispatcher: utau.MediaFramePresentDispatcher;
    private readonly fVInfo: utau.AVDecoderStreamInfo;

    private readonly fRootNode: CompositeRenderNode;
    private readonly fVideoNode: VideoTextureRenderNode;

    private fSubmitter: DrawContextSubmitter;

    constructor(file: string) {
        this.fDecoder = utau.AVStreamDecoder.MakeFromFile(file, { useHWDecoding: true });
        if (!this.fDecoder.hasVideoStream || !this.fDecoder.hasAudioStream) {
            throw Error('Media file must have audio and video stream');
        }
        this.fVInfo = this.fDecoder.getStreamInfo(utau.Constants.STREAM_SELECTOR_VIDEO);

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
        this.fVideoNode = new VideoTextureRenderNode();
        this.fRootNode.appendChild(this.fVideoNode);

        this.fSubmitter = null;
    }

    private onVideoPresent(buffer: utau.VideoBuffer): void {
        if (!this.fSubmitter) {
            return;
        }
        this.fVideoNode.update(buffer);
        this.fSubmitter.submit(this.fRootNode, undefined, false);
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
    }
}


const ctx = new RenderContext(std.args[0]);

const presentThread = await GL.PresentThread.Start();

presentThread.createDisplay().then((display) => {
    return ToplevelWindow.Create(display, ctx.width, ctx.height, true);
}).then((window) => {
    window.addEventListener(CloseRequestEvent, async event => {
        ctx.dispose();

        const display = window.display;
        await window.close();
        await display.close();
        presentThread.dispose();
    });

    ctx.setSubmitter(window.drawContext.submitter);
    ctx.play();
});
