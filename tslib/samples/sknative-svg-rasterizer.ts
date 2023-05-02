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

import * as SVG from 'svg';
import * as std from 'core';
import * as GL from 'glamor';
import { ResourceProvider } from 'resources';

function main(): void {
    let input: string = null, output: string = null;
    let widthHint = 0, heightHint = 0;

    if (std.args.length == 2) {
        input = std.args[0];
        output = std.args[1];
    } else if (std.args.length == 4) {
        input = std.args[0];
        output = std.args[1];
        widthHint = Number.parseInt(std.args[2]);
        heightHint = Number.parseInt(std.args[3]);
    } else {
        std.print('Options: <input SVG file> <output PNG file> [<width>] [<height>]\n');
        return;
    }

    const rp = ResourceProvider.MakeFile('./', false);
    const document = new SVG.SVGDOMLoader()
        .setResourceProvider(ResourceProvider.MakeDataURIProxy(rp, false))
        .setFontManager(GL.defaultFontMgr)
        .makeFromFile(input);

    const lengthContext = SVG.SVGLengthContext.Make(
        {width: widthHint, height: heightHint},
        SVG.Constants.SVG_LENGTH_DEFAULT_DPI
    );

    let width = widthHint, height = heightHint;
    
    const intrinsicSize = document.intrinsicSize(lengthContext);
    if (intrinsicSize.width != 0 && intrinsicSize.height != 0) {
        // SVG can provide an intrinsic size (width and height are
        // not specified as percentage), then we prefer to use intrinsic size.
        width = intrinsicSize.width;
        height = intrinsicSize.height;
    }

    if (width == 0 || height == 0) {
        std.print('The SVG itself cannot provide an intrinsic size. Try specifying a size hint\n');
        return;
    }

    document.setContainerSize(width, height);

    const surface = GL.CkSurface.MakeRaster({
        alphaType: GL.Constants.ALPHA_TYPE_OPAQUE,
        colorType: GL.Constants.COLOR_TYPE_BGRA8888,
        colorSpace: GL.Constants.COLOR_SPACE_SRGB,
        width: width,
        height: height
    });
    const canvas = surface.getCanvas();

    canvas.clear([1, 1, 1, 1]);
    document.render(canvas);

    std.File.WriteFileSync(output,
        surface.makeImageSnapshot(null)
               .encodeToData(GL.Constants.FORMAT_PNG, 100)
    );
}

main();
