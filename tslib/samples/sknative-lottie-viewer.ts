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
import * as Lottie from 'lottie';

// Geometry size of the window
const WINDOW_WIDTH = 512;
const WINDOW_HEIGHT = 512;

if (std.args.length != 1) {
    throw Error('Provide a lottie file in the arguments list');
}

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

// Now the preparation steps have been done

let frameSlotConnect = 0;

// Inspired by Skia's official fiddle example:
// https://jsfiddle.skia.org/canvaskit/e77274c30d63645d3bb82fd366991e27c1e1c3df39def04e999b4fcce9f425a2
function playLottie(file: string) {
    const animation = new Lottie.AnimationBuilder(0)
        .makeFromFile(file);

    const bounds: GL.CkArrayXYWHRect = [0, 0, WINDOW_WIDTH, WINDOW_HEIGHT];
  
    let firstFrame = new Date().getTime() / 1000;
  
    function drawFrame() {
        // Create a recording context
        let recorder = new GL.CkPictureRecorder();
        let canvas = recorder.beginRecording(bounds);
  
        // Seek animation
        let now = new Date().getTime() / 1000;
        animation.seekFrameTime((now - firstFrame) % animation.duration);

        // Painting
        canvas.clear([1, 1, 1, 1]);
        animation.render(canvas, bounds, 0);

        const picture = recorder.finishRecordingAsPicture();
        let scene = new GL.SceneBuilder(WINDOW_WIDTH, WINDOW_HEIGHT)
                    .pushOffset(0, 0)
                    .addPicture(picture, false,0, 0)
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

playLottie(std.args[0]);
