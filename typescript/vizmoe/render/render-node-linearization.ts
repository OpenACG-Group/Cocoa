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

import { LinkedList } from '../../core/linked_list';
import {
    CkPicture,
    CkImageFilter,
    CkRect,
    CkRRect,
    CkMatrix,
    CkPictureRecorder,
    BlendMode,
    SceneBuilder,
    Scene,
    SamplingOption,
    Constants as GConst,
    CkArrayXYWHRect,
    CkPath,
    ClipOp
} from 'glamor';
import { VideoBuffer } from 'utau';
import { PaintRenderNode } from './render-node';
import { Vector2f } from './vector';
import { Rect, RRect } from './rectangle';

export interface Logger {
    beginInstruction(n: number, opcodeName: string, operandsNumber: number): void;
    endInstruction(): void;

    addAnnotation(content: string): void;
}

export class NullLogger implements Logger {
    beginInstruction(n: number, opcodeName: string, operandsNumber: number): void {}
    endInstruction(): void {}
    addAnnotation(content: string): void {}
}

enum IROpcode {
    PushOffset,
    PushRotate,
    PushBackdropFilter,
    PushImageFilter,
    PushRectClip,
    PushRRectClip,
    PushPathClip,
    PushOpacity,
    Pop,

    IsolateLayer,
    
    DrawPaintNode,
    DrawStaticTexture,
    DrawVideoTexture
}

interface RecordOperand {
    fPaintNode?: PaintRenderNode;
    fVideoBuffer?: VideoBuffer;
    fFilter?: CkImageFilter;
    fBlendMode?: BlendMode;
    fRect?: CkArrayXYWHRect;
    fRRect?: CkRRect;
    fPath?: CkPath;
    fMatrix?: CkMatrix;
    fVector?: Vector2f;
    fBool?: boolean;
    fScalar?: number;
    fInt?: number;
}

interface RecordInst {
    fOpcode: IROpcode;
    fOperands: Array<RecordOperand>;
}

const PICT_UNUSED_LIFECYCLE_THRESHOLD = 8;

/**
 * Considering the following snippet of record instructions:
 *   <some instructions...>
 *   <1> DrawPicture
 *   <2> DrawPicture
 *   <3> PushImageFilter
 *   <4> DrawPicture
 *   <some instructions...>
 *
 * There are 3 `DrawPicture` instructions in total, with 2 of them are contiguous
 * and another one is separated by a `PushImageFilter` instruction.
 *
 * We note that contiguous `DrawPicture` instructions (like what is showed above)
 * are very common, as each paint node generates one (because a paint node always
 * supposes that its contents are drawn on a standalone layer, not sharing layers
 * with other paint nodes). However, we can find that those contiguous instructions
 * are unnecessary and can be simplified to a single `DrawPicture` instruction by
 * sharing a single Picture.
 *
 * This optimization is called 'Redundant Picture Elimination', and implemented
 * by this class.
 */
class MergedPicture {
    private readonly fPicture: CkPicture;
    private readonly fOrderedPaintNodeIds: Array<number>;
    private fUnusedLifecycle: number;

    public usedThisFrame: boolean;

    constructor(picture: CkPicture, painters: Array<PaintRenderNode>) {
        this.fPicture = picture;
        this.fOrderedPaintNodeIds = [];
        for (const painter of painters) {
            this.fOrderedPaintNodeIds.push(painter.uniqueNodeId);
        }
        this.fUnusedLifecycle = 0;
        this.usedThisFrame = false;
    }

    public get picture(): CkPicture {
        return this.fPicture;
    }

    public markUnusedLifeCycle(): void {
        this.fUnusedLifecycle++;
    }

    public markUsedLifeCycle(): void {
        this.fUnusedLifecycle = 0;
        this.usedThisFrame = true;
    }

    public isExpired(): boolean {
        return (this.fUnusedLifecycle >= PICT_UNUSED_LIFECYCLE_THRESHOLD);
    }

    public compareOrderedPaintNodes(seq: Array<PaintRenderNode>): boolean {
        if (seq.length != this.fOrderedPaintNodeIds.length) {
            return false;
        }
        for (let i = 0; i < seq.length; i++) {
            // The Paint node itself has changed or it has been repainted (dirty node).
            if (seq[i].uniqueNodeId != this.fOrderedPaintNodeIds[i] || seq[i].isDirty()) {
                return false;
            }
        }
        return true;
    }
}

export class CacheContext {
    private readonly fMergedPicturesCache: LinkedList<MergedPicture>;

    constructor() {
        this.fMergedPicturesCache = new LinkedList();
    }

    public findMergedPictureCache(paintNodes: Array<PaintRenderNode>): CkPicture {
        for (const entry of this.fMergedPicturesCache) {
            if (entry.compareOrderedPaintNodes(paintNodes)) {
                entry.markUsedLifeCycle();
                return entry.picture;
            }
        }
        return null;
    }

    public createMergedPictureCache(picture: CkPicture, paintNodes: Array<PaintRenderNode>): void {
        this.fMergedPicturesCache.push(new MergedPicture(picture, paintNodes));
    }

    public performCacheLifeCycleCheckpoint(): void {
        this.fMergedPicturesCache.removeIf((entry) => {
            // Clear this flag for next frame
            if (!entry.usedThisFrame) {
                entry.markUnusedLifeCycle();
            }
            entry.usedThisFrame = false;

            return entry.isExpired();
        });
    }
}

const BLEND_MODE_NAMES = new Map<number, string>([
    [ GConst.BLEND_MODE_CLEAR, 'Clear' ],
    [ GConst.BLEND_MODE_SRC, 'Src' ],
    [ GConst.BLEND_MODE_DST, 'Dst' ],
    [ GConst.BLEND_MODE_SRC_OVER, 'SrcOver' ],
    [ GConst.BLEND_MODE_DST_OVER, 'DstOver' ],
    [ GConst.BLEND_MODE_SRC_IN, 'SrcIn' ],
    [ GConst.BLEND_MODE_DST_IN, 'DstIn' ],
    [ GConst.BLEND_MODE_SRC_OUT, 'SrcOut' ],
    [ GConst.BLEND_MODE_DST_OUT, 'DstOut' ],
    [ GConst.BLEND_MODE_SRC_ATOP, 'SrcAtop' ],
    [ GConst.BLEND_MODE_DST_ATOP, 'DstAtop' ],
    [ GConst.BLEND_MODE_XOR, 'Xor' ],
    [ GConst.BLEND_MODE_PLUS, 'Plus' ],
    [ GConst.BLEND_MODE_MODULATE, 'Modulate' ],
    [ GConst.BLEND_MODE_SCREEN, 'Screen' ],
    [ GConst.BLEND_MODE_OVERLAY, 'Overlay' ],
    [ GConst.BLEND_MODE_DARKEN, 'Darken' ],
    [ GConst.BLEND_MODE_LIGHTEN, 'Lighten' ],
    [ GConst.BLEND_MODE_COLOR_DODGE, 'ColorDodge' ],
    [ GConst.BLEND_MODE_COLOR_BURN, 'ColorBurn' ],
    [ GConst.BLEND_MODE_HARD_LIGHT, 'HardLight' ],
    [ GConst.BLEND_MODE_SOFT_LIGHT, 'SoftLight' ],
    [ GConst.BLEND_MODE_DIFFERENCE, 'Difference' ],
    [ GConst.BLEND_MODE_EXCLUSION, 'Exclusion' ],
    [ GConst.BLEND_MODE_HUE, 'Hue' ],
    [ GConst.BLEND_MODE_SATURATION, 'Saturation' ],
    [ GConst.BLEND_MODE_COLOR, 'Color' ],
    [ GConst.BLEND_MODE_LUMINOSITY, 'Luminosity' ]
]);

const SAMPLING_NAMES = new Map<number, string>([
    [ GConst.SAMPLING_FILTER_NEAREST, 'Nearest' ],
    [ GConst.SAMPLING_FILTER_LINEAR, 'Linear' ],
    [ GConst.SAMPLING_CUBIC_MITCHELL, 'CubicMitchell' ],
    [ GConst.SAMPLING_CUBIC_CATMULL_ROM, 'CubicCatmullRom' ]
]);

function stringifyCkRect(rect: CkArrayXYWHRect): string {
    return `(${rect[0]},${rect[1]},${rect[2]},${rect[3]})`;
}

export class Recorder {
    private readonly fInsts: Array<RecordInst>;

    constructor() {
        this.fInsts = [];
    }

    private drawOrderedPaint(i: number, picture_count: number, ctx: CacheContext,
                             builder: SceneBuilder, logger: Logger): number
    {
        let j = 0;

        // Pass 1: compute the smallest rect where all the painters' bounds
        //         are contained, and also find the longest subsequence of painters.
        let union_bounds = this.fInsts[i].fOperands[0].fPaintNode.getBounds();
        for (j = i + 1; j < this.fInsts.length; j++) {
            if (this.fInsts[j].fOpcode != IROpcode.DrawPaintNode) {
                break;
            }

            union_bounds = union_bounds.union(
                this.fInsts[j].fOperands[0].fPaintNode.getBounds());
        }

        const paintNodesSlice = this.fInsts.slice(i, j).map((v) => v.fOperands[0].fPaintNode);

        const writeInstructionLog = (n: number, inst: RecordInst, cacheHit: boolean) => {
            logger?.beginInstruction(n, IROpcode[IROpcode.DrawPaintNode], inst.fOperands.length);
            logger?.addAnnotation(`Picture#${picture_count}`);
            if (cacheHit) {
                logger?.addAnnotation('CacheHit');
            }
            logger?.addAnnotation(`id=${inst.fOperands[0].fPaintNode.uniqueNodeId}`);
            logger?.addAnnotation(`dirty=${inst.fOperands[0].fPaintNode.isDirty()}`);
            const bounds = inst.fOperands[0].fPaintNode.getBounds();
            logger?.addAnnotation(`bounds=(${bounds.x},${bounds.y},${bounds.width},${bounds.height})`);
            logger?.endInstruction();
        };

        // Try to find an existing cache
        const cachedPicture = ctx.findMergedPictureCache(paintNodesSlice);
        if (cachedPicture != null) {
            for (let k = i; k < j; k++) {
                writeInstructionLog(k, this.fInsts[k], true);
            }

            builder.addPicture(cachedPicture, true, union_bounds.x, union_bounds.y);
            return j;
        }

        // Pass 2: generate Picture
        const recorder = new CkPictureRecorder();
        const canvas = recorder.beginRecording(union_bounds.makeWH().toGLType());

        // We only need to iterate in the longest subsequence that we have found.
        for (let k = i; k < j; k++) {
            writeInstructionLog(k, this.fInsts[k], false);

            const painter = this.fInsts[k].fOperands[0].fPaintNode;
            const saved = canvas.save();

            // As Painters in the subsequence, which expect themselves to be drawn
            // on separated Pictures, share the same Picture, we should make a transformation
            // that translates the origin to make sure that painters draw elements at proper positions.
            canvas.translate(painter.getBounds().x - union_bounds.x,
                             painter.getBounds().y - union_bounds.y);

            painter.paint(canvas);

            canvas.restoreToCount(saved);
        }

        const picture = recorder.finishRecordingAsPicture();
        builder.addPicture(picture, true, union_bounds.x, union_bounds.y);

        // Finally, we add the picture to the cache
        ctx.createMergedPictureCache(picture, paintNodesSlice);

        return j;
    }

    public finish(viewportSize: Vector2f, ctx: CacheContext, logger?: Logger): Scene {
        /*
        if (logger == null) {
            logger = new NullLogger();
        }
         */

        const builder = new SceneBuilder(viewportSize.x, viewportSize.y)
            .pushOffset(0, 0);

        let i = 0;
        let pictureCount = 0;
        let pushCount = 0;
        while (i < this.fInsts.length) {
            if (this.fInsts[i].fOpcode == IROpcode.DrawPaintNode) {
                i = this.drawOrderedPaint(i, ++pictureCount, ctx, builder, logger);
                continue;
            }

            const inst = this.fInsts[i];
            logger?.beginInstruction(i, IROpcode[inst.fOpcode], inst.fOperands.length);
            switch (inst.fOpcode) {

            case IROpcode.Pop:
                logger?.addAnnotation(`&${pushCount}`);
                builder.pop();
                pushCount--;
                break;

            case IROpcode.PushOffset:
                pushCount++;
                logger?.addAnnotation(`&${pushCount}`);
                logger?.addAnnotation(`offset=${inst.fOperands[0].fVector.toString()}`);
                builder.pushOffset(inst.fOperands[0].fVector.x, inst.fOperands[0].fVector.y);
                break;
            
            case IROpcode.PushRotate:
                pushCount++;
                logger?.addAnnotation(`&${pushCount}`);
                logger?.addAnnotation(`rad=${inst.fOperands[0].fScalar}`);
                logger?.addAnnotation(`pivot=${inst.fOperands[1].fVector.toString()}`);
                builder.pushRotate(inst.fOperands[0].fScalar,
                                   inst.fOperands[1].fVector.x, inst.fOperands[1].fVector.y);
                break;
            
            case IROpcode.PushBackdropFilter:
                pushCount++;
                logger?.addAnnotation(`&${pushCount}`);
                logger?.addAnnotation(`mode=${BLEND_MODE_NAMES.get(inst.fOperands[1].fBlendMode)}`);
                logger?.addAnnotation(`clipping=${inst.fOperands[2].fBool}`);
                builder.pushBackdropFilter(inst.fOperands[0].fFilter,
                                           inst.fOperands[1].fBlendMode,
                                           inst.fOperands[2].fBool);
                break;
            
            case IROpcode.PushImageFilter:
                pushCount++;
                logger?.addAnnotation(`&${pushCount}`);
                builder.pushImageFilter(inst.fOperands[0].fFilter);
                break;

            case IROpcode.PushRectClip:
                pushCount++;
                logger?.addAnnotation(`&${pushCount}`);
                logger?.addAnnotation(`rect=${stringifyCkRect(inst.fOperands[0].fRect)}`);
                logger?.addAnnotation(`AA=${inst.fOperands[1].fBool}`);
                builder.pushRectClip(inst.fOperands[0].fRect, inst.fOperands[1].fBool);
                break;

            case IROpcode.PushRRectClip:
                pushCount++;
                logger?.addAnnotation(`&${pushCount}`);
                logger?.addAnnotation(`AA=${inst.fOperands[1].fBool}`);
                builder.pushRRectClip(inst.fOperands[0].fRRect, inst.fOperands[1].fBool);
                break;

            case IROpcode.PushPathClip:
                pushCount++;
                const opname = inst.fOperands[1].fInt == GConst.CLIP_OP_INTERSECT
                             ? 'intersect' : 'difference';
                logger?.addAnnotation(`&${pushCount}`);
                logger?.addAnnotation(`ClipOp=${opname}`);
                logger?.addAnnotation(`AA=${inst.fOperands[2].fBool}`);
                builder.pushPathClip(inst.fOperands[0].fPath,
                                     inst.fOperands[1].fInt,
                                     inst.fOperands[2].fBool);
                break;

            case IROpcode.PushOpacity:
                pushCount++;
                logger?.addAnnotation(`&${pushCount}`);
                logger?.addAnnotation(`opacity=${inst.fOperands[0].fScalar}`);
                builder.pushOpacity(inst.fOperands[0].fScalar);
                break;

            case IROpcode.IsolateLayer:
                break;

            case IROpcode.DrawVideoTexture:
                logger?.addAnnotation(`offsetX=${inst.fOperands[1].fRect[0]}`);
                logger?.addAnnotation(`offsetY=${inst.fOperands[1].fRect[1]}`);
                logger?.addAnnotation(`width=${inst.fOperands[1].fRect[2]}`);
                logger?.addAnnotation(`height=${inst.fOperands[1].fRect[3]}`);
                logger?.addAnnotation(`sampling=${SAMPLING_NAMES.get(inst.fOperands[2].fInt)}`);
                builder.addVideoBuffer(inst.fOperands[0].fVideoBuffer,
                                       /* offsetX  */ inst.fOperands[1].fRect[0],
                                       /* offsetY  */ inst.fOperands[1].fRect[1],
                                       /* width    */ inst.fOperands[1].fRect[2],
                                       /* height   */ inst.fOperands[1].fRect[3],
                                       /* sampling */ inst.fOperands[2].fInt);
                                       
                inst.fOperands[0].fVideoBuffer.dispose();
                break;
            
            case IROpcode.DrawStaticTexture:
                throw Error('Not implemented yet!');
            }

            logger?.endInstruction();

            i++;
        }

        ctx.performCacheLifeCycleCheckpoint();

        builder.pop();
        return builder.build();
    }

    public pop(): void {
        this.fInsts.push({ fOpcode: IROpcode.Pop, fOperands: [] });
    }

    public pushOffset(vector: Vector2f): void {
        this.fInsts.push({
            fOpcode: IROpcode.PushOffset,
            fOperands: [ { fVector: vector } ]
        });
    }

    public pushRotate(rad: number, pivot: Vector2f): void {
        this.fInsts.push({
            fOpcode: IROpcode.PushRotate,
            fOperands: [ { fScalar: rad }, { fVector: pivot } ]
        });
    }

    public pushOpacity(alpha: number): void {
        this.fInsts.push({
            fOpcode: IROpcode.PushOpacity,
            fOperands: [ { fScalar: alpha } ]
        });
    }

    public pushBackdropFilter(filter: CkImageFilter, blendMode: BlendMode, autoChildClipping: boolean) {
        this.fInsts.push({
            fOpcode: IROpcode.PushBackdropFilter,
            fOperands: [
                { fFilter: filter }, { fBlendMode: blendMode }, { fBool: autoChildClipping }
            ]
        });
    }

    public pushImageFilter(filter: CkImageFilter): void {
        this.fInsts.push({
            fOpcode: IROpcode.PushImageFilter,
            fOperands: [ { fFilter: filter } ]
        });
    }

    public pushRectClip(clip: Rect, AA: boolean): void {
        this.fInsts.push({
            fOpcode: IROpcode.PushRectClip,
            fOperands: [ { fRect: clip.toGLType() }, { fBool: AA } ]
        });
    }

    public pushRRectClip(clip: RRect, AA: boolean): void {
        this.fInsts.push({
            fOpcode: IROpcode.PushRRectClip,
            fOperands: [ { fRRect: clip.toGLType() }, { fBool: AA } ]
        });
    }

    public pushPathClip(clip: CkPath, op: ClipOp, AA: boolean): void {
        this.fInsts.push({
            fOpcode: IROpcode.PushPathClip,
            fOperands: [ { fPath: clip.clone() }, { fInt: op }, { fBool: AA } ]
        });
    }

    public drawPaintNode(node: PaintRenderNode): void {
        this.fInsts.push({
            fOpcode: IROpcode.DrawPaintNode,
            fOperands: [ { fPaintNode: node } ]
        });
    }

    public drawVideoTexture(buffer: VideoBuffer, rect: Rect, sampling: SamplingOption): void {
        this.fInsts.push({
            fOpcode: IROpcode.DrawVideoTexture,
            fOperands: [ { fVideoBuffer: buffer.clone() }, { fRect: rect.toGLType() }, { fInt: sampling } ]
        });
    }

    public isolateLayer(): void {
        this.fInsts.push({
            fOpcode: IROpcode.IsolateLayer,
            fOperands: []
        });
    }

    // TODO(sora): implement `drawStaticTexture`
}
