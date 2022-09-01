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
import * as canvaskit from '../canvaskit/canvaskit';
import * as canvasmath from '../canvaskit/comath';

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

// Create a resource group where the draw commands will be stored.
// Then create a canvas object associated to that resource group
// with given geometry size.
let resourceGroup = new canvaskit.MemoryResourceGroup();
let canvas = new canvaskit.Canvas(resourceGroup, WINDOW_WIDTH, WINDOW_HEIGHT);

// Start drawing!
canvas.clear(canvaskit.U32ColorFromNorm(1, 1, 1, 1));
canvas.drawImageRect(await GL.CkImage.MakeFromEncodedFile('/home/sora/Pictures/Yukimura Chieri.jpeg'),
                      canvasmath.Rect.MakeXYWH(10, 10, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2));

// Finish current canvas and get a picture of it.
// Picture which contains all the translated draw commands is a snapshot
// of current contents of canvas.
canvas.finish();
let picture = canvas.submit().artifact;
canvas.dispose();

// Then build a scene with a single picture layer and submit it to blender.
let scene = new GL.SceneBuilder(WINDOW_WIDTH, WINDOW_HEIGHT)
                .pushOffset(0, 0)
                .addPicture(picture, 0, 0)
                .build();

blender.update(scene).then(() => { scene.dispose(); });

// Finally, we can schedule a new frame and the submitted scene will be displayed.
surface.requestNextFrame();
