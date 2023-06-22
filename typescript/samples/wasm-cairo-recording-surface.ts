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
//! Description: Use Cairo's recording surface

import { DrawToWebp } from './wasm-cairo-drawtowebp';

const WIDTH = 256, HEIGHT = 256;

await DrawToWebp(WIDTH, HEIGHT, 'wasm-cairo-recording-surface.webp', (lib, surface, cairo) => {
    const recSurface = lib.recording_surface_create(lib.Content.COLOR_ALPHA, null);
    const recCairo = lib.cairo_create(recSurface);
    recCairo.set_source_rgba(0, 1, 1, 1);
    recCairo.paint();
    recCairo.delete();

    cairo.set_source_rgba(1, 1, 1, 1);
    cairo.paint();

    cairo.set_source_surface(recSurface, 0, 0);
    cairo.rectangle(100, 100, 100, 100);
    cairo.clip();
    cairo.paint();

    recSurface.delete();
});
