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

const WINDOW_WIDTH = 1000;
const WINDOW_HEIGHT = 300;

const presentThread = await GL.PresentThread.Start();
let display = await presentThread.createDisplay();
let surface = await display.createHWComposeSurface(WINDOW_WIDTH, WINDOW_HEIGHT);

surface.addOnceListener('closed', () => {
    display.close();
});

display.addOnceListener('closed', () => {
    presentThread.dispose();
});
surface.setTitle('PathMeasure');

let blender = await surface.createBlender();

const path = new GL.CkPath();
path.moveTo(100, 200);
path.cubicTo(100, 100, 300, 0, 400, 100);
path.cubicTo(500, 200, 600, 300, 700, 200);
path.cubicTo(800, 100, 900, 100, 900, 100);

const paint = new GL.CkPaint();
paint.setStyle(GL.Constants.PAINT_STYLE_STROKE);
paint.setColor4f([0, 0, 1, 1]);
paint.setStrokeWidth(2);
paint.setAntiAlias(true);

const pointPaint = new GL.CkPaint();
pointPaint.setStyle(GL.Constants.PAINT_STYLE_FILL);
pointPaint.setColor4f([0, 0, 0, 1]);
pointPaint.setAntiAlias(true);

const tanPaint = new GL.CkPaint();
tanPaint.setStyle(GL.Constants.PAINT_STYLE_STROKE);
tanPaint.setColor4f([1, 0, 0, 1]);
tanPaint.setStrokeWidth(2);
tanPaint.setAntiAlias(true);

const pathMeasure = GL.CkPathMeasure.Make(path, false, 1);

let t = 0;
const k = 100;

function render(): void {
    const recorder = new GL.CkPictureRecorder();
    const canvas = recorder.beginRecording([0, 0, WINDOW_WIDTH, WINDOW_HEIGHT]);

    canvas.clear([1, 1, 1, 1]);
    canvas.drawPath(path, paint);

    const {position, tangent} = pathMeasure.getPositionTangent(pathMeasure.getLength() * t);
    canvas.drawLine(
        [
            position[0] + k * tangent[0],
            position[1] + k * tangent[1]
        ],
        [
            position[0] - k * tangent[0],
            position[1] - k * tangent[1]
        ],
        tanPaint
    );
    canvas.drawCircle(position[0], position[1], 4, pointPaint);

    const pict = recorder.finishRecordingAsPicture();
    const scene = new GL.SceneBuilder(WINDOW_WIDTH, WINDOW_HEIGHT)
        .pushOffset(0, 0)
        .addPicture(pict, true, 0, 0)
        .build();

    blender.update(scene).then(() => { scene.dispose(); });

    t = (t + 0.001) % 1;
}

surface.addListener('frame', render);

surface.addOnceListener('close', () => {
    surface.removeListener('frame', render);
    blender.dispose();
    surface.close();
});

render();
