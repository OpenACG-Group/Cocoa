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

//! [Sample: wasm/cairo, SkNative]
//! Description: Let SkNative and Cairo share the same bitmap render target

import * as std from 'core';
import * as gl from 'glamor';
import * as pixenc from 'pixencoder';
import * as Cairo from '../wasm/cairo/lib/cairo';

const WIDTH = 512, HEIGHT = 512;
const BYTES_PER_PIXEL = 4;

const lib = await Cairo.CairoLoad('../../out/cairo.wasm');

function drawCairo(memory: Cairo.MallocMemory): void {
    const surface = lib.surface_create_image(
        WIDTH, HEIGHT,
        memory,
        lib.Format.ARGB32,
        WIDTH * BYTES_PER_PIXEL
    );
    const cairo = lib.cairo_create(surface);

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

    cairo.delete();
    surface.delete();
}

function drawSkNative(memory: Cairo.MallocMemory): void {
    const memoryU8Array = memory.toTypedArray() as Uint8Array;
    const surface = gl.CkSurface.MakeSharedPixels(
        {
            colorType: gl.Constants.COLOR_TYPE_BGRA8888,
            alphaType: gl.Constants.ALPHA_TYPE_UNPREMULTIPLIED,
            colorSpace: gl.Constants.COLOR_SPACE_SRGB,
            width: WIDTH,
            height: HEIGHT
        },
        memoryU8Array.byteOffset,
        WIDTH * BYTES_PER_PIXEL,
        memoryU8Array.buffer
    );
    const canvas = surface.getCanvas();
    
    const typeface = gl.defaultFontMgr.matchFamilyStyle(
        'Consolas',
        gl.CkFontStyle.MakeNormal()
    );
    if (typeface == null) {
        std.print('Failed to load font Consolas');
    }
    const font = gl.CkFont.MakeFromSize(typeface, 15);

    const paint = new gl.CkPaint();
    paint.setAntiAlias(true);
    paint.setColor4f([0, 0, 0, 1]);

    canvas.drawTextBlob(
        gl.CkTextBlob.MakeFromText(
            std.Buffer.MakeFromString(
                'This text is drawn by SkNative, and the arc is by Cairo:',
                std.Buffer.ENCODE_UTF8
            ).byteArray,
            font,
            gl.Constants.TEXT_ENCODING_UTF8
        ),
        0, 20,
        paint
    );
}

const surfaceMemory = lib.malloc(Uint8Array, WIDTH * HEIGHT * BYTES_PER_PIXEL);

drawCairo(surfaceMemory);
drawSkNative(surfaceMemory);

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

lib.free(surfaceMemory);

std.File.WriteFileSync(
    './wasm-cairo-sharing-composite.webp',
    std.Buffer.MakeFromAdoptBuffer(new Uint8Array(encoded))
);
