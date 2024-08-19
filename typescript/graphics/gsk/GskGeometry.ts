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

import { ClipOp, Mat3x3, Paint, Path, PathBuilder, PathDirection, Rect, Vec2 } from 'renderer';
import { GskConcreteType, GskInvalidationRecorder, GskNode, GskProperty, NodeTrait } from './GskNode';
import { GskDLRecorder } from './GskDisplayList';

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

    public draw(dl: GskDLRecorder, paint: Paint): void {
        this.ASSERT_REVALIDATED();
        this.onGeometryDraw(dl, paint);
    }

    public clip(dl: GskDLRecorder, antialias: boolean): void {
        this.ASSERT_REVALIDATED();
        this.onGeometryClip(dl, antialias);
    }

    public contains(p: Vec2): boolean {
        this.ASSERT_REVALIDATED();
        // Quick reject
        if (!this.bounds.contains(p.x, p.y)) {
            return false;
        }
        return this.onGeometryContains(p);
    }

    public asPath(): Path {
        this.ASSERT_REVALIDATED();
        return this.onGeometryAsPath();
    }

    protected abstract onGeometryDraw(dl: GskDLRecorder, paint: Paint): void;
    protected abstract onGeometryClip(dl: GskDLRecorder, antialias: boolean): void;
    protected abstract onGeometryContains(p: Vec2): boolean;
    protected abstract onGeometryAsPath(): Path;
}

export class GskRect extends GskGeometry {
    @GskProperty<Rect, GskRect>(Rect.MakeEmpty())
    public rect: Rect;

    @GskProperty<PathDirection, GskRect>(PathDirection.CW)
    public direction: PathDirection;

    @GskProperty<number, GskRect>(0)
    public initialPointIndex: number;

    public constructor() {
        super(GskConcreteType.kGeometryRect);
    }

    protected onGeometryDraw(dl: GskDLRecorder, paint: Paint): void {
        if (this.rect.isEmpty()) {
            return;
        }
        dl.canvas.drawRect(this.rect, paint);
    }

    protected onGeometryClip(dl: GskDLRecorder, antialias: boolean): void {
        if (this.rect.isEmpty()) {
            return;
        }
        dl.clipRect(this.rect, ClipOp.Intersect, antialias);
    }

    protected onGeometryContains(p: Vec2): boolean {
        return this.rect.contains(p.x, p.y);
    }

    protected onRevalidate(recorder: GskInvalidationRecorder, ctm: Mat3x3): Rect {
        return this.rect;
    }

    protected onGeometryAsPath(): Path {
        return new PathBuilder().addRect(this.rect, this.direction, this.initialPointIndex).detach();
    }
}
