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

import * as ui from 'glamor';
import { print } from 'core';
import {SceneBuilder} from "glamor";

class OffscreenSurface {
    private fContext: ui.GpuDirectContext;
    private fSurface: ui.CkSurface;
    private fWaitSem: ui.GpuBinarySemaphore;
    private fSignalSem: ui.GpuBinarySemaphore;

    private fWaitSemId: bigint;
    private fSignalSemId: bigint;
    private fSurfaceId: bigint;

    constructor() {
        this.fContext = ui.GpuDirectContext.Make();
        const imageInfo = ui.CkImageInfo.MakeSRGB(
            400, 400, ui.Constants.COLOR_TYPE_BGRA8888, ui.Constants.ALPHA_TYPE_PREMULTIPLIED);
        this.fSurface = this.fContext.makeRenderTarget(imageInfo, false, 0);
        this.fWaitSem = this.fContext.makeBinarySemaphore(true);
        this.fSignalSem = this.fContext.makeBinarySemaphore(true);

        this.fWaitSemId = -1n;
        this.fSignalSemId = -1n;
        this.fSurfaceId = -1n;
    }

    public async exportToContentAggregator(aggregator: ui.ContentAggregator): Promise<void> {
        this.fWaitSemId = await aggregator.importGpuSemaphoreFd(
            this.fContext.exportSemaphoreFd(this.fWaitSem));
        this.fSignalSemId = await aggregator.importGpuSemaphoreFd(
            this.fContext.exportSemaphoreFd(this.fSignalSem));
        this.fSurfaceId = await aggregator.importGpuCkSurface(
            this.fContext.exportRenderTargetFd(this.fSurface));
    }

    public submitToScene(builder: ui.SceneBuilder): void {
        this.fSurface.flush({ signalSemaphores: [this.fWaitSem] }, true);
        this.fContext.submit(false);
        builder.addGpuSurfaceView(this.fSurfaceId, [0, 0, 400, 400],
            this.fWaitSemId, this.fSignalSemId, null);
    }

    public insertGpuWait(): void {
        this.fSurface.waitOnGpu([this.fSignalSem], false);
    }

    public getCanvas(): ui.CkCanvas {
        return this.fSurface.getCanvas();
    }
}

async function main(): Promise<void> {
    const thread = await ui.PresentThread.Start();
    const display = await thread.createDisplay();
    const surface = await display.createHWComposeSurface(400, 400);

    const offscreen = new OffscreenSurface();
    await offscreen.exportToContentAggregator(surface.contentAggregator);

    let pending = false;
    let updateResult: ui.UpdateResult = ui.Constants.UPDATE_RESULT_FRAME_DROPPED;

    surface.addListener('pointer-motion', (x: number, y: number) => {
        if (pending) {
            return;
        }

        pending = true;

        if (updateResult == ui.Constants.UPDATE_RESULT_SUCCESS) {
            offscreen.insertGpuWait();
        }
        let canvas = offscreen.getCanvas();

        canvas.clear([0, 1, 1, 1]);
        const paint = new ui.CkPaint();
        paint.setStyle(ui.Constants.PAINT_STYLE_FILL);
        paint.setColor4f([1, 1, 1, 1]);
        canvas.drawRect([x, y, 10, 10], paint);

        const sceneBuilder = new SceneBuilder(400, 400)
            .pushOffset(0, 0);
        offscreen.submitToScene(sceneBuilder);

        surface.contentAggregator.update(sceneBuilder.build()).then((result) => {
            pending = false;
            updateResult = result;
        });
    });

    surface.addListener('close', () => {
        surface.close().then(() => {
            return display.close();
        }).then(() => {
            thread.dispose();
        });
    });

    print('done\n');
}

await main();