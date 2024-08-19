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

import { GskConcreteType, GskHitTestableNode, GskNode, GskProperty, NodeTrait } from './GskNode';
import { Blender, BlendMode, ColorFilter, ImageFilter, Mat3x3, Paint, Rect, Vec2, Shader } from 'renderer';
import { Maybe } from '../../core/error';
import { GskPaintRecord } from './GskPaintRecord';
import { GskDLRecorder } from './GskDisplayList';

function stripShaderCTM(shader: Shader, base: Mat3x3, ctm: Mat3x3): Shader {
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

    let inv = ctm.invert();
    if (!base.equalTo(ctm) && inv != null) {
        return shader.makeWithLocalMatrix(inv.preConcat(base));
    } else {
        return shader;
    }
}

export class RenderContext {
    public fColorFilter: ColorFilter;
    public fShader: Shader;
    public fMaskShader: Shader;
    public fBlender: Blender;
    public fShaderCTM: Mat3x3;
    public fMaskCTM: Mat3x3;
    public fOpacity: number;

    constructor(colorFilter: ColorFilter = null,
                shader: Shader = null,
                maskShader: Shader = null,
                blender: Blender = null,
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
            paint.shader = Shader.Blend(
                BlendMode.SrcIn,
                stripShaderCTM(this.fMaskShader, this.fMaskCTM, ctm),
                paint.shader
            );
        }
    }
}

export class RenderContextMutator {
    private readonly fDL: GskDLRecorder;
    private readonly fRestoreCount: number;
    private fCtx: RenderContext;
    private fMaskShader: Shader | null;

    constructor(dl: GskDLRecorder, ctx: RenderContext) {
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
            // TODO(sora): maybe we can use `canvas.clipShader` instead of handle it manually?
            if (this.fMaskShader != null) {
                const maskPaint = new Paint();
                maskPaint.blendMode = BlendMode.DstIn;
                maskPaint.shader = this.fMaskShader;
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

    public modulateColorFilter(cf: ColorFilter): RenderContextMutator {
        if (cf == null) {
            this.fCtx.fColorFilter = null;
        } else if (this.fCtx.fColorFilter == null) {
            this.fCtx.fColorFilter = cf;
        } else {
            this.fCtx.fColorFilter = ColorFilter.Compose(this.fCtx.fColorFilter, cf);
        }
        return this;
    }

    public modulateShader(shader: Shader, ctm: Mat3x3): RenderContextMutator {
        // Topmost shader takes precedence
        if (this.fCtx.fShader == null) {
            this.fCtx.fShader = shader;
            this.fCtx.fShaderCTM = ctm;
        }
        return this;
    }

    public modulateMaskShader(shader: Shader, ctm: Mat3x3): RenderContextMutator {
        if (this.fCtx.fMaskShader != null) {
            // As we compose mask filters, use the relative transform T for the inner mask:
            //
            //   maskCTM x T = ctm
            //
            //   => T = Inv(maskCTM) x ctm
            //
            const invMaskCTM = this.fCtx.fMaskCTM.invert();
            if (invMaskCTM != null && shader != null) {
                this.fCtx.fMaskShader = Shader.Blend(
                    BlendMode.SrcIn,
                    this.fCtx.fMaskShader,
                    shader.makeWithLocalMatrix(invMaskCTM.postConcat(ctm))
                );
            }
        } else {
            this.fCtx.fMaskShader = shader;
            this.fCtx.fMaskCTM = ctm;
        }
        return this;
    }

    public modulateBlender(blender: Blender): RenderContextMutator {
        this.fCtx.fBlender = blender;
        return this;
    }

    public setIsolation(bounds: Rect, ctm: Mat3x3, isolation: boolean): RenderContextMutator {
        if (isolation && this.fCtx.requiresIsolation()) {
            const layerPaintRec = new GskPaintRecord();
            this.fCtx.modulatePaint(ctm, layerPaintRec, true);
            this.fDL.saveLayer({
                bounds: bounds,
                alpha: layerPaintRec.color.A,
                blendMode: layerPaintRec.blendMode,
                blender: layerPaintRec.blender
            });

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

    public setFilterIsolation(bounds: Rect, ctm: Mat3x3, filter: ImageFilter): RenderContextMutator {
        if (filter != null) {
            const layerPaintRec = new GskPaintRecord();
            this.fCtx.modulatePaint(ctm, layerPaintRec, false);

            this.fDL.saveLayer({
                bounds: bounds,
                alpha: layerPaintRec.color.A,
                blendMode: layerPaintRec.blendMode,
                blender: layerPaintRec.blender,
                filter: filter
            });
            this.fCtx = new RenderContext();
        }

        return this;
    }
}

export function GskScopedRenderContext<ThisT>(
    dl: GskDLRecorder,
    ctx: RenderContext | null,
    callback: (this: ThisT, mutator: RenderContextMutator) => void,
    thisArg: ThisT = undefined,
): void {
    const mutator = new RenderContextMutator(dl, ctx);
    callback.apply(thisArg, [ mutator ]);
    mutator.restore();
}

export abstract class GskRenderNode extends GskNode implements GskHitTestableNode<GskRenderNode> {
    @GskProperty<boolean, GskRenderNode>(true)
    public visible: boolean;

    protected constructor(type: GskConcreteType, trait: NodeTrait) {
        super(type, trait);
    }

    /**
     * Render the node and its descendants to the canvas.
     */
    public render(dl: GskDLRecorder, context: RenderContext): void {
        this.ASSERT_REVALIDATED();
        if (this.visible && !this.bounds.isEmpty()) {
            // Providing the bounds of DrawOps explicitly helps the optimization of
            // LayerTree structure.
            const saveCount = dl.saveBounds(this.bounds);
            this.onRender(dl, context);
            dl.restoreToCount(saveCount);
        }
        this.ASSERT_REVALIDATED();
    }

    /**
     * Perform a front-to-back hit-test, and return the RenderNode located
     * at `point`. Normally, hit-testing stops at leaf `GskDraw` nodes.
     */
    public nodeAt(point: Vec2): Maybe<GskRenderNode> {
        // Do a fast computation to eject most of the points (bounds-rejection).
        if (!this.bounds.contains(point.x, point.y)) {
            return Maybe.None();
        }
        // Accurate test
        return this.onNodeAt(point);
    }

    protected abstract onRender(dl: GskDLRecorder, context: RenderContext): void;
    protected abstract onNodeAt(point: Vec2): Maybe<GskRenderNode>;
}
