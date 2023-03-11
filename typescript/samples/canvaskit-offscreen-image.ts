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

import * as CanvasKit from 'internal://canvaskit';
import * as std from 'core';

const canvaskit = CanvasKit.canvaskit;

function star(): CanvasKit.Path {
    const R = 60.0, C = 128.0;
    let path = new canvaskit.Path();
    path.moveTo(C + R, C);
    for (let i = 0; i < 15; i++) {
        let a = 0.44879895 * i;
        let r = R + R * (i % 2);
        path.lineTo(C + r * Math.cos(a), C + r * Math.sin(a));
    }
    return path;
}

function draw(canvas: CanvasKit.Canvas): void {
    let paint = new CanvasKit.canvaskit.Paint();
    paint.setPathEffect(CanvasKit.canvaskit.PathEffect.MakeDiscrete(10, 4, 12));
    
    let packedColors = new Float32Array([0.26, 0.52, 0.96, 1,
                                         0.06, 0.62, 0.35, 1]);
    let shader = canvaskit.Shader.MakeLinearGradient([0, 0], [256, 256],
                                                     packedColors,
                                                     null,
                                                     canvaskit.TileMode.Clamp);
    paint.setShader(shader);
    paint.setAntiAlias(true);

    canvas.clear(canvaskit.WHITE);
    canvas.drawPath(star(), paint);
}

let surface = canvaskit.MakeSurface(256, 256);
draw(surface.getCanvas());

let buffer = std.Buffer.MakeFromAdoptBuffer(surface.makeImageSnapshot().encodeToBytes());

let file = await std.File.Open('./output.png', std.File.O_RDWR | std.File.O_CREAT, 0o644);
file.write(buffer, 0, buffer.length, 0);
