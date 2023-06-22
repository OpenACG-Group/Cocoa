![Project Logo](./assets/project_logo.svg)
# JavaScript Engine for 2D Rendering on Linux
This project belongs to [OpenACG Group](https://github.com/OpenACG-Group).

わたしわ、高性能ですから！

[Documentations](https://openacg-group.github.io/)

## Motivation & Introduction
Cocoa is a project inspired by
[*ATRI -My Dear Moments-*](https://atri-mdm.com/),
initially aiming to improve the VN experience on the Linux platform and provide
a framework for those who are interested in VN creation.  Although Cocoa is initially
created for VN, it is completely a general-purposed 2D rendering framework that can
fit other similar situations.

The project name **Cocoa** comes from an anime called
[_Is the Order a Rabbit?_](https://www.gochiusa.com/)
which has a heroine named _Kokoa Hoto_. So its pronunciation is actually follows the Japanese
Katakana ココア. But it also doesn't matter much if you pronounce it in English way.

TypeScript is the official programming language of Cocoa. Cocoa itself can be treated
as a JavaScript engine, which is written in C++17.
Rendering framework part of Cocoa is mostly built by native C++,
while the visual novel framework part is completely built by TypeScript.

Cocoa is still being developed and haven't been ready for commercial use.
Issues / Pull requests are welcome.

## Platform
A typical feature of GNU/Linux platform is that there are usually more than one
technique/solution to solve the same problem. For example, both PipeWire and
PulseAudio are designed to be the audio backend on Linux. Some solutions are too
old (but they are usually more stable and compatible, like X11 vs Wayland) and
Cocoa **does not and will not** support them.

Generally, Cocoa always supports the newer technique when we have the choice,
and there is a table showing what are or aren't supported:

| Feature               | Support     | Not support |
|-----------------------|-------------|-------------|
| Display Server        | Wayland     | X11, Mir    |
| Graphics Library      | Vulkan      | OpenGL      |
| Audio Server          | PipeWire    | PulseAudio  |
| Video Decoding Accel. | VAAPI (DRM) | VDPAU       |

* For video decoding acceleration, Vulkan may be supported in the future.

## Features

### JavaScript Engine
* ECMAScript 6 support, script files are treated as ES6 modules
* Synthetic modules: import native language bindings by `import` keyword directly
* Asynchronous filesystem API
* V8 inspector on WebSocket: debug JavaScript with VSCode
* WebAssembly support: run WASM modules compiled by Emscripten

### Rendering (_glamor_ module)
* Skia-like API
* Onscreen and offscreen rendering targets
* Wayland support
* Raster (CPU) and Vulkan (GPU) rendering backends
* Asynchronous onscreen rasterization
* LayerTree-based onscreen rendering
* PNG, JPEG, Webp encoding and decoding

### Other graphical features
* SVG support (draw SVGs or use SVG as a rendering target)
* Text layout
* Lottie animation support
* Computer vision by OpenCV WASM

### Multimedia (_utau_ module)
* Multimedia decoding based on FFmpeg
* Hardware-accelerated (VA-API) video decoding
* Multimedia DAG filtering (some filters are hardware-accelerated)
* PipeWire
* Multimedia representation dispatcher
* Video buffers can be exported as _glamor_ texture

## Building
See [documentation](https://openacg-group.github.io) for more details.

## WebAssembly
WebAssembly is an experimentally supported feature. WASM module compiled by Emscripten
can be loaded by `//typescript/wasm/wasm-loader-polyfill.ts`, and WASM modules
known to run correctly on Cocoa can be find in `//typescript/wasm`, like Cairo and OpenCV.
Every WASM module in that directory has a tutorial `README.md` which shows you how
to compile it and make it run correctly on Cocoa.

Here is an simple example to load an Emscripten compiled module:

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
