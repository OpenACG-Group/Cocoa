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

import { CkPath, CkCanvas, CkPaint, Constants } from 'glamor';
import {
    GskNode,
    GskConcreteType,
    GskInvalidationRecorder,
    NodeTrait,
    GskProperty
} from './GskNode';
import { Rect } from '../base/Rectangle';
import { Point2f } from '../base/Vector';
import { Mat3x3 } from '../base/Matrix';

export enum PathDirection {
    kCW = Constants.PATH_DIRECTION_CW,
    kCCW = Constants.PATH_DIRECTION_CCW
}

/**
 * Base class of drawable geometries, like Rect, RRect, Path, etc.
 * Geometry (opposing to Paint) provides shape information for drawing,
 * while Paint provides "style" information (color, stroke-width...) for drawing.
 */
export abstract class GskGeometry extends GskNode {
    protected constructor(type: GskConcreteType) {
        // Geometry nodes don't generate damage on their own,
        // but via their aggregation ancestor GskDraw nodes.
        super(type, NodeTrait.kBubbleDamage);
    }

    public draw(canvas: CkCanvas, paint: CkPaint): void {
        this.ASSERT_REVALIDATED();
        this.onGeometryDraw(canvas, paint);
    }

    public clip(canvas: CkCanvas, antialias: boolean): void {
        this.ASSERT_REVALIDATED();
        this.onGeometryClip(canvas, antialias);
    }

    public contains(p: Point2f): boolean {
        this.ASSERT_REVALIDATED();
        // Quick reject
        if (!this.bounds.contains(p.x, p.y)) {
            return false;
        }
        return this.onGeometryContains(p);
    }

    public asPath(): CkPath {
        this.ASSERT_REVALIDATED();
        return this.onGeometryAsPath();
    }

    protected abstract onGeometryDraw(canvas: CkCanvas, paint: CkPaint): void;
    protected abstract onGeometryClip(canvas: CkCanvas, antialias: boolean): void;
    protected abstract onGeometryContains(p: Point2f): boolean;
    protected abstract onGeometryAsPath(): CkPath;
}

export class GskRect extends GskGeometry {
    @GskProperty<Rect, GskRect>(Rect.MakeEmpty())
    public rect: Rect;

    @GskProperty<PathDirection, GskRect>(PathDirection.kCW)
    public direction: PathDirection;

    @GskProperty<number, GskRect>(0)
    public initialPointIndex: number;

    public constructor() {
        super(GskConcreteType.kGeometryRect);
    }

    protected onGeometryDraw(canvas: CkCanvas, paint: CkPaint): void {
        if (this.rect.isEmpty()) {
            return;
        }
        canvas.drawRect(this.rect.toCkArrayXYWHRect(), paint);
    }

    protected onGeometryClip(canvas: CkCanvas, antialias: boolean): void {
        if (this.rect.isEmpty()) {
            return;
        }
        canvas.clipRect(this.rect.toCkArrayXYWHRect(), Constants.CLIP_OP_INTERSECT, antialias);
    }

    protected onGeometryContains(p: Point2f): boolean {
        return this.rect.contains(p.x, p.y);
    }

    protected onRevalidate(recorder: GskInvalidationRecorder, ctm: Mat3x3): Rect {
        return this.rect;
    }

    protected onGeometryAsPath(): CkPath {
        const path = new CkPath();
        path.addRect(this.rect.toCkArrayXYWHRect(), this.direction, this.initialPointIndex);
        return path;
    }
}
