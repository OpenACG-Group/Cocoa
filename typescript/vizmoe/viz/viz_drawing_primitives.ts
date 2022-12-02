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

import * as CanvasKit from 'internal://canvaskit';

const Canvas = CanvasKit.canvaskit;

enum DrawOpVerbs {
    SAVE,
    RESTORE,

    CLIP,
    TRANSFORM,

    STROKE,
    FILL
}

const g_draw_executors = [
    { verb: DrawOpVerbs.SAVE, executor: function(canvas: CanvasKit.Canvas) {
        canvas.save();
    }},

    { verb: DrawOpVerbs.RESTORE, executor: function(canvas: CanvasKit.Canvas) {
        canvas.restore();
    }},

    { verb: DrawOpVerbs.STROKE, executor: function(canvas: CanvasKit.Canvas) {
        
    }}
];