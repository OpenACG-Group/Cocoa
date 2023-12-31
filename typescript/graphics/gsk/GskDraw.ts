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
    GskNodeError,
    GskConcreteType,
    NodeTrait,
    GskInvalidationRecorder
} from './GskNode';
import { GskGeometry } from './GskGeometry';
import { GskMaterial } from './GskMaterial';
import { GskRenderNode, RenderContext } from './GskRenderNode';
import { GskPaintStyle } from './GskPaintRecord';
import { GskDisplayList } from './GskDisplayList';
import { Maybe } from '../../core/error';
import { Point2f } from '../base/Vector';
import { Rect } from '../base/Rectangle';
import { Mat3x3 } from '../base/Matrix';

export class GskDraw extends GskRenderNode {
    private readonly fGeometry: GskGeometry;
    private readonly fMaterial: GskMaterial;

    constructor(geometry: GskGeometry, material: GskMaterial) {
        super(GskConcreteType.kDraw, NodeTrait.kNone);
        this.fGeometry = geometry;
        this.fMaterial = material;
        this.observeChild(geometry);
        this.observeChild(material);
    }

    protected onRender(dl: GskDisplayList, context: RenderContext): void {
        const paintRecord = this.fMaterial.makePaintRecord();
        if (context != null) {
            context.modulatePaint(dl.getTotalMatrix(), paintRecord, false);
        }
        const paint = paintRecord.instantiatePaint();
        const skipDraw = paint.nothingToDraw() ||
            (paintRecord.style == GskPaintStyle.kStroke && paintRecord.strokeWidth <= 0);
        if (!skipDraw) {
            this.fGeometry.draw(dl.canvas, paint);
        }
    }

    protected onNodeAt(point: Point2f): Maybe<GskRenderNode> {
        const paintRecord = this.fMaterial.makePaintRecord();
        // Transparent geometry elements are completely invisible
        if (paintRecord.color.transparent()) {
            return Maybe.None();
        }

        if (paintRecord.style == GskPaintStyle.kFill && this.fGeometry.contains(point)) {
            return Maybe.Ok(this);
        }


        const strokePath = this.fGeometry.asPath().fillWithPaint(
            paintRecord.instantiatePaint(), null, 1);
        if (strokePath == null) {
            return Maybe.None();
        }

        return strokePath.contains(point.x, point.y) ? Maybe.Ok(this) : Maybe.None();
    }

    protected onRevalidate(recorder: GskInvalidationRecorder, ctm: Mat3x3): Rect {
        const bounds = this.fGeometry.revalidate(recorder, ctm);
        this.fMaterial.revalidate(recorder, ctm);

        const paint = this.fMaterial.makePaintRecord().instantiatePaint();
        if (!paint.canComputeFastBounds()) {
            GskNodeError.Throw(this, 'Failed to compute the fast bounds of geometry node');
        }

        return Rect.MakeFromGL(paint.computeFastBounds(bounds.toCkArrayXYWHRect()));
    }
}
