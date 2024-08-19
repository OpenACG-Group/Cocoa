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

import { Surface, ContentAggregator, Scene } from 'present';
import { Event, EventEmitter } from '../../core/event-dispatcher';

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

export class DrawContext extends EventEmitter {
    private readonly fSurface: Surface;
    private readonly fAggregator: ContentAggregator;
    private fWidth: number;
    private fHeight: number;

    public static async Make(surface: Surface): Promise<DrawContext> {
        return new DrawContext(surface);
    }

    private constructor(surface_: Surface) {
        super();

        this.fSurface = surface_;
        this.fAggregator = surface_.contentAggregator;
        this.fWidth = surface_.dimensions[0];
        this.fHeight = surface_.dimensions[1];

        this.forwardNative(surface_, 'frame', PaintEvent, () => {
            return new PaintEvent(this.fWidth, this.fHeight, this);
        });

        this.forwardNative(surface_, 'resize', ResizeEvent, (w: number, h: number) => {
            this.fWidth = w;
            this.fHeight = h;
            return new ResizeEvent(w, h, this);
        });
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

    public dispose(): void {
        this.removeAllListeners(PaintEvent);
        this.removeAllListeners(ResizeEvent);
    }

    public async submit(scene: Scene): Promise<number> {
        return this.fAggregator.update(scene);
    }

    public async requestNextFrame(): Promise<number> {
        return this.fSurface.requestNextFrame();
    }

    public async purgeRasterCacheResources(): Promise<void> {
        await this.fAggregator.purgeRasterCacheResources();
    }
}
