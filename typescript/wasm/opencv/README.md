# Powerful Computer Vision through OpenCV WebAssembly

<div align="center">
<img src="../../../assets/OpenCV_logo_black-2.png" alt="OpenCV Logo"/>
</div>

## Introduction
Cocoa provides the ability of simple image processing through Skia (like `CkImageFilter` interface),
but when it comes to more complicated image processing, Skia cannot handle them, for example,
finding contours or detecting targets in an image. In that case, consider using this OpenCV
WebAssembly module to get the advanced support of computer vision.

## Build OpenCV

OpenCV officially supports using WebAssembly as the building target, and we can just clone the
repository:
```bash
## In the root directory of Cocoa project
$ cd third_party
$ git clone https://github.com/opencv/opencv
$ cd opencv

## Optional: You can choose a latest stable version to checkout
$ git switch -c v4.x 4.x
```

Then make sure that you have installed Emscripten, and run the `platforms/js/build_js.py`
script to configure and build OpenCV:
```bash
$ python ./platforms/js/build_js.py \
         --build_wasm \
         --emscripten_dir /usr/lib/emscripten \
         --disable_single_file \
         build_wasm
```

The path `/usr/lib/emscripten` should be replaced with the path where Emscripten
is installed on your system. `--disable_single_file` option is required to prevent
Emscripten combining JavaScript file and WASM file.

When it is finished, copy necessary files into Cocoa's default directory:
```bash
$ mkdir -p ../wasm-build/bin
$ cp build_wasm/bin/opencv_js.wasm ../wasm-build/bin
$ cp build_wasm/bin/opencv.js ../wasm-build/bin

## Optionally, you can format the JavaScript file:
$ npx js-beautify ../wasm-build/bin/opencv.js -r
```

## Use OpenCV
[`//typescript/samples/wasm-opencv-fft-illustrator.ts`](../../samples/wasm-opencv-fft-illustrator.ts)
shows how to load and use OpenCV WebAssembly module.
Only after the OpenCV is compiled and the files are copied into `//third_party/wasm-build`,
can this example be run.
