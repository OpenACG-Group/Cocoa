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

import { print } from 'core';
import { CkMatrix } from 'glamor';
import { Mat3x3 } from '../graphics/base/Matrix';
import { Vector2f } from '../graphics/base/Vector';

const N = 1000000;

const m33 = Mat3x3.Identity();
const concat = Mat3x3.Rotate(new Vector2f(0, 0), Math.PI / 15);

let st = getMillisecondTimeCounter();
for (let i = 0; i < N; i++) {
    m33.preConcat(concat);
}
let et = getMillisecondTimeCounter();
print(`Mat3x3 ${et - st}ms\n`);

const mat = CkMatrix.Identity();
// const matConcat = CkMatrix.RotateRad(Math.PI / 15, [0, 0]);
const matConcat = concat.toCkMat3x3Array();
st = getMillisecondTimeCounter();
for (let i = 0; i < N; i++) {
    mat.preConcat(matConcat);
}
et = getMillisecondTimeCounter();
print(`CkMatrix ${et - st}ms\n`);