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

import {
    CkImageFilter,
    BlendMode,
    CkRect,
    CkRRect,
    CkMatrix,
    Scene,
    SceneBuilder,
    CkPictureRecorder,
    CkPicture,
    SamplingOption
} from 'synthetic://glamor';

import { VideoBuffer } from 'synthetic://utau';

import { LinkedList } from '../../core/linked_list';
import { PainterRenderObject } from "./PainterRenderObject"
import { Rect } from '../graphics/Rect';
import { Vector2f } from '../graphics/Vector';

enum IROpcode {
    PUSH_OFFSET,
    PUSH_ROTATE,
    PUSH_BACKDROP_FILTER,
    PUSH_IMAGE_FILTER,
    PUSH_RECT_CLIP,
    PUSH_RRECT_CLIP,
    PUSH_OPACITY,
    POP,

    ISOLATE_LAYER,
    
    DRAW_PAINTER,
    DRAW_TEXTURE,
    DRAW_VIDEO_BUFFER,
}

interface IROperand {
    painter?: PainterRenderObject;
    video_buffer?: VideoBuffer;
    filter?: CkImageFilter;
    blend_mode?: BlendMode;
    rect?: CkRect;
    rrect?: CkRRect;
    matrix?: CkMatrix;
    vector?: Vector2f;
    bool?: boolean;
    scalar?: number;
    int?: number;
}

interface IRInst {
    opcode: IROpcode;
    operands: Array<IROperand>;
}

const PICT_UNUSED_LIFECYCLE_THRESHOLD = 8;

/**
 * A complex Picture is a Picture which consists of one or more
 * Painter objects.
 */
class ComplexPicture {
    private picture_: CkPicture;
    private layered_painter_ids_: Array<number>;
    private unused_lifecycle_: number;

    public usedThisFrame: boolean;

    constructor(picture: CkPicture, painters: Array<PainterRenderObject>) {
        this.picture_ = picture;
        this.layered_painter_ids_ = [];
        for (const painter of painters) {
            this.layered_painter_ids_.push(painter.uniqueNodeId);
        }
        this.unused_lifecycle_ = 0;
        this.usedThisFrame = false;
    }

    public get picture(): CkPicture {
        return this.picture_;
    }

    public markUnusedLifeCycle(): void {
        this.unused_lifecycle_++;
    }

    public markUsedLifeCycle(): void {
        this.unused_lifecycle_ = 0;
        this.usedThisFrame = true;
    }

    public isExpired(): boolean {
        return (this.unused_lifecycle_ >= PICT_UNUSED_LIFECYCLE_THRESHOLD);
    }

    public equalToPainterSequence(seq: Array<PainterRenderObject>): boolean {
        if (seq.length != this.layered_painter_ids_.length) {
            return false;
        }
        for (let i = 0; i < seq.length; i++) {
            // The Painter node itself has changed or it has been
            // repainted (dirty node).
            if (seq[i].uniqueNodeId != this.layered_painter_ids_[i] || seq[i].isDirty) {
                return false;
            }
        }
        return true;
    }
}

export class LinearizationContext {
    private readonly last_complex_pictures_: LinkedList<ComplexPicture>;

    constructor() {
        this.last_complex_pictures_ = new LinkedList();
    }

    public findComplexPictureCache(painters: Array<PainterRenderObject>): CkPicture {
        for (const entry of this.last_complex_pictures_) {
            if (entry.equalToPainterSequence(painters)) {
                entry.markUsedLifeCycle();
                return entry.picture;
            }
        }
        return null;
    }

    public createPictureCache(picture: CkPicture, painters: Array<PainterRenderObject>): void {
        this.last_complex_pictures_.push(
            new ComplexPicture(picture, painters));
    }

    public performCacheLifeCycleCheckpoint(): void {
        this.last_complex_pictures_.removeIf((entry) => {
            // Clear this flag for next frame
            if (!entry.usedThisFrame) {
                entry.markUnusedLifeCycle();
            }
            entry.usedThisFrame = false;

            return entry.isExpired();
        });
    }
}

export interface LinearizationLogger {
    beginInstruction(n: number, opcodeName: string, operandsNumber: number): void;
    endInstruction(): void;

    addAnnotation(content: string): void;
}

export class NullLinearizationLogger implements LinearizationLogger {
    beginInstruction(n: number, opcodeName: string, operandsNumber: number): void {}
    endInstruction(): void {}
    addAnnotation(content: string): void {}
}

export class RenderTreeRecorder {
    private readonly insts_: Array<IRInst>;

    constructor() {
        this.insts_ = [];
    }

    private generateDrawPainterSeq(i: number,
                                   picture_count: number,
                                   ctx: LinearizationContext,
                                   builder: SceneBuilder,
                                   logger: LinearizationLogger): number
    {
        let j = 0;

        // Pass 1: compute the smallest rect where all the painters' bounds
        //         are contained, and also find the longest subsequence of painters.
        let union_bounds = this.insts_[i].operands[0].painter.bounds;
        for (j = i + 1; j < this.insts_.length; j++) {
            if (this.insts_[j].opcode != IROpcode.DRAW_PAINTER) {
                break;
            }

            union_bounds = union_bounds.union(
                this.insts_[j].operands[0].painter.bounds);
        }

        const painters_slice = this.insts_.slice(i, j).map((v) => v.operands[0].painter);

        // Try to find an existing cache
        const cached_pict = ctx.findComplexPictureCache(painters_slice);
        if (cached_pict != null) {
            for (let k = i; k < j; k++) {
                logger.beginInstruction(k, IROpcode[IROpcode.DRAW_PAINTER], this.insts_[k].operands.length);
                logger.addAnnotation(`Picture#${picture_count}`);
                logger.addAnnotation('CacheHit');
                logger.addAnnotation(`id=${this.insts_[k].operands[0].painter.uniqueNodeId}`);
                logger.addAnnotation(`dirty=${this.insts_[k].operands[0].painter.isDirty}`);
                const bounds = this.insts_[k].operands[0].painter.bounds;
                logger.addAnnotation(`bounds=(${bounds.x},${bounds.y},${bounds.width},${bounds.height})`);
                logger.endInstruction();
            }

            builder.addPicture(cached_pict, true, union_bounds.x, union_bounds.y);
            return j;
        }

        // Pass 2: generate Picture
        const recorder = new CkPictureRecorder();
        const canvas = recorder.beginRecording(union_bounds.makeWH().toGLType());

        // We only need to iterate in the longest subsequence that we have found.
        for (let k = i; k < j; k++) {
            logger.beginInstruction(k, IROpcode[IROpcode.DRAW_PAINTER], this.insts_[k].operands.length);
            logger.addAnnotation(`Picture#${picture_count}`);
            logger.addAnnotation(`id=${this.insts_[k].operands[0].painter.uniqueNodeId}`);
            logger.addAnnotation(`dirty=${this.insts_[k].operands[0].painter.isDirty}`);
            const bounds = this.insts_[k].operands[0].painter.bounds;
            logger.addAnnotation(`bounds=(${bounds.x},${bounds.y},${bounds.width},${bounds.height})`);
            logger.endInstruction();

            const painter = this.insts_[k].operands[0].painter;
            const saved = canvas.save();

            // As Painters in the subsequence, which expect themselves to be drawn
            // on separated Pictures, share the same Picture, we should make a transformation
            // that translates the origin to make sure that painters draw elements at proper positions.
            canvas.translate(painter.bounds.x - union_bounds.x,
                             painter.bounds.y - union_bounds.y);

            painter.paint(canvas);

            canvas.restoreToCount(saved);
        }

        const picture = recorder.finishRecordingAsPicture();
        builder.addPicture(picture, true, union_bounds.x, union_bounds.y);

        // Finally, we add the picture to the cache
        ctx.createPictureCache(picture, painters_slice);

        return j;
    }

    public generate(viewport_size: Vector2f, ctx: LinearizationContext,
                    logger?: LinearizationLogger): Scene
    {
        if (logger == null) {
            logger = new NullLinearizationLogger();
        }

        const builder = new SceneBuilder(viewport_size.x, viewport_size.y)
            .pushOffset(0, 0);

        let i = 0;
        let picture_count = 0;
        while (i < this.insts_.length) {
            if (this.insts_[i].opcode == IROpcode.DRAW_PAINTER) {
                i = this.generateDrawPainterSeq(i, ++picture_count, ctx, builder, logger);
                continue;
            }

            const inst = this.insts_[i];
            logger.beginInstruction(i, IROpcode[inst.opcode], inst.operands.length);
            switch (inst.opcode) {

            case IROpcode.POP:
                builder.pop();
                break;

            case IROpcode.PUSH_OFFSET:
                builder.pushOffset(inst.operands[0].vector.x, inst.operands[0].vector.y);
                break;
            
            case IROpcode.PUSH_ROTATE:
                builder.pushRotate(inst.operands[0].scalar,
                                   inst.operands[1].vector.x, inst.operands[1].vector.y);
                break;
            
            case IROpcode.PUSH_BACKDROP_FILTER:
                builder.pushBackdropFilter(inst.operands[0].filter,
                                           inst.operands[1].blend_mode,
                                           inst.operands[2].bool);
                break;
            
            case IROpcode.PUSH_IMAGE_FILTER:
                builder.pushImageFilter(inst.operands[0].filter);
                break;

            case IROpcode.PUSH_RECT_CLIP:
                builder.pushRectClip(inst.operands[0].rect, inst.operands[1].bool);
                break;

            case IROpcode.PUSH_RRECT_CLIP:
                builder.pushRRectClip(inst.operands[0].rrect, inst.operands[1].bool);
                break;

            case IROpcode.PUSH_OPACITY:
                builder.pushOpacity(inst.operands[0].scalar);
                break;

            case IROpcode.ISOLATE_LAYER:
                break;

            case IROpcode.DRAW_VIDEO_BUFFER:
                builder.addVideoBuffer(inst.operands[0].video_buffer,
                                       /* offsetX  */ inst.operands[1].rect[0],
                                       /* offsetY  */ inst.operands[1].rect[1],
                                       /* width    */ inst.operands[1].rect[2],
                                       /* height   */ inst.operands[1].rect[3],
                                       /* sampling */ inst.operands[2].int);
                                       
                inst.operands[0].video_buffer.dispose();
                break;
            
            case IROpcode.DRAW_TEXTURE:
                throw Error('Not implemented yet!');
            }

            logger.endInstruction();

            i++;
        }

        ctx.performCacheLifeCycleCheckpoint();

        return builder.build();
    }

    public insertPop(): void {
        this.insts_.push({ opcode: IROpcode.POP, operands: [] });
    }

    public insertPushOffset(vector: Vector2f): void {
        this.insts_.push({
            opcode: IROpcode.PUSH_OFFSET,
            operands: [ { vector: vector } ]
        });
    }

    public insertPushRotate(rad: number, pivot: Vector2f): void {
        this.insts_.push({
            opcode: IROpcode.PUSH_ROTATE,
            operands: [ { scalar: rad }, { vector: pivot } ]
        });
    }

    public insertPushOpacity(alpha: number): void {
        this.insts_.push({
            opcode: IROpcode.PUSH_OPACITY,
            operands: [ { scalar: alpha } ]
        });
    }

    public insertPushBackdropFilter(filter: CkImageFilter, blend_mode: BlendMode, auto_child_clipping: boolean) {
        this.insts_.push({
            opcode: IROpcode.PUSH_BACKDROP_FILTER,
            operands: [
                { filter: filter }, { blend_mode: blend_mode }, { bool: auto_child_clipping }
            ]
        });
    }

    public insertPushImageFilter(filter: CkImageFilter): void {
        this.insts_.push({
            opcode: IROpcode.PUSH_IMAGE_FILTER,
            operands: [ { filter: filter } ]
        });
    }

    public insertPushRectClip(clip: CkRect, AA: boolean): void {
        this.insts_.push({
            opcode: IROpcode.PUSH_RECT_CLIP,
            operands: [ { rect: clip }, { bool: AA } ]
        });
    }

    public insertPushRRectClip(clip: CkRRect, AA: boolean): void {
        this.insts_.push({
            opcode: IROpcode.PUSH_RRECT_CLIP,
            operands: [ { rrect: clip }, { bool: AA } ]
        });
    }

    public insertDrawPainter(painter: PainterRenderObject): void {
        this.insts_.push({
            opcode: IROpcode.DRAW_PAINTER,
            operands: [ { painter: painter } ]
        });
    }

    public insertDrawVideoBuffer(buffer: VideoBuffer, rect: Rect, sampling: SamplingOption): void {
        this.insts_.push({
            opcode: IROpcode.DRAW_VIDEO_BUFFER,
            operands: [ { video_buffer: buffer }, { rect: rect.toGLType() }, { int: sampling } ]
        });
    }

    public insertIsolateLayer(): void {
        this.insts_.push({
            opcode: IROpcode.ISOLATE_LAYER,
            operands: []
        });
    }
}
