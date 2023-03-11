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

import * as std from 'core';
import * as GL from 'glamor';

const surface = GL.CkSurface.MakeRaster({
    colorType: GL.Constants.COLOR_TYPE_BGRA8888,
    alphaType: GL.Constants.ALPHA_TYPE_OPAQUE,
    colorSpace: GL.Constants.COLOR_SPACE_SRGB,
    width: 256,
    height: 256
});

const canvas = surface.getCanvas();

// Source image: 64x64 Chessboard, PNG format (encoded from //assets/chessboard_64x64.png)
const source = await GL.CkImage.MakeFromEncodedData(std.Buffer.MakeFromBase64(
    'iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAIAAAAlC+aJAAAAfklEQVR4nOzZwQkDMQwAwTi4/5ad' +
    'GgQ51gczbyNY/BFof4bOOaP3a61H539Hry8koCagJqAmoCagJqAmoLZv2++n81//AwJqAmoCagJq' +
    'AmoCagJqs+XbfeD/BNQE1ATUBNQE1ATUBNTcB2oCagJqAmoCagJqAmoCar8AAAD//zsDGIB3vGLE' +
    'AAAAAElFTkSuQmCC'
));

const paint = new GL.CkPaint();
paint.setShader(source.makeShader(GL.Constants.TILE_MODE_CLAMP, GL.Constants.TILE_MODE_CLAMP,
                                  GL.Constants.SAMPLING_FILTER_LINEAR, null));
paint.setAntiAlias(true);

canvas.clear([1, 1, 1, 1]);
canvas.scale(30, 30);
canvas.drawPatch([[3, 1], [4, 2], [5, 1], [7, 3],
                  [6, 4], [7, 5], [5, 7],
                  [4, 6], [3, 7], [1, 5],
                  [2, 4], [1, 3]],
                 null,
                 [[0, 0], [0, 62], [62, 62], [62, 0]],
                 GL.Constants.BLEND_MODE_MODULATE,
                 paint);

const image = surface.makeImageSnapshot(null);
std.File.WriteFileSync('result.png', image.encodeToData(GL.Constants.FORMAT_PNG, 100));
