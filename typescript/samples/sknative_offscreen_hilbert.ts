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

import * as std from 'core';
import * as GL from 'glamor';

const path = new GL.CkPath();
path.moveTo(2, 2);

function L(x: number, y: number) {
    path.rLineTo(x, y);
}

// Following 4 functions generate a Hilbert curve path: https://en.wikipedia.org/wiki/Hilbert_curve.
// The algorithm is originally from CairoScript examples of Cairo project, and
// translated to TypeScript by Cocoa. The original CairoScript is:
// https://github.com/freedesktop/cairo/blob/master/util/cairo-script/examples/hilbert.cs

function hA(a: number, b: number) {
    if (b == 0) {
        return a;
    } else {
        const c = b - 1;
        L(0, hB(a, c));
        L(hA(a, c), 0);
        L(0, -hA(a, c));
        return hC(a, c);
    }
}

function hB(a: number, b: number) {
    if (b == 0) {
        return a;
    } else {
        const c = b - 1;
        L(hA(a, c), 0);
        L(0, hB(a, c));
        L(-hB(a, c), 0);
        return hD(a, c);
    }
}

function hC(a: number, b: number) {
    if (b == 0) {
        return a;
    } else {
        const c = b - 1;
        L(-hD(a, c), 0);
        L(0, -hC(a, c));
        L(hC(a, c), 0);
        return hA(a, c);
    }
}

function hD(a: number, b: number) {
    if (b == 0) {
        return a;
    } else {
        const c = b - 1;
        L(0, -hC(a, c));
        L(-hD(a, c), 0);
        L(0, hD(a, c));
        return hB(a, c);
    }
}

const surface = GL.CkSurface.MakeRaster({
    colorType: GL.Constants.COLOR_TYPE_BGRA8888,
    alphaType: GL.Constants.ALPHA_TYPE_OPAQUE,
    colorSpace: GL.Constants.COLOR_SPACE_SRGB,
    width: 1024,
    height: 1024
});

const canvas = surface.getCanvas();
const paint = new GL.CkPaint();
paint.setStyle(GL.Constants.PAINT_STYLE_STROKE);
paint.setStrokeWidth(2);
paint.setColor4f([0, 0, 1, 1]);

canvas.clear([1, 1, 1, 1]);

// The 10th orders Hilbert curve
hA(4, 10);
canvas.drawPath(path, paint);

const image = surface.makeImageSnapshot(null);
std.File.WriteFileSync('result.png', image.encodeToData(GL.Constants.FORMAT_PNG, 100));
