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
import * as Lottie from 'lottie';

const WINDOW_WIDTH = 512;
const WINDOW_HEIGHT = 512;

if (std.args.length != 1) {
    throw Error('Provide a lottie file in the arguments list');
}

GL.RenderHost.Initialize({
    name: 'Example',
    major: 1,
    minor: 0,
    patch: 0
});

let display = await GL.RenderHost.Connect();
let surface = await display.createHWComposeSurface(WINDOW_WIDTH, WINDOW_HEIGHT);

surface.connect('closed', () => {
    display.close();
});

display.connect('closed', GL.RenderHost.Dispose);

surface.setTitle('Lottie Viewer');

let blender = await surface.createBlender();

let frameSlotConnect = 0;
function playLottie(file: string) {
    const animation = new Lottie.AnimationBuilder(0)
        .makeFromFile(file);

    const bounds: GL.CkArrayXYWHRect = [0, 0, WINDOW_WIDTH, WINDOW_HEIGHT];
  
    let firstFrame = new Date().getTime() / 1000;
  
    function drawFrame() {
        let recorder = new GL.CkPictureRecorder();
        let canvas = recorder.beginRecording(bounds);
  
        let now = new Date().getTime() / 1000;
        animation.seekFrameTime((now - firstFrame) % animation.duration);

        canvas.clear([1, 1, 1, 1]);
        animation.render(canvas, bounds, 0);

        const picture = recorder.finishRecordingAsPicture();
        let scene = new GL.SceneBuilder(WINDOW_WIDTH, WINDOW_HEIGHT)
                    .pushOffset(0, 0)
                    .addPicture(picture, false,0, 0)
                    .build();

        blender.update(scene).then(() => { scene.dispose(); });
    }

    frameSlotConnect = surface.connect('frame', drawFrame);
    surface.requestNextFrame();
}

surface.connect('close', () => {
    surface.disconnect(frameSlotConnect);
    blender.dispose();
    surface.close();
});

playLottie(std.args[0]);