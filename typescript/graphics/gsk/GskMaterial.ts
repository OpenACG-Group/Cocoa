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

import { Color4f } from '../base/Color';
import {
    GskConcreteType,
    GskNode,
    NodeTrait,
    GskProperty,
    GskInvalidationRecorder
} from './GskNode';

import { GskPaintRecord } from './GskPaintRecord';
import { Mat3x3, BlendMode, Rect, Style, LineCap, LineJoin } from 'renderer';

export abstract class GskMaterial extends GskNode {
    @GskProperty<boolean, GskMaterial>(false)
    public antiAlias: boolean;

    @GskProperty<number, GskMaterial>(1)
    public opacity: number;

    @GskProperty<BlendMode, GskMaterial>(BlendMode.SrcOver)
    public blendMode: BlendMode;

    @GskProperty<number, GskMaterial>(1)
    public strokeWidth: number;

    @GskProperty<number, GskMaterial>(4)
    public strokeMiter: number;

    @GskProperty<Style, GskMaterial>(Style.Fill)
    public style: Style;

    @GskProperty<LineJoin, GskMaterial>(LineJoin.Default)
    public strokeJoin: LineJoin;

    @GskProperty<LineCap, GskMaterial>(LineCap.Default)
    public strokeCap: LineCap;

    protected constructor(type: GskConcreteType) {
        // Material nodes don't generate damage on their own,
        // but via their aggregation ancestor Draw nodes.
        super(type, NodeTrait.kBubbleDamage);
    }

    public makePaintRecord(): GskPaintRecord {
        this.ASSERT_REVALIDATED();
        const paint = new GskPaintRecord();
        paint.antiAlias = this.antiAlias;
        paint.blendMode = this.blendMode;
        paint.style = this.style;
        paint.strokeWidth = this.strokeWidth;
        paint.strokeMiter = this.strokeMiter;
        paint.strokeJoin = this.strokeJoin;
        paint.strokeCap = this.strokeCap;

        // Apply subclass values
        this.onApplyToPaint(paint);

        // Compose opacity on top of the subclass value
        paint.color = paint.color.mula(this.opacity);

        return paint;
    }

    protected abstract onApplyToPaint(paint: GskPaintRecord): void;
}

export class GskMaterialColor extends GskMaterial {
    @GskProperty<Color4f, GskMaterialColor>(Color4f.Make(0, 0, 0, 1))
    public color: Color4f;

    constructor() {
        super(GskConcreteType.kMaterialColor);
    }

    protected onApplyToPaint(paint: GskPaintRecord): void {
        paint.color = this.color;
    }

    protected onRevalidate(recorder: GskInvalidationRecorder, ctm: Mat3x3): Rect {
        return Rect.MakeEmpty();
    }
}
