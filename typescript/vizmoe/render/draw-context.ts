/**
 * This file is part of Vizmoe.
 *
 * Vizmoe is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Vizmoe is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vizmoe. If not, see <https://www.gnu.org/licenses/>.
 */

import * as GL from 'glamor';
import { Event, EventEmitter } from '../base/event-dispatcher';
import { DrawContextSubmitter } from "./draw-context-submitter";

export class PaintEvent extends Event {
    public readonly width: number;
    public readonly height: number;
    public readonly drawContext: DrawContext;

    constructor(w: number, h: number, ctx: DrawContext) {
        super();
        this.width = w;
        this.height = h;
        this.drawContext = ctx;
    }
}

export class ResizeEvent extends Event {
    public readonly width: number;
    public readonly height: number;
    public readonly drawContext: DrawContext;

    constructor(w: number, h: number, ctx: DrawContext) {
        super();
        this.width = w;
        this.height = h;
        this.drawContext = ctx;
    }
}

export class FrameCapturedEvent extends Event {
    public readonly picture: GL.CriticalPicture;
    public readonly drawContext: DrawContext;
    constructor(picture: GL.CriticalPicture, ctx: DrawContext) {
        super();
        this.picture = picture;
        this.drawContext = ctx;
    }
}

export class DrawContext extends EventEmitter {
    private readonly fSurface: GL.Surface;
    private readonly fAggregator: GL.ContentAggregator;
    private fWidth: number;
    private fHeight: number;
    private fLastCaptureSerial: number;
    private readonly fSubmitter: DrawContextSubmitter;

    public static async Make(surface: GL.Surface): Promise<DrawContext> {
        return new DrawContext(surface);
    }

    private constructor(surface_: GL.Surface) {
        super();

        this.fSurface = surface_;
        this.fAggregator = surface_.contentAggregator;
        this.fWidth = surface_.width;
        this.fHeight = surface_.height;
        this.fLastCaptureSerial = 0;
        this.fSubmitter = new DrawContextSubmitter(this);

        this.forwardNative(surface_, 'frame', PaintEvent, () => {
            return new PaintEvent(this.fWidth, this.fHeight, this);
        });

        this.forwardNative(surface_, 'resize', ResizeEvent, (w: number, h: number) => {
            this.fWidth = w;
            this.fHeight = h;
            return new ResizeEvent(w, h, this);
        });

        this.forwardNative(this.fAggregator, 'picture-captured', FrameCapturedEvent,
            (picture: GL.CriticalPicture, serial: number) => {
                if (serial != this.fLastCaptureSerial) {
                    return null;
                }
                return new FrameCapturedEvent(picture, this);
            }
        );
    }

    public get surface() {
        return this.fSurface;
    }

    public get contentAggregator() {
        return this.fAggregator;
    }

    public get width() {
        return this.fWidth;
    }

    public get height() {
        return this.fHeight;
    }

    public get submitter() {
        return this.fSubmitter;
    }

    public dispose(): void {
        this.removeAllListeners(PaintEvent);
        this.removeAllListeners(ResizeEvent);
        this.removeAllListeners(FrameCapturedEvent);
    }

    public async submitSceneOwnership(scene: GL.Scene, captureThis: boolean): Promise<void>
    {
        if (captureThis) {
            this.fLastCaptureSerial =
                await this.fAggregator.captureNextFrameAsPicture();
        }
        return this.fAggregator.update(scene).then(() => {
            scene.dispose();
        });
    }

    public async requestNextFrame(): Promise<void> {
        return this.fSurface.requestNextFrame();
    }

    public async purgeRasterCacheResources(): Promise<void> {
        await this.fAggregator.purgeRasterCacheResources();
    }
}
