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
import * as CanvasKit from 'internal://canvaskit';

const canvaskit = CanvasKit.canvaskit;

// Geometry size of the window
const WINDOW_WIDTH = 1280;
const WINDOW_HEIGHT = 720;

// First of all, initialize Glamor context by providing the name 
// and version of your application for `Initialize` function.
GL.RenderHost.Initialize({
    name: 'Example',
    major: 1,
    minor: 0,
    patch: 0
});

// Now the rendering thread has been started and we can do
// remote method calls based on promise.

// Connect to default display server (Usually Wayland on Linux)
let display = await GL.RenderHost.Connect();

// Create a window with hardware-accelerated rasterization backend
let surface = await display.createHWComposeSurface(WINDOW_WIDTH, WINDOW_HEIGHT);

// Close the display attached to the window when the window is closed.
// We can do this because `surface` is the unique window in this application.
surface.connect('closed', () => {
    display.close();
});

// Dispose the Glamor context when the display is closed
display.connect('closed', GL.RenderHost.Dispose);

// Set a reasonable title for our window
surface.setTitle('White Eternity');

// As the final step of window system related initialization,
// we create a blender so that we can render things through it.
let blender = await surface.createBlender();

// Register a close event callback
surface.connect('close', () => {
    // When the window is notified by system that it should be closed,
    // this callback function will be fired.
    // We just print a message and close the window in this simplest
    // application.
    std.print('Window was closed by user...\n');

    // The invocation of `close` only sends a message to rendering thread,
    // which means it will NOT make the window closed immediately.
    // This request will be processed asynchronously in event loop.
    blender.dispose();
    surface.close();
});

// Now the preparation steps have been done and we can draw something
// by canvaskit.

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

function drawPicture(): CanvasKit.SkPicture {
    let recorder = new canvaskit.PictureRecorder();
    let canvas = recorder.beginRecording(canvaskit.XYWHRect(0, 0, 256, 256));
    draw(canvas);
    return recorder.finishRecordingAsPicture();
}

let buffer = std.Buffer.MakeFromAdoptBuffer(drawPicture().serialize());

// Then build a scene with a single picture layer and submit it to blender.
let scene = new GL.SceneBuilder(WINDOW_WIDTH, WINDOW_HEIGHT)
                .pushOffset(0, 0)
                .addPicture(GL.CkPicture.MakeFromData(buffer), 0, 0)
                .build();

blender.update(scene).then(() => { scene.dispose(); });

// Finally, we can schedule a new frame and the submitted scene will be displayed.
surface.requestNextFrame();
