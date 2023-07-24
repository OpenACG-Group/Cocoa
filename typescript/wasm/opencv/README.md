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
__Note that you can add a `--simd` option to enable the WASM SIMD feature for__
__better performance. However, you may be stuck with some compilation errors related to__
__the deprecated SIMD intrinsics. Read the next section to know how to fix it.__


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

## Fix SIMD compilation errors

All the SIMD compilation errors are from `modules/core/include/opencv2/core/hal/intrin_wasm.hpp`
file, where OpenCV implements an abstraction layer to adapt to various CPUs.
As the SIMD for WASM is still an experimental feature
(see [this proposal](https://github.com/WebAssembly/simd)
and [this article from V8 team](https://v8.dev/features/simd)),
the WASM intrinsic APIs provided by Emscripten are changing with time, and the compilation
errors may occur when you use the newest Emscripten.

The following fix should work fine with __OpenCV 4.x__ and __Emscripten 3.1.41-git__.

Edit the `modules/core/include/opencv2/core/hal/intrin_wasm.hpp` file:
* Find the following content:
```c++
#if (__EMSCRIPTEN_major__ * 1000000 + __EMSCRIPTEN_minor__ * 1000 + __EMSCRIPTEN_tiny__) < (1038046)
<folded code>
#endif // COMPATIBILITY: <1.38.46
```

Replace it with:

```c++
#define wasm_i32x4_trunc_saturate_f32x4 wasm_i32x4_trunc_sat_f32x4
#define wasm_u8x16_sub_saturate wasm_u8x16_sub_sat
#define wasm_u8x16_add_saturate wasm_u8x16_add_sat
#define wasm_u16x8_sub_saturate wasm_u16x8_sub_sat
#define wasm_u16x8_add_saturate wasm_u16x8_add_sat
#define wasm_i16x8_add_saturate wasm_i16x8_add_sat
#define wasm_i16x8_sub_saturate wasm_i16x8_sub_sat
#define wasm_i8x16_add_saturate wasm_i8x16_add_sat
#define wasm_i8x16_sub_saturate wasm_i8x16_sub_sat
#define wasm_i8x16_any_true wasm_v128_any_true
```

* Replace all `wasm_v8x16_shuffle` with `wasm_i8x16_shuffle`
* Find the following content:
```c++
#if (__EMSCRIPTEN_major__ * 1000000 + __EMSCRIPTEN_minor__ * 1000 + __EMSCRIPTEN_tiny__) >= (1039012)
// details: https://github.com/opencv/opencv/issues/18097 ( https://github.com/emscripten-core/emscripten/issues/12018 )
// 1.39.12: https://github.com/emscripten-core/emscripten/commit/cd801d0f110facfd694212a3c8b2ed2ffcd630e2
<folded code>
#else
OPENCV_HAL_IMPL_WASM_BIN_FUNC(v_uint8x16, v_mul_wrap, wasm_i8x16_mul)
OPENCV_HAL_IMPL_WASM_BIN_FUNC(v_int8x16, v_mul_wrap, wasm_i8x16_mul)
#endif 
```
Remove all the lines except `<folded code>`.

* Save the file and try recompiling OpenCV.

## Use OpenCV
[`//typescript/samples/wasm-opencv-fft-illustrator.ts`](../../samples/wasm-opencv-fft-illustrator.ts)
shows how to load and use OpenCV WebAssembly module.
Only after the OpenCV is compiled and the files are copied into `//third_party/wasm-build`,
can this example be run.
