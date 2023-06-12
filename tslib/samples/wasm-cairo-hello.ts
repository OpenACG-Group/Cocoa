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

//! [Sample: wasm/cairo]
//! Description: Use Cairo graphics library built into WebAssembly to
//!              render directly in memory.

import * as pixenc from 'pixencoder';
import * as std from 'core';
import * as gl from 'glamor';
import * as Cairo from "../wasm/cairo/lib/cairo";

// Geometry size of the canvas
const WIDTH = 256, HEIGHT = 256;

const BYTES_PER_PIXEL = 4;

// Load the compiled WASM file and its glue JavaScript code.
// The provided path is relative to the Cocoa's working directory.
// This function returns an instance of Cairo library module.
const lib = await Cairo.CairoLoad('../../out/cairo.wasm');

// `malloc` function has the same semantic to what it is in the
// C/C++ environment, which is to allocate some memory on the WASM
// heap and return a pointer of the allocated memory.
// WASM heap memory is allocated when the WASM module is instantiated
// by JavaScript, and consequently JavaScript can read/write the WASM
// heap without restrictions.
// We allocate a buffer to let Cairo write the rasterization result into
// it, then we can get the rasterized image by reading the buffer.
const surfaceMemory = lib.malloc(Uint8Array, WIDTH * HEIGHT * BYTES_PER_PIXEL);
const surface = lib.surface_create_image(
    WIDTH, HEIGHT,
    surfaceMemory,
    lib.Format.ARGB32,
    WIDTH * BYTES_PER_PIXEL
);
const cairo = lib.cairo_create(surface);

// Draw something

cairo.set_source_rgba(1, 1, 1, 1);
cairo.paint();

const cx = WIDTH / 2, cy = HEIGHT / 2;
const R = 120;
const angle1 = Math.PI / 4;
const angle2 = Math.PI;

cairo.set_source_rgba(0, 0, 0, 1);
cairo.set_line_width(10);
cairo.arc(cx, cy, R, angle1, angle2);
cairo.stroke();

cairo.set_source_rgba(1, 0.2, 0.2, 0.6);
cairo.set_line_width(6);

cairo.arc(cx, cy, 10, 0, 2 * Math.PI);
cairo.fill();

cairo.arc(cx, cy, R, angle1, angle1);
cairo.line_to(cx, cy);
cairo.arc(cx, cy, R, angle2, angle2);
cairo.line_to(cx, cy);
cairo.stroke();

// As the memory management of WASM objects should not rely
// on the garbage collector of JavaScript, we must delete
// objects manually. Otherwise, WASM will run out of its heap memory.
cairo.delete();
surface.delete();

// Encode pixels
const encoded = pixenc.WebpEncoder.EncodeMemory(
    {
        colorType: gl.Constants.COLOR_TYPE_BGRA8888,
        alphaType: gl.Constants.ALPHA_TYPE_UNPREMULTIPLIED,
        colorSpace: gl.Constants.COLOR_SPACE_SRGB,
        width: WIDTH,
        height: HEIGHT
    },
    surfaceMemory.toTypedArray() as Uint8Array,
    WIDTH * BYTES_PER_PIXEL,
    {
        compression: pixenc.Constants.WEBP_ENCODER_COMPRESSION_LOSSLESS
    }
);

// Allocated heap memory should be freed as soon as they
// will not be used anymore.
// Note that `free()` function will not actually release the allocated
// memory, because the WASM heap memory should keep available during
// the whole lifetime of the WASM instance.
// JavaScript ArrayBuffers gotten from `MallocMemory.toTypedArray()`
// or `MallocMemory.subarray()` can still be accessed safely, but they
// will become dirty buffers after calling `free()`.
lib.free(surfaceMemory);

// Just write the encoded picture into filesystem
std.File.WriteFileSync(
    './wasm-cairo-hello.webp',
    std.Buffer.MakeFromAdoptBuffer(new Uint8Array(encoded))
);
