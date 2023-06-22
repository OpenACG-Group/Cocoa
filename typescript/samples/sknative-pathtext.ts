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

//! [Sample: SKNATIVE]
//! Description: Draw a string along with a certain path
//! Reference:
//!   [1] Understand how to position glyphs along with a certain path:
//!       https://www.pushing-pixels.org/2022/02/10/drawing-text-on-a-path-in-compose-desktop-with-skia.html
//!   [2] Text path
//!       https://developer.mozilla.org/zh-CN/docs/Web/SVG/Element/textPath

import * as GL from 'glamor';
import * as std from 'core';

const TEXT = 'Hello, World! Text on the Curve!'
const TEXT_UTF8 = std.Buffer.MakeFromString(TEXT, std.Buffer.ENCODE_UTF8).byteArray;

const typeface = GL.defaultFontMgr.matchFamilyStyle('Georgia', GL.CkFontStyle.MakeNormal());
if (typeface == null) {
    throw Error('Missing system font Georgia');
}

const font = GL.CkFont.MakeFromSize(typeface, 42.5);

const glyphs: Uint16Array = typeface.textToGlyphs(TEXT_UTF8, GL.Constants.TEXT_ENCODING_UTF8);
const glyphBounds: GL.CkArrayXYWHRect[] = font.getBounds(glyphs, null);
const glyphPositions: GL.CkPoint[] = font.getPos(glyphs, [0, 0]);

const path = new GL.CkPath();
path.moveTo(100, 200);
path.cubicTo(100, 100, 300, 0, 400, 100);
path.cubicTo(500, 200, 600, 300, 700, 200);
path.cubicTo(800, 100, 900, 100, 900, 200);

const pathMeasure = GL.CkPathMeasure.Make(path, false, 1);

const textStartOffset = glyphPositions[0][0];
const visibleGlyphTransforms: GL.CkRSXform[] = [];

for (let g = 0; g < glyphs.length; g++) {
    const startOffset = glyphPositions[g];
    const width = glyphBounds[g][2];
    const midPointOffset = textStartOffset + startOffset[0] + width / 2;
    if (midPointOffset >= 0 && midPointOffset < pathMeasure.getLength()) {
        const {position, tangent} = pathMeasure.getPositionTangent(midPointOffset);
        let tx = position[0], ty = position[1];
        tx -= tangent[0] * width / 2;
        ty -= tangent[1] * width / 2;
        const y = glyphPositions[g][1];
        tx -= y * tangent[1];
        ty += y * tangent[0];

        visibleGlyphTransforms.push({
            scos: tangent[0],
            ssin: tangent[1],
            tx: tx,
            ty: ty
        });
    }
}

const surface = GL.CkSurface.MakeRaster({
    alphaType: GL.Constants.ALPHA_TYPE_OPAQUE,
    colorType: GL.Constants.COLOR_TYPE_BGRA8888,
    colorSpace: GL.Constants.COLOR_SPACE_SRGB,
    width: 1000,
    height: 300
});

const canvas = surface.getCanvas();
canvas.clear([1, 1, 1, 1]);

const paint = new GL.CkPaint();
paint.setColor4f([0, 0, 0, 1]);
paint.setAntiAlias(true);

const textBlob = GL.CkTextBlob.MakeFromRSXformText(
    TEXT_UTF8,
    visibleGlyphTransforms,
    font,
    GL.Constants.TEXT_ENCODING_UTF8
);

canvas.drawTextBlob(textBlob, 0, 0, paint);

paint.setColor4f([0, 0, 1, 1]);
paint.setStyle(GL.Constants.PAINT_STYLE_STROKE);
paint.setStrokeWidth(3);
canvas.drawPath(path, paint);

std.File.WriteFileSync('./output.png',
    surface.makeImageSnapshot(null).encodeToData(GL.Constants.FORMAT_PNG, 100));
