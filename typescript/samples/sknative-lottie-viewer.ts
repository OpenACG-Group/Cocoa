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

if (std.args.length !== 1) {
    throw Error('Provide a lottie file in the arguments list');
}

const presentThread = await GL.PresentThread.Start();
let display = await presentThread.createDisplay();
let surface = await display.createHWComposeSurface(WINDOW_WIDTH, WINDOW_HEIGHT);

surface.addOnceListener('closed', () => {
    display.close();
});

display.addOnceListener('closed', () => {
    presentThread.dispose();
});

surface.setTitle('Lottie Viewer');

const aggregator = surface.contentAggregator;

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
                    .addPicture(picture, false)
                    .build();

        aggregator.update(scene).catch(reason => {
            std.print(`Failed to update: ${reason}\n`);
        });
    }

    surface.addOnceListener('close', () => {
        surface.removeListener('frame', drawFrame);
        surface.close();
    });

    surface.addListener('frame', drawFrame);
    surface.requestNextFrame();
}

playLottie(std.args[0]);
