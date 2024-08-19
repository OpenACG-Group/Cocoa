![Project Logo](./assets/project_logo.svg)
# JavaScript Engine for 2D Rendering on Linux
This project belongs to [OpenACG Group](https://github.com/OpenACG-Group).

わたしわ、高性能ですから！

[Documentations](https://openacg-group.github.io/)

## Introduction
Basically another version of SDL library, but with many extensions and for JavaScript.

The project name **Cocoa** comes from an anime called
[_Is the Order a Rabbit?_](https://www.gochiusa.com/)
which has a heroine named _Kokoa Hoto_. So its pronunciation is actually follows the Japanese
Katakana ココア. But it also doesn't matter much if you pronounce it in English way.

Cocoa is still being developed and haven't been ready for practical use.
Issues / Pull requests are welcome.

## Example
Here is a simple example to create a window and draw a rectangle.

```javascript
// `renderer` module provides canvas API for drawing geometries
import * as renderer from 'renderer';
// `present` module provides interface to manipulate windows and show contents on it
import * as present from 'present';

// A present thread dispatches the process of onscreen rendering
const thread = present.PresentThread.Start();
// A `Display` object is a standalone connection to the current system display server.
// I.e. it connects to the Wayland compositor such as sway, kwin, mutter, etc.
// It is asynchronous as the operation is sent to present thread to execute.
const display = await thread.createDisplay();

// Create a toplevel 512x512 Wayland surface, which represents a visible window.
// `surface.contentAggregator` manages onscreen rendering of the window.
const surface = await display.createSurface(512, 512, {
    // Enable hardware acceleration for this window
    enableGpuPipeline: true
});

// Add event listeners
surface.addListener('close', () => {
    // Close the window itself first
    surface.close().then(() => {
        // We can close the display server connection because we have no other windows.
        return display.close();
    }).then(() => {
        // Finally we terminate the present thread, then the main event loop should exit.
        thread.dispose();
    });
});


// Preparations have been done, and we can start to draw something.

// First, create a PictureRecorder to record our drawing commands:
const recorder = new renderer.PictureRecorder();

// Begin recording and get a canvas object. We will emit our commands using canvas API.
recorder.beginRecording(renderer.Rect.MakeWH(512, 512));
const canvas = recorder.getRecordingCanvas();

// DRAW: clear the whole canvas
canvas.clear([1, 1, 1, 1]);

// DRAW: draw a rectangle
const paint = new renderer.Paint();
paint.color = 0xff66ccff;
paint.antiAlias = true;
paint.style = renderer.Style.Fill;
canvas.drawRect(renderer.Rect.MakeXYWH(50, 50, 200, 200), paint);

// Finish our recording to get a Picture object where the commands are stored.
const picture = recorder.finishRecordingAsPicture();

// Construct a layer tree and add our Picture object into it as a node.
const layers = new present.SceneBuilder(renderer.Rect.MakeWH(512, 512))
    .addPicture(picture, true)
    .build();

// Commit the layer tree. Layer tree we commited will be rasterized on the present
// thread, and be presented when the next VSync signal arrives.
await surface.contentAggregator.update(layers);
```

## Platform
Cocoa **ONLY** supports GNU/Linux platform, with Wayland, Vulkan and PipeWire support.

| Feature               | Backend  |
|-----------------------|----------|
| Display Server        | Wayland  |
| Rendering             | Vulkan   |
| Audio                 | PipeWire |
| Video Decoding Accel. | Vulkan   |

## Features

### JavaScript Engine
* ECMAScript 6 support, script files are treated as ES6 modules
* Synthetic modules: import native language bindings by `import` keyword directly
* Asynchronous filesystem API
* V8 inspector on WebSocket: debug JavaScript with VSCode
* WebAssembly support: run WASM modules compiled by Emscripten

### Rendering
* Skia-like API
* Onscreen and offscreen rendering targets
* Wayland support
* Vulkan support
* Asynchronous onscreen rasterization
* LayerTree-based onscreen rendering

### Other graphical features
* Image encoding and decoding

### Multimedia
* Multimedia decoding based on FFmpeg
* Vulkan acceleration
* Frame filtering (libavfilter)
* PipeWire
* Media dispatcher (for video playing)

## Building

Clone the repository, and fetch dependencies:
```shell
$ ./script/deptools.py build libuv jsoncpp libwebsockets fmt libyuv skia v8 ffmpeg
```

Then build with CMake, using Clang toolchain:
```shell
$ mkdir -p out && cd out
$ cmake -G Ninja -DCMAKE_C_COMPILER="clang" -DCMAKE_CXX_COMPILER="clang++" .
$ ninja
```

Run a JavaScript file:
```shell
$ ./Cocoa /path/to/the/script.js
```

See [documentation](https://openacg-group.github.io) for more details.

## TypeScript
Cocoa provides a simple TypeScript library, including `.d.ts` files of native modules.

To use it, extend the configuration file `//typescript/tsconfig.base.json`.
An example configuration `tsconfig.json`:

```json
{
    "extends": "/path/to/cocoa/typescript/tsconfig.base.json",
    "files": [
        "your-script-file.ts"
    ]
}
```

## WebAssembly
WebAssembly is an experimentally supported feature. WASM module compiled by Emscripten
can be loaded by `//typescript/wasm/wasm-loader-polyfill.ts`, and WASM modules
known to run correctly on Cocoa can be find in `//typescript/wasm`, like Cairo and OpenCV.
Every WASM module in that directory has a tutorial `README.md` which shows you how
to compile it and make it run correctly on Cocoa.

Here is a simple example to load an Emscripten compiled module:

```typescript
// `wasm-loader-polyfill.ts` is at `//third_party/typescript/wasm/wasm-loader-polyfill.ts`
import { LoadFromFile } from "./wasm-loader-polyfill";

const module = await LoadFromFile('/path/to/module.wasm', '/path/to/module.js');
```

## Third Parties
Cocoa depends on many opensource projects and thanks to them give us a more convenient way
to develop Cocoa without suffering.

* [Google Skia - New BSD License](https://skia.org)
* [Google V8 - BSD License](https://v8.dev)
* [Vulkan - Apache License 2.0](https://www.vulkan.org)
* [FFmpeg - LGPL v2.1+ License](https://ffmpeg.org)
* [libuv - MIT License](https://libuv.org)
* ... and other many dependencies
