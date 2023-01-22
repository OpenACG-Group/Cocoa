![Project Logo](./assets/project_logo.svg)
# Cocoa — General-Purposed 2D Rendering Framework for Linux
This project belongs to [OpenACG Group](https://github.com/OpenACG-Group).

わたしわ、高性能ですから！

[Documentations](https://openacg-group.github.io/)

## Motivation & Introduction
Cocoa is a project inspired by
[*ATRI -My Dear Moments-*](https://atri-mdm.com/),
initially aiming to improve the VN experience on the Linux platform and provide
a framework for those who are interested in VN creation.

Although Cocoa is initially created for VN, it is completely a
general-purposed 2D rendering framework that can fit other similar situations.

Cocoa is a type of plant whose bean is the thing from which chocolate is made.
But _Cocoa_ here doesn't mean that. Instead, it comes from an anime called
[_Is the Order a Rabbit?_](https://www.gochiusa.com/)
which has a heroine named _Kokoa Hoto_. So its pronunciation is actually follows the Japanese
Katakana ココア. But it also doesn't matter much if you pronounce it in  English way.

TypeScript is the official programming language of Cocoa. Cocoa itself can be treated
as a JavaScript engine, which is written in C++17.
Rendering framework part of Cocoa is mostly built by native C++,
while the visual novel framework part is completely built by TypeScript.

Cocoa is still being developed and haven't been ready for commercial use.
Issues / Pull requests are welcome.

## Build and use Cocoa
See [documentation](https://openacg-group.github.io) for more details.

## Rendering Framework
After building Cocoa successfully, you can run a simple example which renders a star filled with
linear gradient colors.

TypeScript code of that example is like:

```typescript
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

// Geometry size of the window
const WINDOW_WIDTH = 256;
const WINDOW_HEIGHT = 256;

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

// Now the preparation steps have been done, and we can draw something
// by Skia.

// Create a path. Path is a set of contours which we can stroke along with.
// Closed path can be filled with colors.
function star(): GL.CkPath {
    const R = 60.0, C = 128.0;
    let path = new GL.CkPath();
    path.moveTo(C + R, C);
    for (let i = 0; i < 15; i++) {
        let a = 0.44879895 * i;
        let r = R + R * (i % 2);
        path.lineTo(C + r * Math.cos(a), C + r * Math.sin(a));
    }
    return path;
}

// Draw something on the canvas
function draw(canvas: GL.CkCanvas): void {
    let paint = new GL.CkPaint();
    paint.setPathEffect(GL.CkPathEffect.MakeFromDSL('discrete(10, 4, 12)', {}));

    const shader = GL.CkShader.MakeFromDSL(
        'gradient_linear([0, 0], [256, 256], [[0.26,0.52,0.96,1], [0.06,0.62,0.35,1]], _, %tile)',
        {tile: GL.Constants.TILE_MODE_CLAMP});

    paint.setShader(shader);
    paint.setAntiAlias(true);

    canvas.clear([1, 1, 1, 1]);
    canvas.drawPath(star(), paint);
}

// Record all the drawing operations in a Picture.
// Those drawing operations will be replayed by rasterizer later.
function drawPicture(): GL.CkPicture {
    let recorder = new GL.CkPictureRecorder();
    let canvas = recorder.beginRecording([0, 0, 256, 256]);
    draw(canvas);
    return recorder.finishRecordingAsPicture();
}

// Then build a scene with a single picture layer and submit it to blender.
let scene = new GL.SceneBuilder(WINDOW_WIDTH, WINDOW_HEIGHT)
    .pushOffset(0, 0)
    .addPicture(drawPicture(), false, 0, 0)
    .build();

blender.update(scene).then(() => { scene.dispose(); });
```

## Visual Novel Framework
Cocoa based visual novel framework is still developing and undocumented.

## Tracing APIs
Tracing APIs are designed to trace resources and generate profiling
to help developers and users improve performance of the application,
or discover latent leaking of resources. Cocoa supports two tracing APIs now,
by which you can trace graphic resources and message delivery between rendering
thread and main thread.

To generate a graphic resources report in JSON format, call `TraceGraphicsResources`
function in JavaScript:

```javascript
// import * as GL from 'glamor';

let json = await GL.RenderHost.TraceGraphicsResources();

// `json` is a string in JSON format
```

To trace the message delivery of rendering thread, just add a `--gl-transfer-queue-profile`
option to the commandline of Cocoa.
And Cocoa will generate a JSON file named `transfer-profiling-<pid>.json` in the working directory.

Note that the tracing of message delivery may cause slight loss of performance
and increases CPU time. But the tracing of resources will not have obvious effect
on the performance.

## Third Parties
Cocoa depends on many opensource projects and thanks to them give us a more convenient way
to develop Cocoa without suffering.

* [Google Skia - New BSD License](https://skia.org)
* [Google V8 - BSD License](https://v8.dev)
* [Vulkan - Apache License 2.0](https://www.vulkan.org)
* [FFmpeg - LGPL v2.1+ License](https://ffmpeg.org)
* [libuv - MIT License](https://libuv.org)
* ... and other many dependencies
