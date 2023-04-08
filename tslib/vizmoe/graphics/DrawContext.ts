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

import * as GL from 'synthetic://glamor';
import { Buffer } from 'synthetic://core';
import { Event, EventEmitter } from '../base/EventDispatch';
import { Rect } from './Rect';

export class PaintEvent extends Event {
    public readonly width: number;
    public readonly height: number;

    constructor(w: number, h: number) {
        super();
        this.width = w;
        this.height = h;
    }
}

export class ResizeEvent extends Event {
    public readonly width: number;
    public readonly height: number;

    constructor(w: number, h: number) {
        super();
        this.width = w;
        this.height = h;
    }
}

export class FrameCapturedEvent extends Event {
    public readonly picture: GL.CriticalPicture;
    constructor(picture: GL.CriticalPicture) {
        super();
        this.picture = picture;
    }
}

export class DrawContext extends EventEmitter {
    private surface_: GL.Surface;
    private blender_: GL.Blender;
    private surface_slots_: GL.SlotID[];
    private blender_slots_: GL.SlotID[];
    private width_: number;
    private height_: number;
    private last_capture_serial_: number;

    public static async Make(surface: GL.Surface): Promise<DrawContext> {
        const blender = await surface.createBlender();
        return new DrawContext(surface, blender);
    }

    private constructor(surface_: GL.Surface, blender_: GL.Blender) {
        super();

        this.surface_ = surface_;
        this.blender_ = blender_;
        this.width_ = surface_.width;
        this.height_ = surface_.height;
        this.last_capture_serial_ = 0;

        this.surface_slots_ = [
            surface_.connect('frame', () => {
                super.emitEvent(new PaintEvent(this.width_, this.height_))
            }),
            surface_.connect('resize', (w: number, h: number) => {
                this.width_ = w;
                this.height_ = h;
                super.emitEvent(new ResizeEvent(w, h));
            })
        ];

        this.blender_slots_ = [
            blender_.connect('picture-captured', (picture: GL.CriticalPicture, serial: number) => {
                if (serial != this.last_capture_serial_) {
                    return;
                }
                super.emitEvent(new FrameCapturedEvent(picture));
            })
        ];

        super.registerEvent(PaintEvent);
        super.registerEvent(ResizeEvent);
        super.registerEvent(FrameCapturedEvent);
    }

    public get surface() {
        return this.surface_;
    }

    public get blender() {
        return this.blender_;
    }

    public get width() {
        return this.width_;
    }

    public get height() {
        return this.height_;
    }

    public async dispose(): Promise<void> {
        for (const id of this.blender_slots_) {
            this.blender_.disconnect(id);
        }
        this.blender_slots_ = [];

        for (const id of this.surface_slots_) {
            this.surface_.disconnect(id);
        }
        this.surface_slots_ = [];

        return this.blender_.dispose();
    }

    public async submitSceneOwnership(scene: GL.Scene,
                                      captureThis: boolean = false): Promise<void>
    {
        if (captureThis) {
            this.last_capture_serial_ =
                await this.blender_.captureNextFrameAsPicture();
        }
        return this.blender_.update(scene).then(() => {
            scene.dispose();
        });
    }

    public async requestNextFrame(): Promise<void> {
        return this.surface_.requestNextFrame();
    }

    public async purgeRasterCacheResources(): Promise<void> {
        this.blender_.purgeRasterCacheResources();
    }
}

export class TextureDeletionEvent extends Event {
    constructor() {
        super();
    }
}

export class PersistentTexture extends EventEmitter {
    private draw_context_: DrawContext;
    private texture_id_: number;
    private width_: number;
    private height_: number;
    private texture_delete_slot_: GL.SlotID;

    public static async MakeFromImage(dc: DrawContext, image: GL.CkImage,
                                      annotation?: string): Promise<PersistentTexture>
    {
        if (annotation === undefined) {
            annotation = '#anonymous-texture';
        }
        const id = await dc.blender.createTextureFromImage(image, annotation);
        return PersistentTexture.MakeFromId(dc, id, image.width, image.height);
    }

    public static async MakeFromPixmap(dc: DrawContext,
                                       pixels: Buffer,
                                       width: number,
                                       height: number,
                                       colorType: GL.ColorType,
                                       alphaType: GL.AlphaType,
                                       annotation?: string): Promise<PersistentTexture>
    {
        if (annotation === undefined) {
            annotation = '#anonymous-texture';
        }
        const id = await dc.blender.createTextureFromPixmap(pixels, width, height,
                                                            colorType, alphaType, annotation);
        return PersistentTexture.MakeFromId(dc, id, width, height);
    }

    private static async MakeFromId(dc: DrawContext, id: GL.TextureId,
                                    w: number, h: number): Promise<PersistentTexture>
    {
        const signame = `texture#${id}`;
        await dc.blender.newTextureDeletionSubscriptionSignal(id, signame);
        return new PersistentTexture(dc, id, w, h, signame);
    }

    private constructor(dc: DrawContext, id: number, w: number,
                        h: number, signame: string)
    {
        super();
        this.draw_context_ = dc;
        this.texture_id_ = id;
        this.width_ = w;
        this.height_ = h;

        super.registerEvent(TextureDeletionEvent);

        this.texture_delete_slot_ = dc.blender.connect(signame, () => {
            // As the signal will only be emitted once, we can disconnect to it now.
            this.draw_context_.blender.disconnect(this.texture_delete_slot_);
            super.emitEvent(new TextureDeletionEvent());

            // Reset all the properties to indicate this texture is invalidated
            this.draw_context_ = null;
            this.texture_id_ = -1;
            this.width_ = 0;
            this.height_ = 0;
        });
    }

    private checkTextureIsDeleted(): void {
        if (this.draw_context_ == null) {
            throw Error('Texture has already been deleted');
        }
    }

    public get drawContext() {
        return this.draw_context_;
    }

    public get textureId() {
        return this.draw_context_;
    }

    public get width() {
        return this.width_;
    }

    public get height() {
        return this.height_;
    }

    public addTextureToScene(builder: GL.SceneBuilder, rect?: Rect,
                             sampling?: GL.SamplingOption): void
    {
        this.checkTextureIsDeleted();

        if (rect === undefined) {
            // Use the original geometry of texture
            rect = Rect.MakeWH(this.width_, this.height_);
        }
        if (sampling === undefined) {
            // Use linear filter by default
            sampling = GL.Constants.SAMPLING_FILTER_LINEAR;
        }
        builder.addTexture(this.texture_id_, rect.left, rect.top,
                           rect.width, rect.height, sampling);
    }

    public async delete(): Promise<void> {
        this.checkTextureIsDeleted();
    
        // When the texture is deleted on rendering thread, the deletion signal
        // will be emitted.
        return this.draw_context_.blender.deleteTexture(this.texture_id_);
    }
}
