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
import {
    Blender,
    BlendMode,
    ColorFilter,
    ImageFilter,
    LineCap,
    LineJoin,
    Paint,
    PathEffect,
    Shader,
    Style
} from 'renderer';

export class GskPaintRecord {
    public pathEffect: PathEffect = null;
    public shader: Shader = null;
    public colorFilter: ColorFilter = null;
    public imageFilter: ImageFilter = null;
    public blender: Blender = null;

    public strokeWidth: number = 1;
    public strokeMiter: number = 4;
    public color: Color4f = Colors.kColorBlackF;
    public antiAlias: boolean = false;
    public dither: boolean = false;
    public strokeCap: LineCap = LineCap.Default;
    public strokeJoin: LineJoin = LineJoin.Default;
    public style: Style = Style.Fill;
    public blendMode: BlendMode = BlendMode.SrcOver;

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

    public instantiatePaint(): Paint {
        const paint = new Paint();
        paint.pathEffect = this.pathEffect;
        paint.shader = this.shader;
        paint.colorFilter = this.colorFilter;
        paint.imageFilter = this.imageFilter;
        paint.strokeWidth = this.strokeWidth;
        paint.strokeMiter = this.strokeMiter;
        paint.color = this.color.toColor32();
        paint.antiAlias = this.antiAlias;
        paint.dither = this.dither;
        paint.strokeCap = this.strokeCap;
        paint.strokeJoin = this.strokeJoin;
        paint.style = this.style;
        // Blender has higher priority than BlendMode
        if (this.blender != null) {
            paint.blender = this.blender;
        } else {
            paint.blendMode = this.blendMode;
        }
        return paint;
    }

    public postConcatColorFilter(cf: ColorFilter): void {
        if (this.colorFilter == null) {
            this.colorFilter = cf;
            return;
        }
        this.colorFilter = ColorFilter.Compose(cf, this.colorFilter);
    }
}
