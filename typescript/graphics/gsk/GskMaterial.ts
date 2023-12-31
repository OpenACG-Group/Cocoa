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
import { Rect } from '../base/Rectangle';
import {
    GskConcreteType,
    GskNode,
    NodeTrait,
    GskProperty,
    GskInvalidationRecorder
} from './GskNode';

import {
    GskPaintRecord,
    GskPaintStyle,
    GskStrokeJoin,
    GskStrokeCap
} from './GskPaintRecord';

import { GskBlendMode } from './GskBlendMode';
import { Mat3x3 } from '../base/Matrix';

export abstract class GskMaterial extends GskNode {
    @GskProperty<boolean, GskMaterial>(false)
    public antiAlias: boolean;

    @GskProperty<number, GskMaterial>(1)
    public opacity: number;

    @GskProperty<GskBlendMode, GskMaterial>(GskBlendMode.kSrcOver)
    public blendMode: GskBlendMode;

    @GskProperty<number, GskMaterial>(1)
    public strokeWidth: number;

    @GskProperty<number, GskMaterial>(4)
    public strokeMiter: number;

    @GskProperty<GskPaintStyle, GskMaterial>(GskPaintStyle.kFill)
    public style: GskPaintStyle;

    @GskProperty<GskStrokeJoin, GskMaterial>(GskStrokeJoin.kMiter)
    public strokeJoin: GskStrokeJoin;

    @GskProperty<GskStrokeCap, GskMaterial>(GskStrokeCap.kSquare)
    public strokeCap: GskStrokeCap;

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
