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

import * as pixenc from 'pixencoder';
import * as std from 'core';
import * as gl from 'glamor';
import * as Cairo from "../wasm/cairo/lib/cairo";
import { LoadFromProjectThirdParty } from "../wasm/wasm-loader";

const BYTES_PER_PIXEL = 4;

export type DrawCallback = (lib: Cairo.CairoLib, surface: Cairo.Surface, cairo: Cairo.Cairo) => void;

export async function DrawToWebp(width: number, height: number,
                                 file: string, callback: DrawCallback,
                                 preloadLib: Cairo.CairoLib = null): Promise<void>
{
    let lib = preloadLib;
    if (lib == null) {
        lib = await LoadFromProjectThirdParty<Cairo.CairoLib>('cairo.wasm', 'cairo.js');
    }

    // `malloc` function has the same semantic to what it is in the
    // C/C++ environment, which is to allocate some memory on the WASM
    // heap and return a pointer of the allocated memory.
    // WASM heap memory is allocated when the WASM module is instantiated
    // by JavaScript, and consequently JavaScript can read/write the WASM
    // heap without restrictions.
    // We allocate a buffer to let Cairo write the rasterization result into
    // it, then we can get the rasterized image by reading the buffer.
    const surfaceMemory = lib.malloc(Uint8Array, width * height * BYTES_PER_PIXEL);
    const surface = lib.image_surface_create(
        width, height,
        surfaceMemory,
        lib.Format.ARGB32,
        width * BYTES_PER_PIXEL
    );
    const cairo = lib.cairo_create(surface);

    callback(lib, surface, cairo);

    // As the memory management of WASM objects should not rely
    // on the garbage collector of JavaScript, we must delete
    // objects manually. Otherwise, WASM will run out of its heap memory.
    cairo.delete();
    surface.delete();

    const imageInfo = gl.CkImageInfo.MakeSRGB(
        width, height,
        gl.Constants.COLOR_TYPE_BGRA8888,
        gl.Constants.ALPHA_TYPE_UNPREMULTIPLIED
    );

    // Encode pixels
    const encoded = pixenc.WebpEncoder.EncodeMemory(
        imageInfo,
        surfaceMemory.toTypedArray() as Uint8Array,
        width * BYTES_PER_PIXEL,
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
        file,
        std.Buffer.MakeFromAdoptBuffer(new Uint8Array(encoded))
    );
}
