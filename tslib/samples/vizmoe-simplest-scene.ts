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

import * as GL from 'glamor';
import * as std from 'core';
import { DrawContext, PaintEvent } from "../vizmoe/render/draw-context";
import {DrawContextSubmitter, SubmittedEvent} from "../vizmoe/render/draw-context-submitter";
import { Rect } from "../vizmoe/render/rectangle";
import { Vector2f } from "../vizmoe/render/vector";
import * as Geometry from '../vizmoe/render/geometry';
import * as RenderNode from '../vizmoe/render/render-node';
import * as Color from '../vizmoe/render/color';

const WIN_W = 800, WIN_H = 600;

class RenderContext {
    private readonly fRootRenderNode: RenderNode.CompositeRenderNode;
    private readonly fPaintNode: RenderNode.PaintRenderNode;
    private readonly fWidth: number;
    private readonly fHeight: number;
    private readonly fTriVert: Float32Array;

    constructor(width: number, height: number) {
        this.fWidth = width;
        this.fHeight = height;

        this.fRootRenderNode = new RenderNode.CompositeRenderNode();
        this.fPaintNode = new RenderNode.PaintRenderNode();

        const clearNode = new RenderNode.PaintRenderNode();
        clearNode.update(Rect.MakeWH(width, height), C => C.clear([1, 1, 1, 1]));
        this.fRootRenderNode.appendChild(clearNode);

        this.fRootRenderNode.appendChild(this.fPaintNode);

        this.fTriVert = new Float32Array([
            width / 2, height / 3,
            width / 3, height * 2 / 3,
            width * 2 / 3, height * 2 / 3
        ]);
    }

    public render(submitter: DrawContextSubmitter): void {
        const box = Geometry.ComputeMinimalBoundingBox(this.fTriVert);

        this.fPaintNode.update(box, (canvas) => {
            const u = this.fTriVert;
            const v = [
                u[0] - box.x, u[1] - box.y,
                u[2] - box.x, u[3] - box.y,
                u[4] - box.x, u[5] - box.y
            ];

            const path = new GL.CkPath();
            path.moveTo(v[0], v[1]);
            path.lineTo(v[2], v[3]);
            path.lineTo(v[4], v[5]);
            path.close();

            const paint = new GL.CkPaint();
            paint.setStyle(GL.Constants.PAINT_STYLE_FILL);
            paint.setAntiAlias(true);
            paint.setColor4f(Color.Const.kColorBlueF.toGLType());

            canvas.drawPath(path, paint);

            paint.setColor4f(Color.Const.kColorMagentaF.toGLType());
            canvas.drawCircle(v[0], v[1], 5, paint);
            canvas.drawCircle(v[2], v[3], 5, paint);
            canvas.drawCircle(v[4], v[5], 5, paint);
        });

        submitter.submit(this.fRootRenderNode, 'USERDATA', false);
    }
}

function onPaintEvent(event: PaintEvent, renderCtx: RenderContext): void {
    renderCtx.render(event.drawContext.submitter);
}

function onSubmittedEvent(event: SubmittedEvent): void {
    std.print(`Scene submitted, serial = ${event.serial}, closure (str) = ${event.closure}\n`);
}

async function main(): Promise<void> {
    GL.RenderHost.Initialize({ name: 'Vizmoe',  major: 1,  minor: 0,  patch: 0 });
    const display = await GL.RenderHost.Connect();

    const surface = await display.createHWComposeSurface(WIN_W, WIN_H);
    await surface.setTitle('Vizmoe Sample');
    const drawCtx = await DrawContext.Make(surface);
    const renderCtx = new RenderContext(WIN_W, WIN_H);

    drawCtx.addEventListener(PaintEvent, event => onPaintEvent(event, renderCtx));
    drawCtx.submitter.addEventListener(SubmittedEvent, event => onSubmittedEvent(event));

    await surface.requestNextFrame();
}

await main();
