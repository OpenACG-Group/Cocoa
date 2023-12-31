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

import { GskNode, NodeTrait, GskConcreteType, GskProperty } from './GskNode';
import {
    CkColorFilter,
    CkShader,
    CkBlender,
    CkMatrix,
    CkPaint,
    CkImageFilter
} from 'glamor';
import { Point2f } from '../base/Vector';
import { Maybe } from '../../core/error';
import { GskPaintRecord } from './GskPaintRecord';
import { GskBlendMode } from './GskBlendMode';
import { GskDisplayList } from './GskDisplayList';
import { Rect } from '../base/Rectangle';
import { Mat3x3 } from '../base/Matrix';

function stripShaderCTM(shader: CkShader, base: Mat3x3, ctm: Mat3x3): CkShader {
    // Mask filters / shaders are declared to operate under a specific transform, but due to the
    // deferral mechanism, other transformations might have been pushed to the state.
    // We want to undo these transforms (T):
    //
    //   baseCTM x T = ctm
    //
    //   =>  T = Inv(baseCTM) x ctm
    //
    //   =>  Inv(T) = Inv(Inv(baseCTM) x ctm)
    //
    //   =>  Inv(T) = Inv(ctm) x baseCTM

    let lm = ctm.invert();
    if (!base.equalTo(ctm) && lm.has()) {
        return shader.makeWithLocalMatrix(lm.unwrap().preConcat(base).toCkMat3x3Array());
    } else {
        return shader.makeWithLocalMatrix(CkMatrix.Identity());
    }
}

export class RenderContext {
    public fColorFilter: CkColorFilter;
    public fShader: CkShader;
    public fMaskShader: CkShader;
    public fBlender: CkBlender;
    public fShaderCTM: Mat3x3;
    public fMaskCTM: Mat3x3;
    public fOpacity: number;

    constructor(colorFilter: CkColorFilter = null,
                shader: CkShader = null,
                maskShader: CkShader = null,
                blender: CkBlender = null,
                shaderCTM: Mat3x3 = Mat3x3.Identity(),
                maskCTM: Mat3x3 = Mat3x3.Identity(),
                opacity: number = 1)
    {
        this.fColorFilter = colorFilter;
        this.fShader = shader;
        this.fMaskShader = maskShader;
        this.fBlender = blender;
        this.fShaderCTM = shaderCTM;
        this.fMaskCTM = maskCTM;
        this.fOpacity = opacity;
    }

    public clone(): RenderContext {
        return new RenderContext(
            this.fColorFilter,
            this.fShader,
            this.fMaskShader,
            this.fBlender,
            this.fShaderCTM,
            this.fMaskCTM,
            this.fOpacity
        );
    }

    public requiresIsolation(): boolean {
        if (Math.round(this.fOpacity * 255) != 255) {
            return true;
        }
        return (this.fColorFilter != null || this.fBlender != null ||
                this.fMaskShader != null);
    }

    public modulatePaint(ctm: Mat3x3, paint: GskPaintRecord, isLayerPaint: boolean): void {
        paint.color = paint.color.mula(this.fOpacity);
        if (this.fColorFilter != null) {
            paint.postConcatColorFilter(this.fColorFilter);
        } else {
            paint.colorFilter = null;
        }

        if (this.fShader != null) {
            paint.shader = stripShaderCTM(this.fShader, this.fShaderCTM, ctm);
        }
        if (this.fBlender != null) {
            paint.blender = this.fBlender;
        }

        // Only apply the shader mask for regular paints. Isolation layers require
        // special handling on restore. (See `RenderContextMutator.restore()` method).
        if (this.fMaskShader != null && !isLayerPaint) {
            paint.shader = CkShader.MakeFromDSL('blend(%mode, %dst, %src)', {
                mode: GskBlendMode.kSrcIn,
                dst: stripShaderCTM(this.fMaskShader, this.fMaskCTM, ctm),
                src: paint.shader
            });
        }
    }
}

export class RenderContextMutator {
    private readonly fDL: GskDisplayList;
    private readonly fRestoreCount: number;
    private fCtx: RenderContext;
    private fMaskShader: CkShader | null;

    constructor(dl: GskDisplayList, ctx: RenderContext) {
        this.fDL = dl;
        if (ctx == null) {
            ctx = new RenderContext();
        } else {
            // The origin context should be left untouched
            ctx = ctx.clone();
        }
        this.fCtx = ctx;
        this.fRestoreCount = dl.getSaveCount();
        this.fMaskShader = null;
    }

    public asRC(): RenderContext {
        return this.fCtx;
    }

    public restore(): void {
        if (this.fRestoreCount >= 0) {
            if (this.fMaskShader != null) {
                const maskPaint = new CkPaint();
                maskPaint.setBlendMode(GskBlendMode.kDstIn);
                maskPaint.setShader(this.fMaskShader);
                this.fDL.canvas.drawPaint(maskPaint);
            }
            this.fDL.restoreToCount(this.fRestoreCount);
        }
    }

    public modulateOpacity(opacity: number): RenderContextMutator {
        if (opacity < 0 || opacity > 1) {
            throw RangeError('Opacity is out of range');
        }
        this.fCtx.fOpacity *= opacity;
        return this;
    }

    public modulateColorFilter(cf: CkColorFilter): RenderContextMutator {
        if (cf == null) {
            this.fCtx.fColorFilter = null;
        } else if (this.fCtx.fColorFilter == null) {
            this.fCtx.fColorFilter = cf;
        } else {
            this.fCtx.fColorFilter = CkColorFilter.MakeFromDSL(
                'compose(%outer, %inner)',
                { outer: this.fCtx.fColorFilter, inner: cf }
            );
        }
        return this;
    }

    public modulateShader(shader: CkShader, ctm: Mat3x3): RenderContextMutator {
        // Topmost shader takes precedence
        if (this.fCtx.fShader == null) {
            this.fCtx.fShader = shader;
            this.fCtx.fShaderCTM = ctm;
        }
        return this;
    }

    public modulateMaskShader(shader: CkShader, ctm: Mat3x3): RenderContextMutator {
        if (this.fCtx.fMaskShader != null) {
            // As we compose mask filters, use the relative transform T for the inner mask:
            //
            //   maskCTM x T = ctm
            //
            //   => T = Inv(maskCTM) x ctm
            //
            const invMaskCTM = this.fCtx.fMaskCTM.invert();
            if (invMaskCTM.has() && shader != null) {
                this.fCtx.fMaskShader = CkShader.MakeFromDSL('blend(%mode, %dst, %src)', {
                    mode: GskBlendMode.kSrcIn,
                    dst: this.fCtx.fMaskShader,
                    src: shader.makeWithLocalMatrix(invMaskCTM.unwrap().postConcat(ctm).toCkMat3x3Array())
                });
            }
        } else {
            this.fCtx.fMaskShader = shader;
            this.fCtx.fMaskCTM = ctm;
        }
        return this;
    }

    public modulateBlender(blender: CkBlender): RenderContextMutator {
        this.fCtx.fBlender = blender;
        return this;
    }

    public setIsolation(bounds: Rect, ctm: Mat3x3, isolation: boolean): RenderContextMutator {
        if (isolation && this.fCtx.requiresIsolation()) {
            const layerPaintRec = new GskPaintRecord();
            this.fCtx.modulatePaint(ctm, layerPaintRec, true);
            this.fDL.saveLayer(bounds, layerPaintRec);

            // Fetch the mask shader for restore
            if (this.fCtx.fMaskShader != null) {
                this.fMaskShader = stripShaderCTM(this.fCtx.fMaskShader, this.fCtx.fMaskCTM, ctm);
            }

            // Reset only the props applied via isolation layers
            this.fCtx.fColorFilter = null;
            this.fCtx.fMaskShader = null;
            this.fCtx.fBlender = null;
            this.fCtx.fOpacity = 1;
        }
        return this;
    }

    public setFilterIsolation(bounds: Rect, ctm: Mat3x3, filter: CkImageFilter): RenderContextMutator {
        if (filter != null) {
            const layerPaintRec = new GskPaintRecord();
            this.fCtx.modulatePaint(ctm, layerPaintRec, false);

            layerPaintRec.imageFilter = filter;
            this.fDL.saveLayer(bounds, layerPaintRec);
            this.fCtx = new RenderContext();
        }

        return this;
    }
}

export function GskScopedRenderContext<ThisT>(
    dl: GskDisplayList,
    ctx: RenderContext | null,
    callback: (this: ThisT, mutator: RenderContextMutator) => void,
    thisArg: ThisT = undefined,
): void {
    const mutator = new RenderContextMutator(dl, ctx);
    callback.apply(thisArg, [ mutator ]);
    mutator.restore();
}

export abstract class GskRenderNode extends GskNode {
    @GskProperty<boolean, GskRenderNode>(true)
    public visible: boolean;

    protected constructor(type: GskConcreteType, trait: NodeTrait) {
        super(type, trait);
    }

    /**
     * Render the node and its descendants to the canvas.
     */
    public render(dl: GskDisplayList, context: RenderContext): void {
        this.ASSERT_REVALIDATED();
        if (this.visible && !this.bounds.isEmpty()) {
            this.onRender(dl, context);
        }
        this.ASSERT_REVALIDATED();
    }

    /**
     * Perform a front-to-back hit-test, and return the RenderNode located
     * at `point`. Normally, hit-testing stops at leaf `GskDraw` nodes.
     */
    public nodeAt(point: Point2f): Maybe<GskRenderNode> {
        // Do a fast computation to eject most of the points (bounds-rejection).
        if (!this.bounds.contains(point.x, point.y)) {
            return Maybe.None();
        }
        // Accurate test
        return this.onNodeAt(point);
    }

    protected abstract onRender(dl: GskDisplayList, context: RenderContext): void;
    protected abstract onNodeAt(point: Point2f): Maybe<GskRenderNode>;
}
