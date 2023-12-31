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

import {
    GskConcreteType,
    NodeTrait,
    GskNode,
    GskInvalidationRecorder,
    GskProperty
} from './GskNode';

import { GskRenderNode, RenderContext, GskScopedRenderContext } from './GskRenderNode';
import { GskDisplayList } from './GskDisplayList';
import { Point2f } from '../base/Vector';
import { Rect } from '../base/Rectangle';
import { Maybe } from '../../core/error';
import { CkImageFilter, CkMatrix, Constants } from 'glamor';
import { Mat3x3 } from '../base/Matrix';
import { GskTransform } from './GskTransform';

export class GskEffect extends GskRenderNode {
    protected readonly fChild: GskRenderNode;

    protected constructor(child: GskRenderNode, type: GskConcreteType, traits: number) {
        super(type, traits);
        this.fChild = child;
        this.observeChild(child);
    }

    protected onRender(dl: GskDisplayList, context: RenderContext): void {
        this.fChild.render(dl, context);
    }

    protected onNodeAt(point: Point2f): Maybe<GskRenderNode> {
        return this.fChild.nodeAt(point);
    }

    protected onRevalidate(recorder: GskInvalidationRecorder, ctm: Mat3x3): Rect {
        this.ASSERT_HAS_INVL();
        return this.fChild.revalidate(recorder, ctm);
    }
}

export class GskOpacityEffect extends GskEffect {
    @GskProperty<number, GskOpacityEffect>(1)
    public opacity: number;

    constructor(child: GskRenderNode) {
        super(child, GskConcreteType.kOpacityEffect, 0);
    }

    protected onRender(dl: GskDisplayList, context: RenderContext): void {
        // opacity <= 0 disables rendering
        if (this.opacity <= 0) {
            return;
        }

        // opacity >= 1 has no effect
        if (this.opacity >= 1) {
            super.onRender(dl, context);
            return;
        }

        GskScopedRenderContext(dl, context, (mutator) => {
            mutator.modulateOpacity(this.opacity);
            super.onRender(dl, mutator.asRC());
        });
    }

    protected onNodeAt(point: Point2f): Maybe<GskRenderNode> {
        return (this.opacity > 0 ? super.onNodeAt(point) : Maybe.None());
    }

    protected onRevalidate(recorder: GskInvalidationRecorder, ctm: Mat3x3): Rect {
        this.ASSERT_HAS_INVL();
        return (this.opacity > 0 ? super.onRevalidate(recorder, ctm) : Rect.MakeEmpty());
    }
}

/**
 * Wrap a `CkImageFilter` object into a scenegraph node.
 * This node is one of the descendants of `GskImageFilterEffect` node.
 */
export class GskImageFilter extends GskNode {
    @GskProperty<CkImageFilter, GskImageFilter>()
    public filter: CkImageFilter;

    private fCropRect: Rect;
    private fCroppedFilter: CkImageFilter;

    constructor() {
        super(GskConcreteType.kImageFilter, 0);
        this.fCropRect = Rect.MakeEmpty();
        this.fCroppedFilter = null;
    }

    public resolveCroppedFilter(crop: Rect): CkImageFilter {
        // No cropping is applied so the original filter is returned
        if (crop.isEmpty()) {
            return this.filter;
        }
        // Cache is available
        if (this.fCropRect.equalTo(crop) && this.fCroppedFilter != null) {
            return this.fCroppedFilter;
        }

        // If a valid cropping should be applied, and the cache is not available,
        // the cropped filter need creating.
        this.fCropRect = crop;
        this.fCroppedFilter = CkImageFilter.MakeFromDSL('crop(%crop, _, %input)', {
            crop: new Float32Array([crop.x, crop.y, crop.width, crop.height]),
            input: this.filter
        });

        return this.fCroppedFilter;
    }

    protected onRevalidate(recorder: GskInvalidationRecorder, ctm: Mat3x3): Rect {
        this.ASSERT_HAS_INVL();
        return Rect.MakeEmpty();
    }
}

export enum GskFilterCropping {
    kNone,
    kContent
}

export class GskImageFilterEffect extends GskEffect {
    private readonly fFilter: GskImageFilter;
    private fResolvedFilter: CkImageFilter;

    @GskProperty<GskFilterCropping, GskImageFilterEffect>(GskFilterCropping.kNone)
    public cropping: GskFilterCropping;

    constructor(child: GskRenderNode, filter: GskImageFilter) {
        super(child, GskConcreteType.kImageFilterEffect, NodeTrait.kOverrideDamage);
        this.fFilter = filter;
        this.fResolvedFilter = null;
    }

    protected onRevalidate(recorder: GskInvalidationRecorder, ctm: Mat3x3): Rect {
        this.fFilter.revalidate(recorder, ctm);

        const contentBounds = super.onRevalidate(recorder, ctm);
        const filter = (this.cropping === GskFilterCropping.kContent)
                     ? this.fFilter.resolveCroppedFilter(contentBounds)
                     : this.fFilter.filter;

        this.fResolvedFilter = filter;

        return Rect.MakeFromGL(filter.filterBounds(
            contentBounds.toCkArrayXYWHRect(),
            CkMatrix.Identity(),
            Constants.IMAGE_FILTER_MAP_DIRECTION_FORWARD,
            null
        ));
    }

    protected onNodeAt(point: Point2f): Maybe<GskRenderNode> {
        // FIXME: Map `point` through the filter before dispatching to descendants?
        return super.onNodeAt(point);
    }

    protected onRender(dl: GskDisplayList, context: RenderContext) {
        GskScopedRenderContext(dl, context, (mutator) => {
            // Note: we're using source content bounds for new layer,
            //       not the filtered bounds.
            mutator.setFilterIsolation(this.fChild.bounds, dl.getTotalMatrix(), this.fResolvedFilter);
            super.onRender(dl, mutator.asRC());
        });
    }
}

export class GskTransformEffect extends GskEffect {
    private readonly fT: GskTransform;
    private fInvCache: Mat3x3 | null;

    constructor(child: GskRenderNode, transform: GskTransform) {
        super(child, GskConcreteType.kTransformEffect, 0);
        this.fT = transform;
        this.fInvCache = null;
    }

    protected onRevalidate(recorder: GskInvalidationRecorder, ctm: Mat3x3): Rect {
        this.ASSERT_HAS_INVL();

        this.fInvCache = null;
        this.fT.revalidate(recorder, ctm);
        const matrix = this.fT.asMat3x3();

        const bounds = super.onRevalidate(recorder, Mat3x3.Concat(ctm, matrix));
        return matrix.mapRect(bounds, true);
    }

    protected onRender(dl: GskDisplayList, context: RenderContext) {
        const saveCount = dl.save();
        dl.concatMatrix(this.fT.asMat3x3());
        super.onRender(dl, context);
        dl.restoreToCount(saveCount);
    }

    protected onNodeAt(point: Point2f): Maybe<GskRenderNode> {
        const m = this.fT.asMat3x3();
        if (this.fInvCache == null) {
            const inverse = m.invert();
            if (!inverse.has()) {
                return Maybe.None();
            }
            this.fInvCache = inverse.unwrap();
        }
        return super.onNodeAt(this.fInvCache.mapPoint(point));
    }
}

