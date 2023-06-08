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

import { Rect } from './rectangle';

/**
 * Compute the minimal rectangle where all the given points are contained.
 *
 * @param   pts Flattened array of points, [x1, y1, x2, y2, ...];
 *              throws an exception if the provided array is invalid.
 *
 * @return  A valid rectangle; or an empty rectangle if no points is provided.
 */
export function ComputeMinimalBoundingBox(pts: ArrayLike<number>): Rect {
    if (pts.length & 1) {
        throw Error('Invalid flattened array of points');
    }

    if (pts.length == 0) {
        return Rect.MakeEmpty();
    }

    let minX = Infinity, minY = Infinity;
    let maxX = -Infinity, maxY = -Infinity;
    for (let i = 0; i < pts.length / 2; i++) {
        const px = pts[2 * i], py = pts[2 * i + 1];
        minX = (px < minX) ? px : minX;
        minY = (py < minY) ? py : minY;
        maxX = (px > maxX) ? px : maxX;
        maxY = (py > maxY) ? py : maxY;
    }

    if (minX >= maxY || minY >= maxY) {
        return Rect.MakeEmpty();
    }

    return Rect.MakeLTRB(minX, minY, maxX, maxY);
}
