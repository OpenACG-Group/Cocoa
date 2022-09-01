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
const WINDOW_WIDTH = 512;
const WINDOW_HEIGHT = 512;

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

// Now the preparation steps have been done and we can draw something
// by canvaskit.

let frameSlotConnect = 0;

// Inspired by Skia's official fiddle example:
// https://jsfiddle.skia.org/canvaskit/ac0574825f9e517f2dfa8e822126ee75b005e8156c3de4a95d4ffd17ab6ca57b
function run() {
    const program = `
    uniform float rad_scale;
    uniform float2 in_center;
    uniform float4 in_colors0;
    uniform float4 in_colors1;
    
    half4 main(float2 p) {
        float2 pp = p - in_center;
        float radius = sqrt(dot(pp, pp));
        radius = sqrt(radius);
        float angle = atan(pp.y / pp.x);
        float t = (angle + 3.1415926/2) / (3.1415926);
        t += radius * rad_scale;
        t = fract(t);
        return half4(mix(in_colors0, in_colors1, t));
    }`;
    
    const effect = canvaskit.RuntimeEffect.Make(program);
    const paint = new canvaskit.Paint();

    function drawFrame() {
        let bounds = canvaskit.LTRBRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

        // Create a recording context
        let recorder = new canvaskit.PictureRecorder();
        let canvas = recorder.beginRecording(bounds);
 
        // Painting
        canvas.clear(canvaskit.WHITE);
        const shader = effect.makeShader([Math.sin(Date.now() / 2000) / 5,
                                          256, 256,
                                          1, 0, 0, 1,
                                          0, 1, 0, 1]);
        
        paint.setShader(shader);
        canvas.drawRect(bounds, paint);
        shader.delete();
 
        // Post processing:
        // Get the generated picture object by `recorder.finishRecordingAsPicture`
        // (see Skia's documentations). To convert it to the `GL.CkPicture` object,
        // we should serialize it to a binary buffer and then deserialize it
        // by `GL.CkPicture.MakeFromData` function.
        let buffer = std.Buffer.MakeFromAdoptBuffer(recorder.finishRecordingAsPicture().serialize());
        let scene = new GL.SceneBuilder(WINDOW_WIDTH, WINDOW_HEIGHT)
                    .pushOffset(0, 0)
                    .addPicture(GL.CkPicture.MakeFromData(buffer), 0, 0)
                    .build();
 
        // Finally, submit the constructed `Scene` object to blender, which will rasterize and
        // represent it on the surface (window).
        // `scene` must be disposed to release acquired resources after the updating is
        // completed. `update` implicitly requests next frame, so there is no need to
        // and should not call `surface.requestNextFrame` explicitly.
        // It makes the frame scheduler into a wrong state that call `surface.requestNextFrame` for
        // multiple times before next frame arrives.
        blender.update(scene).then(() => { scene.dispose(); });
    }
 
    frameSlotConnect = surface.connect('frame', drawFrame);
    surface.requestNextFrame();
}

// Register a close event callback
surface.connect('close', () => {
    // When the window is notified by system that it should be closed,
    // this callback function will be fired.
    // We just print a message and close the window in this simplest
    // application.
    std.print('Window was closed by user...\n');

    surface.disconnect(frameSlotConnect);

    // The invocation of `close` only sends a message to rendering thread,
    // which means it will NOT make the window closed immediately.
    // This request will be processed asynchronously in event loop.
    blender.dispose();
    surface.close();
});

run();
