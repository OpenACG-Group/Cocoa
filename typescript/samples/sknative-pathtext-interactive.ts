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

//! [Sample: SKNATIVE, GLAMOR]
//! Description: Draw a string along a certain path
//! Reference: Offscreen version: `samples/sknative-pathtext.ts`

import * as GL from 'glamor';
import * as std from 'core';

const TEXT = 'Hello, World! Text on the Curve!'
const TEXT_UTF8 = std.Buffer.MakeFromString(TEXT, std.Buffer.ENCODE_UTF8).byteArray;

const WINDOW_WIDTH = 1000, WINDOW_HEIGHT = 300;

const typeface = GL.defaultFontMgr.matchFamilyStyle('Georgia', GL.CkFontStyle.MakeNormal());
if (typeface == null) {
    throw Error('Missing system font Georgia');
}

const font = GL.CkFont.MakeFromSize(typeface, 42.5);

const glyphs: Uint16Array = typeface.textToGlyphs(TEXT_UTF8, GL.Constants.TEXT_ENCODING_UTF8);
const glyphBounds: GL.CkArrayXYWHRect[] = font.getBounds(glyphs, null);
const glyphPositions: GL.CkPoint[] = font.getPos(glyphs, [0, 0]);

function draw(canvas: GL.CkCanvas, path: GL.CkPath): void {
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
}

interface WindowContext {
    display: GL.Display;
    surface: GL.Surface;
}

async function setupWindowContext(): Promise<WindowContext> {
    const presentThread = await GL.PresentThread.Start();
    let display = await presentThread.createDisplay();
    display.addOnceListener('closed', () => {
        presentThread.dispose();
    });

    const surface = await display.createHWComposeSurface(WINDOW_WIDTH, WINDOW_HEIGHT);

    await surface.setTitle('PathText Interactive');

    return {
        display: display,
        surface: surface
    };
}

function render(ctx: WindowContext, pts: GL.CkPoint[]): void {
    const rec = new GL.CkPictureRecorder();
    const canvas = rec.beginRecording([0, 0, WINDOW_WIDTH, WINDOW_HEIGHT]);

    const path = new GL.CkPath();
    path.moveTo(pts[0][0], pts[0][1]);
    for (let i = 1; i < pts.length; i += 3) {
        path.cubicTo(pts[i][0], pts[i][1], pts[i + 1][0], pts[i + 1][1], pts[i + 2][0], pts[i + 2][1]);
    }

    draw(canvas, path);

    const paint = new GL.CkPaint();
    paint.setColor4f([1, 0.753, 0.796, 1]);
    paint.setAntiAlias(true);
    paint.setStyle(GL.Constants.PAINT_STYLE_FILL);
    for (const p of pts) {
        canvas.drawCircle(p[0], p[1], 4, paint);
    }

    const pict = rec.finishRecordingAsPicture();
    const scene = new GL.SceneBuilder(WINDOW_WIDTH, WINDOW_HEIGHT)
        .pushOffset(0, 0)
        .addPicture(pict, true, 0, 0)
        .build();

    ctx.surface.contentAggregator.update(scene).then(() => {
        scene.dispose();
    });
}

function pointAreaHitTest(point: GL.CkPoint, cursorPos: GL.CkPoint): boolean {
    const x = cursorPos[0], y = cursorPos[1];
    const u = point[0], v = point[1];
    return ((x - u) * (x - u) + (y - v) * (y - v) - 16) < 0;
}

async function main(): Promise<void> {
    const ctx = await setupWindowContext();

    const pts: GL.CkPoint[] = [
        [100, 200],
        [100, 100],
        [300, 0],
        [400, 100],
        [500, 200],
        [600, 300],
        [700, 200],
        [800, 100],
        [900, 100],
        [900, 200]
    ];

    render(ctx, pts);

    let hitPointIdx = -1;
    let cursorX = 0, cursorY = 0;

    ctx.surface.addListener('pointer-motion', (x: number, y: number) => {
        cursorX = x;
        cursorY = y;
        if (hitPointIdx >= 0) {
            pts[hitPointIdx] = [x, y];
            render(ctx, pts);
        }
    });

    ctx.surface.addListener('pointer-button', (button: GL.PointerButton, pressed: boolean) => {
        if (button != GL.Constants.POINTER_BUTTON_LEFT) {
            return;
        }
        if (pressed) {
            for (let i = 0; i < pts.length; i++) {
                if (pointAreaHitTest(pts[i], [cursorX, cursorY])) {
                    hitPointIdx = i;
                    break;
                }
            }
        } else {
            hitPointIdx = -1;
        }
    });

    ctx.surface.addOnceListener('close', () => {
        ctx.surface.close();
    });

    ctx.surface.addOnceListener('closed', () => {
        ctx.display.close();
    });
}

await main();
