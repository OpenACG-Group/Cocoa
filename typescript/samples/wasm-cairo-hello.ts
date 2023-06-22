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

import * as Cairo from "../wasm/cairo/lib/cairo";
import { DrawToWebp } from "./wasm-cairo-drawtowebp";

// Geometry size of the canvas
const WIDTH = 256, HEIGHT = 256;

await DrawToWebp(WIDTH, HEIGHT, './wasm-cairo-hello.webp', (lib, surface, cairo) => {
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
});
