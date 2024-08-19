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

import * as Fmt from '../../core/formatter';
import { GskSampling} from './GskSampling';
import { Color4f} from '../base/Color';
import { Blender, BlendMode, ClipOp, Image, ImageFilter, Mat3x3, Paint, Path, Rect, RRect, Shader } from 'renderer';
import { GskDLDrawOpInspect, GskDLDrawOpVerb, GskDrawOpArgType } from './GskDLDrawOpInfo';
import { Optimizers, Translator } from './GskDLProcessors';

// Info that describes how a layer is composited with its underlying layer
export interface DLLayerInfo {
    bounds: Rect;
    initWithPreviousLayerContent: boolean;
    alpha: number;
    blendMode: BlendMode;
    blender: Blender | null;
    filter: ImageFilter;
    backdrop: ImageFilter | null;
}

function CompareDLLayerInfo(a: DLLayerInfo, b: DLLayerInfo): boolean {
    return a.bounds.equalTo(b.bounds) &&
           a.initWithPreviousLayerContent == b.initWithPreviousLayerContent &&
           a.alpha == b.alpha &&
           a.blendMode == b.blendMode &&
           a.blender == b.blender &&
           a.filter == b.filter &&
           a.backdrop == b.backdrop;
}

function CompareRRect(a: RRect, b: RRect): boolean {
    if (!a.rect.equalTo(b.rect) || a.uniformRadii != b.uniformRadii) {
        return false;
    }
    return a.borderRadii.every((value, index) => {
        return b.uniformRadii[index] == value;
    });
}

function FormatDLLayerInfo(info: DLLayerInfo, ctx: Fmt.FormatterContext): Array<Fmt.TextBlock> {
    const tbs = [
        Fmt.TB(Fmt.TextBlockLayoutHint.kCompoundStructureBegin, [Fmt.TAG('{')])
    ];

    if (info.initWithPreviousLayerContent) {
        tbs.push(
            Fmt.TB(Fmt.TextBlockLayoutHint.kValue, [Fmt.TAG('init-previous-content', Fmt.TextColor.kBlue)]),
            Fmt.TB(Fmt.TextBlockLayoutHint.kSeparator, [Fmt.TAG(',')])
        );
    }
    tbs.push(Fmt.TB(Fmt.TextBlockLayoutHint.kValue, [Fmt.TAG(`alpha=${info.alpha}`)]));
    tbs.push(Fmt.TB(Fmt.TextBlockLayoutHint.kSeparator, [Fmt.TAG(',')]));

    if (info.blender == null) {
        const blendModeName = GskDLDrawOpInspect.StringifyBlendMode(info.blendMode);
        tbs.push(Fmt.TB(Fmt.TextBlockLayoutHint.kValue, [Fmt.TAG(`blend-mode=${blendModeName}`)]));
    } else {
        tbs.push(Fmt.TB(Fmt.TextBlockLayoutHint.kValue, [Fmt.TAG('use-blender')]));
    }

    if (info.filter != null) {
        tbs.push(Fmt.TB(Fmt.TextBlockLayoutHint.kSeparator, [Fmt.TAG(',')]),
                 Fmt.TB(Fmt.TextBlockLayoutHint.kValue, [Fmt.TAG('filters')]));
    }

    if (info.backdrop != null) {
        tbs.push(Fmt.TB(Fmt.TextBlockLayoutHint.kSeparator, [Fmt.TAG(',')]),
                 Fmt.TB(Fmt.TextBlockLayoutHint.kValue, [Fmt.TAG('backdrop')]));
    }

    tbs.push(Fmt.TB(Fmt.TextBlockLayoutHint.kCompoundStructureEnd, [Fmt.TAG('}')]));
    return tbs;
}

export enum DLClippingType {
    kRect,
    kRRect,
    kPath,
    kShader
}

export class DLSliceClipping {
    public static Rect(rect: Rect, op: ClipOp, antiAlias: boolean): DLSliceClipping {
        return new DLSliceClipping(DLClippingType.kRect, op, antiAlias, rect, null, null, null);
    }

    public static RRect(rrect: RRect, op: ClipOp, antiAlias: boolean): DLSliceClipping {
        return new DLSliceClipping(DLClippingType.kRRect, op, antiAlias, null, rrect, null, null);
    }

    public static Path(path: Path, op: ClipOp, antiAlias: boolean): DLSliceClipping {
        return new DLSliceClipping(DLClippingType.kPath, op, antiAlias, null, null, path, null);
    }

    public static Shader(shader: Shader, op: ClipOp): DLSliceClipping {
        return new DLSliceClipping(DLClippingType.kShader, op, false, null, null, null, shader);
    }

    private constructor(public readonly fType: DLClippingType,
                        public readonly fOp: ClipOp,
                        public readonly fAntiAlias: boolean,
                        public readonly fRectClip: Rect | null,
                        public readonly fRRectClip: RRect | null,
                        public readonly fPathClip: Path | null,
                        public readonly fShaderClip: Shader | null) {}

    public static IsEqual(c1: DLSliceClipping, c2: DLSliceClipping): boolean {
        if (c1.fType != c2.fType || c1.fAntiAlias != c2.fAntiAlias || c1.fOp != c2.fOp) {
            return false;
        }
        if (c1.fType == DLClippingType.kRect) {
            return c1.fRectClip.equalTo(c2.fRectClip);
        }
        if (c1.fType == DLClippingType.kRRect) {
            return CompareRRect(c1.fRRectClip, c2.fRRectClip);
        }
        if (c1.fType == DLClippingType.kShader) {
            return c1.fShaderClip == c2.fShaderClip;
        }
        if (c1.fType == DLClippingType.kPath) {
            return c1.fPathClip.equalTo(c2.fPathClip);
        }
        throw Error('unexpected clipping type');
    }

    public [Fmt.kObjectFormatter](ctx: Fmt.FormatterContext): Array<Fmt.TextBlock> {
        const opName = this.fOp == ClipOp.Intersect ? 'Intersect' : 'Difference';
        return [
            Fmt.TB(Fmt.TextBlockLayoutHint.kPrefix, [Fmt.TAG('DLSliceClipping')]),
            Fmt.TB(Fmt.TextBlockLayoutHint.kCompoundStructureBegin, [Fmt.TAG('(')]),

            Fmt.TB(Fmt.TextBlockLayoutHint.kValue, [Fmt.TAG(DLClippingType[this.fType])]),
            Fmt.TB(Fmt.TextBlockLayoutHint.kSeparator, [Fmt.TAG(',')]),

            Fmt.TB(Fmt.TextBlockLayoutHint.kPropertyName, [Fmt.TAG('op=')]),
            Fmt.TB(Fmt.TextBlockLayoutHint.kValue, [Fmt.TAG(opName)]),
            Fmt.TB(Fmt.TextBlockLayoutHint.kSeparator, [Fmt.TAG(',')]),

            Fmt.TB(Fmt.TextBlockLayoutHint.kPropertyName, [Fmt.TAG('AA=')]),
            ...Fmt.formatAnyValue(this.fAntiAlias, ctx),

            Fmt.TB(Fmt.TextBlockLayoutHint.kCompoundStructureEnd, [Fmt.TAG(')')])
        ];
    }
}

export type SliceId = number;
export type SliceComparisonCache = boolean[][];

let sliceIdCounter = 0;

export class DLDrawOpSlice {
    public readonly fId: SliceId;
    public fBounds: Rect;

    // Child slice array. DrawSlice and DrawSliceLayer operation will find slices
    // by indices of the array.
    public fChildren: Array<DLDrawOpSlice>;

    // Local CTM, relative to the parent node
    public fMatrix: Mat3x3 | null;
    public fClipping: Array<DLSliceClipping> | null;

    // Stores the draw operations (verb and arguments) in order
    public fOps: Array<unknown>;
    public fCanvas: DLSliceCanvas;

    // Internal information for translation
    private fTrctxInfo: Translator.SliceTrctxInfo | null;

    constructor(bounds: Rect, matrix: Mat3x3 | null = null,
                clipping: Array<DLSliceClipping> | null = null) {
        this.fId = ++sliceIdCounter;
        this.fBounds = bounds;
        this.fChildren = [];
        this.fMatrix = matrix;
        this.fClipping = clipping;
        this.fOps = [];
        this.fCanvas = null;
        this.fTrctxInfo = null;
    }

    public _internal_setTrctxInfo(info: Translator.SliceTrctxInfo): void {
        this.fTrctxInfo = info;
    }

    public _internal_getTrctxInfo(): Translator.SliceTrctxInfo {
        return this.fTrctxInfo;
    }

    public insertSubSlice(child: DLDrawOpSlice): void {
        this.fChildren.push(child);
        this.fOps.push(GskDLDrawOpVerb.kDrawSlice, this.fChildren.length - 1);
    }

    public insertSubSliceLayer(child: DLDrawOpSlice, layerInfo: DLLayerInfo): void {
        this.fChildren.push(child);
        this.fOps.push(GskDLDrawOpVerb.kDrawSliceLayer, this.fChildren.length - 1, layerInfo);
    }

    public getCanvas(): DLSliceCanvas {
        if (this.fCanvas == null) {
            this.fCanvas = new DLSliceCanvas(this);
        }
        return this.fCanvas;
    }

    public isEmpty(): boolean {
        return this.fOps.length == 0;
    }

    public concatMatrix(mat: Mat3x3): void {
        if (this.fMatrix == null) {
            this.fMatrix = mat.clone();
            return;
        }
        this.fMatrix.preConcat(mat);
    }

    public appendClipping(clip: DLSliceClipping): void {
        if (this.fClipping == null) {
            this.fClipping = [];
        }
        this.fClipping.push(clip);
    }

    public static Compare(slice1: DLDrawOpSlice, slice2: DLDrawOpSlice, cache?: SliceComparisonCache): boolean {
        if (cache == null) {
            return DLDrawOpSlice.DoCompare(slice1, slice2, null);
        }

        const cacheValue = cache[slice1.fId][slice2.fId];
        if (cacheValue != null) {
            return cacheValue;
        }
        const compareValue = DLDrawOpSlice.DoCompare(slice1, slice2, cache);
        cache[slice1.fId][slice2.fId] = compareValue;
        cache[slice2.fId][slice1.fId] = compareValue;
        return compareValue;
    }

    private static DoCompare(slice1: DLDrawOpSlice, slice2: DLDrawOpSlice, cache: SliceComparisonCache): boolean {
        if (slice1.fId == slice2.fId) {
            return true;
        }

        // Compare bounds, matrix, and clipping first to do a quick rejection.

        if (!slice1.fBounds.equalTo(slice2.fBounds)) {
            return false;
        }
        if ((slice1.fMatrix == null) != (slice2.fMatrix == null)) {
            // nullability of matrix is not equal
            return false;
        }
        if (slice1.fMatrix != null && slice2.fMatrix != null && !slice1.fMatrix.equalTo(slice2.fMatrix)) {
            // matrix is not equal
            return false;
        }
        if ((slice1.fClipping == null) != (slice2.fClipping == null)) {
            // nullability of clipping is not equal
            return false;
        }
        if (slice1.fClipping != null && slice2.fClipping != null) {
            if (slice1.fClipping.length != slice2.fClipping.length) {
                return false;
            }
            for (let i = 0; i < slice1.fClipping.length; i++) {
                if (!DLSliceClipping.IsEqual(slice1.fClipping[i], slice2.fClipping[i])) {
                    return false;
                }
            }
        }

        // Compare child slices
        if (slice1.fChildren.length != slice2.fChildren.length) {
            return false;
        }
        for (let i = 0; i < slice1.fChildren.length; i++) {
            if (!DLDrawOpSlice.Compare(slice1.fChildren[i], slice2.fChildren[i], cache)) {
                return false;
            }
        }

        // Compare DrawOps

        const seq1 = slice1.fOps, seq2 = slice2.fOps;
        if (seq1.length != seq2.length) {
            return false;
        }

        // Deep comparison, compare each DrawOp
        const itr1 = GskDLDrawOpInspect.Iterate(seq1), itr2 = GskDLDrawOpInspect.Iterate(seq2);
        while (true) {
            const next1 = itr1.next(), next2 = itr2.next();
            if (next1.done || next2.done) {
                break;
            }
            const op1 = next1.value as GskDLDrawOpInspect.DrawOp,
                  op2 = next2.value as GskDLDrawOpInspect.DrawOp;

            if (op1.verb != op2.verb) {
                continue;
            }
            for (let i = 0; i < op1.args.length; i++) {
                switch (op1.reflection.args[i].type) {
                    case GskDrawOpArgType.kInternal_Obj_DLLayerInfo:
                        if (!CompareDLLayerInfo(op1.args[i] as DLLayerInfo, op2.args[i] as DLLayerInfo)) {
                            return false;
                        }
                        break;

                    case GskDrawOpArgType.kObj_Color4f:
                        if (!(op1.args[i] as Color4f).equalTo(op2.args[i] as Color4f)) {
                            return false;
                        }
                        break;

                    case GskDrawOpArgType.kObj_Mat3x3:
                        if (!(op1.args[i] as Mat3x3).equalTo(op2.args[i] as Mat3x3)) {
                            return false;
                        }
                        break;

                    case GskDrawOpArgType.kObj_Rect:
                        if (!(op1.args[i] as Rect).equalTo(op2.args[i] as Rect)) {
                            return false;
                        }
                        break;

                    case GskDrawOpArgType.kObj_RRect:
                        if (!CompareRRect(op1.args[i] as RRect, op2.args[i] as RRect)) {
                            return false;
                        }
                        break;

                    case GskDrawOpArgType.kObj_Paint:
                        if (!(op1.args[i] as Paint).equalTo(op2.args[i] as Paint)) {
                            return false;
                        }
                        break;

                    default:
                        // For primitive values and other `kObj_*` values, compare directly
                        if (op1.args[i] != op2.args[i]) {
                            return false;
                        }
                        break;
                }
            }
        }

        return true;
    }

    public [Fmt.kObjectFormatter](ctx: Fmt.FormatterContext): Array<Fmt.TextBlock> {
        const blocks: Array<Fmt.TextBlock> = [
            Fmt.TB(Fmt.TextBlockLayoutHint.kPrefix, [Fmt.TAG('DLDrawOpSlice', Fmt.TextColor.kBlueBright)]),
            Fmt.TB(Fmt.TextBlockLayoutHint.kCompoundStructureBegin, [Fmt.TAG('{')])
        ];

        const arrBounds = [this.fBounds.left, this.fBounds.top,
                           this.fBounds.right, this.fBounds.bottom];
        blocks.push(
            Fmt.TB(Fmt.TextBlockLayoutHint.kPropertyName, [Fmt.TAG('@BoundsLTRB:')]),
            ...Fmt.formatAnyValue(arrBounds, ctx),

            Fmt.TB(Fmt.TextBlockLayoutHint.kSeparator, [Fmt.TAG(';')]),
            Fmt.TB(Fmt.TextBlockLayoutHint.kPropertyName, [Fmt.TAG('@Clipping:')]),
            ...Fmt.formatAnyValue(this.fClipping, ctx),

            Fmt.TB(Fmt.TextBlockLayoutHint.kSeparator, [Fmt.TAG(';')]),
            Fmt.TB(Fmt.TextBlockLayoutHint.kPropertyName, [Fmt.TAG('@Matrix:')]),
        );
        if (this.fMatrix != null) {
            blocks.push(...GskDLDrawOpInspect.FormatMat3x3(this.fMatrix, ctx));
        } else {
            blocks.push(Fmt.TB(Fmt.TextBlockLayoutHint.kValue,
                               [Fmt.TAG('<Identity>', Fmt.TextColor.kGreen)]));
        }

        // Format DrawOps
        for (const op of GskDLDrawOpInspect.Iterate(this.fOps)) {
            blocks.push(
                Fmt.TB(Fmt.TextBlockLayoutHint.kSeparator, [Fmt.TAG(';')]),
                Fmt.TB(Fmt.TextBlockLayoutHint.kPropertyName,
                       [Fmt.TAG(GskDLDrawOpInspect.GetName(op.verb), Fmt.TextColor.kCyanBright)]),
                Fmt.TB(Fmt.TextBlockLayoutHint.kCompoundStructureBegin, [Fmt.TAG('(', Fmt.TextColor.kYellowBright)])
            );

            for (let i = 0; i < op.args.length; i++) {
                const argInfo = op.reflection.args[i];
                const argValue = op.args[i];
                blocks.push(Fmt.TB(Fmt.TextBlockLayoutHint.kPropertyName, [
                    Fmt.TAG(`${argInfo.name}`, Fmt.TextColor.kYellowBright),
                    Fmt.TAG(` = <${argInfo.type}>`)
                ]));

                switch (argInfo.type) {
                    case GskDrawOpArgType.kInternal_I32_ChildIndex:
                        blocks.push(...Fmt.formatAnyValue(this.fChildren[argValue as number], ctx));
                        break;

                    case GskDrawOpArgType.kInternal_Obj_DLLayerInfo:
                        blocks.push(...FormatDLLayerInfo(argValue as DLLayerInfo, ctx));
                        break;

                    case GskDrawOpArgType.kI32:
                    case GskDrawOpArgType.kF32:
                    case GskDrawOpArgType.kBool:
                    case GskDrawOpArgType.kString:
                        blocks.push(...Fmt.formatAnyValue(argValue, ctx));
                        break;

                    // TODO(sora): implement other types.
                }

                if (i < op.args.length - 1) {
                    blocks.push(Fmt.TB(Fmt.TextBlockLayoutHint.kSeparator, [Fmt.TAG(',')]));
                }
            }

            blocks.push(Fmt.TB(Fmt.TextBlockLayoutHint.kCompoundStructureEnd,
                               [Fmt.TAG(')', Fmt.TextColor.kYellowBright)]));
        }

        blocks.push(Fmt.TB(Fmt.TextBlockLayoutHint.kCompoundStructureEnd, [Fmt.TAG('}')]));
        return blocks;
    }
}

interface RecorderState<T> {
    saveCount: number;
    matrix: Mat3x3;
    slice: DLDrawOpSlice;
    extraStates: T;
}

export interface SaveLayerOptions {
    bounds?: Rect;
    initWithPreviousLayerContent?: boolean;
    alpha?: number;
    blendMode?: BlendMode;
    blender?: Blender;
    filter?: ImageFilter;
    backdrop?: ImageFilter;
}

export abstract class GskDLRecorderBase<ExtraStates> {
    private readonly fStateStack: Array<RecorderState<ExtraStates>>;
    private readonly fRootSlice: DLDrawOpSlice;
    private fTotalSliceCount: number;

    protected constructor(bounds: Rect, initialExtraStates: ExtraStates) {
        this.fStateStack = [];
        this.fRootSlice = new DLDrawOpSlice(bounds);
        this.fTotalSliceCount = 1;
        // Current state
        this.fStateStack.push({
            saveCount: 1,
            matrix: Mat3x3.Identity(),
            slice: this.fRootSlice,
            extraStates: initialExtraStates
        });
    }

    protected abstract onCopyExtraStates(from: ExtraStates): ExtraStates;

    private currentState(): RecorderState<ExtraStates> {
        if (this.fStateStack.length == 0) {
            throw Error('State stack is empty');
        }
        return this.fStateStack[this.fStateStack.length - 1];
    }

    public save(): number {
        const top = this.currentState();
        this.fStateStack.push({
            saveCount: top.saveCount + 1,
            matrix: top.matrix,
            // No states are changed, and we should not create a new slice.
            slice: top.slice,
            extraStates: this.onCopyExtraStates(top.extraStates)
        });
        return top.saveCount;
    }

    public saveBounds(bounds: Rect): number {
        const top = this.currentState();

        // Bounds is changed, so a new slice should be created.
        const newSlice = new DLDrawOpSlice(bounds);
        top.slice.insertSubSlice(newSlice);
        this.fTotalSliceCount++;

        this.fStateStack.push({
            saveCount: top.saveCount + 1,
            matrix: top.matrix,
            slice: newSlice,
            extraStates: this.onCopyExtraStates(top.extraStates)
        });
        return top.saveCount;
    }

    public saveLayer(options: SaveLayerOptions): number {
        const top = this.currentState();

        const layerInfo: DLLayerInfo = {
            bounds: options.bounds != null ? options.bounds : top.slice.fBounds,
            initWithPreviousLayerContent:
                options.initWithPreviousLayerContent != null ? options.initWithPreviousLayerContent : false,
            alpha: options.alpha != null ? options.alpha : 1,
            blendMode: options.blendMode != null ? options.blendMode : BlendMode.SrcOver,
            blender: options.blender != null ? options.blender : null,
            filter: options.filter != null ? options.filter : null,
            backdrop: options.backdrop != null ? options.backdrop : null
        };
        const newSlice = new DLDrawOpSlice(layerInfo.bounds);
        top.slice.insertSubSliceLayer(newSlice, layerInfo);
        this.fTotalSliceCount++;

        this.fStateStack.push({
            saveCount: top.saveCount + 1,
            matrix: top.matrix,
            slice: newSlice,
            extraStates: this.onCopyExtraStates(top.extraStates)
        });
        return top.saveCount;
    }

    public getSaveCount(): number {
        return this.fStateStack.length;
    }

    public restore(): void {
        if (this.fStateStack.length == 1) {
            return;
        }
        this.fStateStack.pop();
    }

    public restoreToCount(saveCount: number): void {
        const top = this.currentState();
        if (saveCount >= top.saveCount) {
            return;
        }

        saveCount = Math.max(1, saveCount);
        this.fStateStack.splice(saveCount);
    }

    private getStateMutableSlice(): DLDrawOpSlice {
        const top = this.currentState();
        // The states of an empty slice are mutable, since the change of states
        // does not affect any existing DrawOps.
        if (top.slice.isEmpty()) {
            return top.slice;
        }
        // The states of a non-empty are immutable. The slice has recorded some DrawOps,
        // which are supposed to be in the original states. Create a new empty slice
        // as the mutable slice.
        const newSlice = new DLDrawOpSlice(top.slice.fBounds);
        top.slice.insertSubSlice(newSlice);
        top.slice = newSlice;
        this.fTotalSliceCount++;
        return newSlice;
    }

    public concatMatrix(ctm: Mat3x3): void {
        const top = this.currentState();
        top.matrix = top.matrix.clone();
        top.matrix.preConcat(ctm);
        this.getStateMutableSlice().concatMatrix(ctm);
    }

    public getTotalMatrix(): Mat3x3 {
        const top = this.currentState();
        return top.matrix.clone();
    }

    public clipRect(rect: Rect, op: ClipOp, antiAlias: boolean): void {
        this.getStateMutableSlice().appendClipping(DLSliceClipping.Rect(rect, op, antiAlias));
    }

    public clipRRect(rrect: RRect, op: ClipOp, antiAlias: boolean): void {
        this.getStateMutableSlice().appendClipping(DLSliceClipping.RRect(rrect, op, antiAlias));
    }

    public clipPath(path: Path, op: ClipOp, antiAlias: boolean): void {
        this.getStateMutableSlice().appendClipping(DLSliceClipping.Path(path, op, antiAlias));
    }

    public clipShader(shader: Shader, op: ClipOp): void {
        this.getStateMutableSlice().appendClipping(DLSliceClipping.Shader(shader, op));
    }

    public get canvas(): DLSliceCanvas {
        return this.currentState().slice.getCanvas();
    }

    public finalize(translationContext: Translator.Context): DLDrawOpSlice {
        // introspect.print(`${Fmt.format(this.fRootSlice.fOps)}\n`);
        /*
        for (const op of GskDLDrawOpInspect.Iterate(this.fRootSlice.fOps)) {
            introspect.print(`${GskDLDrawOpInspect.GetName(op.verb)}\n`);
        }
         */

        Optimizers.EliminateRedundantSlices(this.fRootSlice);

        // TODO(sora): perform translation and returns a `Scene`
        Translator.Perform(this.fRootSlice, translationContext, this.fTotalSliceCount);

        introspect.print(`${Fmt.format(this.fRootSlice, { numberToString: v => v.toFixed(2) })}\n`);
        return this.fRootSlice;
    }
}

// A generic implementation of DisplayList recorder.
export class GskDLRecorder extends GskDLRecorderBase<undefined> {
    constructor(viewport: Rect) {
        super(viewport, undefined);
    }
    protected onCopyExtraStates(from: undefined): undefined {}
}

export class DLSliceCanvas {
    private readonly fOps: Array<unknown>;

    constructor(slice: DLDrawOpSlice) {
        this.fOps = slice.fOps;
    }

    public drawColor(color: Color4f, mode: BlendMode): void {
        this.fOps.push(GskDLDrawOpVerb.kDrawColor, color, mode);
    }

    public clear(color: Color4f): void {
        this.fOps.push(GskDLDrawOpVerb.kClear, color);
    }

    public drawPaint(paint: Paint): void {
        this.fOps.push(GskDLDrawOpVerb.kDrawPaint, paint.clone());
    }

    // `pts` should be immutable until `GskDLRecorderBase.finalize()` is called.
    public drawPoints(mode: BlendMode, pts: Float32Array, paint: Paint): void {
        this.fOps.push(GskDLDrawOpVerb.kDrawPoints, mode, pts, paint.clone());
    }

    public drawPoint(x: number, y: number, paint: Paint): void {
        this.fOps.push(GskDLDrawOpVerb.kDrawPoint, x, y, paint.clone());
    }

    public drawRect(rect: Rect, paint: Paint): void {
        this.fOps.push(GskDLDrawOpVerb.kDrawRect, rect, paint.clone());
    }

    // `image` should not be disposed until `GskDLRecorderBase.finalize()` is called.
    public drawImage(image: Image, left: number, top: number, sampling: GskSampling, paint: Paint = null): void {
        this.fOps.push(GskDLDrawOpVerb.kDrawImage, image, left, top, sampling, paint != null ? paint.clone() : null);
    }

    // TODO(sora): implement other DrawOps
}
