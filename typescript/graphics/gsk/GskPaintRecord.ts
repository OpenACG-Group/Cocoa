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

import { Color4f, Const as Colors } from '../base/Color';
import { GskBlendMode } from './GskBlendMode';
import {
    Constants,
    CkPaint,
    CkPathEffect,
    CkShader,
    CkColorFilter,
    CkImageFilter,
    CkBlender
} from 'glamor';

export enum GskPaintStyle {
    kFill = Constants.PAINT_STYLE_FILL,
    kStroke = Constants.PAINT_STYLE_STROKE,
    kStrokeFill = Constants.PAINT_STYLE_STROKE_FILL
}

export enum GskStrokeJoin {
    kMiter = Constants.PAINT_JOIN_MITER,
    kRound = Constants.PAINT_JOIN_ROUND,
    kBevel = Constants.PAINT_JOIN_BEVEL
}

export enum GskStrokeCap {
    kButt = Constants.PAINT_CAP_BUTT,
    kRound = Constants.PAINT_JOIN_ROUND,
    kSquare = Constants.PAINT_CAP_SQUARE
}

export class GskPaintRecord {
    public pathEffect: CkPathEffect = null;
    public shader: CkShader = null;
    public colorFilter: CkColorFilter = null;
    public imageFilter: CkImageFilter = null;
    public blender: CkBlender = null;

    public strokeWidth: number = 1;
    public strokeMiter: number = 4;
    public color: Color4f = Colors.kColorBlackF;
    public antiAlias: boolean = false;
    public dither: boolean = false;
    public strokeCap: GskStrokeCap = GskStrokeCap.kSquare;
    public strokeJoin: GskStrokeJoin = GskStrokeJoin.kMiter;
    public style: GskPaintStyle = GskPaintStyle.kFill;
    public blendMode: GskBlendMode = GskBlendMode.kSrcOver;

    constructor(from?: GskPaintRecord) {
        if (from) {
            this.pathEffect = from.pathEffect;
            this.shader = from.shader;
            this.colorFilter = from.colorFilter;
            this.imageFilter = from.imageFilter;
            this.blender = from.blender;
            this.strokeWidth = from.strokeWidth;
            this.strokeMiter = from.strokeMiter;
            this.color = from.color;
            this.antiAlias = from.antiAlias;
            this.dither = from.dither;
            this.strokeCap = from.strokeCap;
            this.strokeJoin = from.strokeJoin;
            this.style = from.style;
            this.blendMode = from.blendMode;
        }
    }

    public isOpaque(): boolean {
        return Math.round(this.color.A * 255) === 255;
    }

    public instantiatePaint(): CkPaint {
        const paint = new CkPaint();
        if (this.pathEffect) {
            paint.setPathEffect(this.pathEffect);
        }
        if (this.shader) {
            paint.setShader(this.shader);
        }
        if (this.colorFilter) {
            paint.setColorFilter(this.colorFilter);
        }
        if (this.imageFilter) {
            paint.setImageFilter(this.imageFilter);
        }
        if (this.blender) {
            paint.setBlender(this.blender);
        }
        paint.setStrokeWidth(this.strokeWidth);
        paint.setStrokeMiter(this.strokeMiter);
        paint.setColor4f(this.color.toCkColor4f());
        paint.setAntiAlias(this.antiAlias);
        paint.setDither(this.dither);
        paint.setStrokeCap(this.strokeCap);
        paint.setStrokeJoin(this.strokeJoin);
        paint.setStyle(this.style);
        return paint;
    }

    /**
     * Assuming the original color filter is G(c), and the provided filter is F(c).
     * The result filter is G(F(c)).
     */
    public preConcatColorFilter(f: CkColorFilter): GskPaintRecord {
        if (this.colorFilter == null) {
            this.colorFilter = f;
        } else {
            this.colorFilter = CkColorFilter.MakeFromDSL('compose(%outer, %inner)', {
                outer: this.colorFilter,
                inner: f
            });
        }
        return this;
    }

    /**
     * Assuming the original color filter is G(c), and the provided filter is F(c).
     * The result filter is F(G(c)).
     */
    public postConcatColorFilter(f: CkColorFilter): GskPaintRecord {
        if (this.colorFilter == null) {
            this.colorFilter = f;
        } else {
            this.colorFilter = CkColorFilter.MakeFromDSL('compose(%outer, %inner)', {
                outer: f,
                inner: this.colorFilter
            });
        }
        return this;
    }

    public preConcatImageFilter(f: CkImageFilter): GskPaintRecord {
        if (this.imageFilter == null) {
            this.imageFilter = f;
        } else {
            this.imageFilter = CkImageFilter.MakeFromDSL('compose(%outer, %inner)', {
                outer: this.imageFilter,
                inner: f
            });
        }
        return this;
    }

    public postConcatImageFilter(f: CkImageFilter): GskPaintRecord {
        if (this.imageFilter == null) {
            this.imageFilter = f;
        } else {
            this.imageFilter = CkImageFilter.MakeFromDSL('compose(%outer, %inner)', {
                outer: f,
                inner: this.imageFilter
            });
        }
        return this;
    }
}
