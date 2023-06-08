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
import { DrawContext } from "./draw-context";
import { Event, EventEmitter } from "../base/event-dispatcher";
import { Buffer } from 'core';
import { Rect } from "./rectangle";

export class TextureDeletionEvent extends Event {
    constructor() {
        super();
    }
}

export class StaticTexture extends EventEmitter {
    private fDrawContext: DrawContext;
    private fTextureId: number;
    private fWidth: number;
    private fHeight: number;
    private readonly fTexDeletionSlot: GL.SlotID;

    public static async MakeFromImage(dc: DrawContext, image: GL.CkImage,
                                      annotation?: string): Promise<StaticTexture>
    {
        if (annotation === undefined) {
            annotation = '#anonymous-texture';
        }
        const id = await dc.blender.createTextureFromImage(image, annotation);
        return StaticTexture.MakeFromId(dc, id, image.width, image.height);
    }

    public static async MakeFromPixmap(dc: DrawContext,
                                       pixels: Buffer,
                                       width: number,
                                       height: number,
                                       colorType: GL.ColorType,
                                       alphaType: GL.AlphaType,
                                       annotation?: string): Promise<StaticTexture>
    {
        if (annotation === undefined) {
            annotation = '#anonymous-texture';
        }
        const id = await dc.blender.createTextureFromPixmap(pixels.byteArray, width, height,
            colorType, alphaType, annotation);
        return StaticTexture.MakeFromId(dc, id, width, height);
    }

    private static async MakeFromId(dc: DrawContext, id: GL.TextureId,
                                    w: number, h: number): Promise<StaticTexture>
    {
        const signame = `texture#${id}`;
        await dc.blender.newTextureDeletionSubscriptionSignal(id, signame);
        return new StaticTexture(dc, id, w, h, signame);
    }

    private constructor(dc: DrawContext, id: number, w: number,
                        h: number, signame: string)
    {
        super();
        this.fDrawContext = dc;
        this.fTextureId = id;
        this.fWidth = w;
        this.fHeight = h;

        super.registerEvent(TextureDeletionEvent);

        this.fTexDeletionSlot = dc.blender.connect(signame, () => {
            // As the signal will only be emitted once, we can disconnect to it now.
            this.fDrawContext.blender.disconnect(this.fTexDeletionSlot);
            super.emitEvent(new TextureDeletionEvent());

            // Reset all the properties to indicate this texture is invalidated
            this.fDrawContext = null;
            this.fTextureId = -1;
            this.fWidth = 0;
            this.fHeight = 0;
        });
    }

    private checkTextureIsDeleted(): void {
        if (this.fDrawContext == null) {
            throw Error('Texture has already been deleted');
        }
    }

    public get drawContext() {
        return this.fDrawContext;
    }

    public get textureId() {
        return this.fDrawContext;
    }

    public get width() {
        return this.fWidth;
    }

    public get height() {
        return this.fHeight;
    }

    public addTextureToScene(builder: GL.SceneBuilder, rect?: Rect,
                             sampling?: GL.SamplingOption): void
    {
        this.checkTextureIsDeleted();

        if (rect === undefined) {
            // Use the original geometry of texture
            rect = Rect.MakeWH(this.fWidth, this.fHeight);
        }
        if (sampling === undefined) {
            // Use linear filter by default
            sampling = GL.Constants.SAMPLING_FILTER_LINEAR;
        }
        builder.addTexture(this.fTextureId, rect.left, rect.top,
            rect.width, rect.height, sampling);
    }

    public async delete(): Promise<void> {
        this.checkTextureIsDeleted();

        // When the texture is deleted on rendering thread, the deletion signal
        // will be emitted.
        return this.fDrawContext.blender.deleteTexture(this.fTextureId);
    }
}