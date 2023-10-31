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

import * as Cairo from '../wasm/cairo/lib/cairo';
import { DrawToWebp } from "./wasm-cairo-drawtowebp";
import { LoadFromProjectThirdParty } from "../wasm/wasm-loader";

const lib = await LoadFromProjectThirdParty<Cairo.CairoLib>('cairo.wasm', 'cairo.js');

const csi = lib.script_interpreter_create();

let recSurface: Cairo.RecordingSurface = null;

csi.install_hooks({
    surface_create: (content, width, height, uid): Cairo.Surface => {
        recSurface = lib.recording_surface_create(content, [0, 0, width, height]);
        return recSurface;
    }
});

csi.feed_string(`
%!CairoScript

/1sqrt3 0.577359269 def

/T { % cr size -- cr
  exch % size cr
  0 0 m 1 index 0 l 1 index dup 2 div exch //1sqrt3 mul l h

  exch 2 div
  dup 4 ge {
    exch % size/2 cr

    1 index T
    save 1 index 0 translate 1 index T restore
    save 1 index dup 2 div exch //1sqrt3 mul translate 1 index T restore

    exch
  } if
  pop
} bind def

dict
  /width  512 set
  /height 300 set
  surface context

1 1 1 set-source-rgb paint

.5 set-line-width

0 300 translate
1 -1 scale

512 T 0 0 0 set-source-rgb stroke
`);

csi.finish();

const [x, y, width, height] = recSurface.get_extents();

await DrawToWebp(
    width, height,
    'wasm-cairo-script.webp',
    (lib, surface, cairo): void => {
        cairo.set_source_surface(recSurface, 0, 0);
        cairo.paint();
    },
    lib
);

recSurface.delete();
