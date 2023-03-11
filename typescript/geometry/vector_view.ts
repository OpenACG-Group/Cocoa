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

import {Vector2f, Vector3f} from "./vector";
import {GeoViewBase} from "./geometry_base";
import * as GL from 'glamor';

export interface DrawVectorParameters {
    /* Supposing the vector to be drawn is v: */

    // Linear scalar for the length of vector landing on the canvas (in pixels):
    // L = |v| * K + B
    linearScalarK: number;
    linearScalarB: number;

    // Angle size in rad of arrow head
    arrowheadAngle: number;
    arrowheadSize: number;

    // A single line text to describe the vector
    text?: string;
    textFont?: CanvasKit.Font;
    textFontPaint?: CanvasKit.Paint;

    // A paint to specify extra effects
    paint: CanvasKit.Paint;

    // Origin point to draw the vector
    origin: Vector2f;
}


export class Vector2fView implements GeoViewBase {
    defaultFontPaint: CanvasKit.Paint;
    defaultFont: CanvasKit.Font;

    constructor() {
        // TODO(sora): Setup default font paint and default font.
        //             As we have changed canvaskit.js, recompilation of QResource is required.
    }

    draw(v: Vector2f, canvas: CanvasKit.Canvas, parameters: DrawVectorParameters): void
    {
        const arrowEnd = parameters.origin.add(v);

        const head = v.neg().normalize().multiply(parameters.arrowheadSize);
        let r1 = arrowEnd.add(head.rotate(parameters.arrowheadAngle / 2));
        let r2 = arrowEnd.add(head.rotate(-parameters.arrowheadAngle / 2));

        const path = new canvaskit.Path();
        path.moveTo(parameters.origin.x, parameters.origin.y);
        path.lineTo(arrowEnd.x, arrowEnd.y);
        path.lineTo(r1.x, r1.y);
        path.moveTo(arrowEnd.x, arrowEnd.y);
        path.lineTo(r2.x, r2.y);

        canvas.drawPath(path, parameters.paint);
        path.delete();

        if (parameters.text) {
            let fontPaint = parameters.textFontPaint;
            let font = parameters.textFont;
            if (!fontPaint) {
                fontPaint = this.defaultFontPaint;
            }
            if (!font) {
                font = this.defaultFont;
            }
            canvas.drawText(parameters.text, arrowEnd.x, arrowEnd.y, fontPaint, font);
        }
    }
}
