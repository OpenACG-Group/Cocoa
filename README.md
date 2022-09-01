![Project Logo](./literature/assets/project_logo.svg)
# Cocoa — General-Purposed 2D Rendering Framework for Linux
This project belongs to [OpenACG Group](https://github.com/OpenACG-Group).

わたしわ、高性能ですから！

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
Rendering framework part of Cocoa is mostly built by native C++ and WebAssembly,
while the visual novel framework part is completely built by TypeScript.

Cocoa is still being developed and haven't been ready for commercial use.
Issues / Pull requests are welcome.

## Build Cocoa
Third parties should be built before building Cocoa.
See [Build Third Party (Chinese)](./literature/build_third_party.md) for more details.

Then `cmake` can be used to build Cocoa:
```shell
$ mkdir out
$ cd out
$ cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ ..
$ ninja
```

To package and deploy Cocoa, See [Deploy Cocoa]() for more details.


## Rendering Framework
After building Cocoa successfully, you can run a simple example which can play the lottie
animations.

More details about how to run this example and explore Cocoa are available
in the [quick tutorial (Chinese)](./literature/quick_tutorial.md).

TypeScript code of that example is like:

```typescript
import * as std from 'core';
import * as GL from 'glamor';
import * as CanvasKit from 'internal://canvaskit';

const canvaskit = CanvasKit.canvaskit;

// Geometry size of the window
const WINDOW_WIDTH = 533;
const WINDOW_HEIGHT = 500;

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

// Now the preparation steps have been done and we can draw something
// by canvaskit.

let frameSlotConnect = 0;

// Inspired by Skia's official fiddle example:
// https://jsfiddle.skia.org/canvaskit/e77274c30d63645d3bb82fd366991e27c1e1c3df39def04e999b4fcce9f425a2
function playLottie(jsonStr: string) {
    const animation = canvaskit.MakeAnimation(jsonStr);
    const duration = animation.duration() * 1000;
    const bounds = canvaskit.LTRBRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
  
    let firstFrame = new Date().getTime();
  
    function drawFrame() {
        // Create a recording context
        let recorder = new canvaskit.PictureRecorder();
        let canvas = recorder.beginRecording(bounds);
  
        // Seek animation
        let now = new Date().getTime();
        let seek = ((now - firstFrame) / duration) % 1.0;
        animation.seek(seek);

        // Painting
        canvas.clear(canvaskit.WHITE);
        animation.render(canvas, bounds);

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

let jsonBuffer = await std.Buffer.MakeFromFile(std.args[0]);
playLottie(jsonBuffer.toString(std.Buffer.ENCODE_UTF8, jsonBuffer.length));
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

