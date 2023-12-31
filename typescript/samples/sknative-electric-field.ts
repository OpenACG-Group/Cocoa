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
import { Vector2f } from '../graphics/base/Vector';
import { Rect } from '../graphics/base/Rectangle';
import { Color4f } from '../graphics/base/Color';
import { Maybe } from '../core/error';

const FIELD_K = 10000;

interface FieldSourceModel {
    readonly bounds: Rect;
    probe(r: Vector2f): Maybe<Vector2f>;
    draw(canvas: ui.CkCanvas): void;
}

class PointCharge implements FieldSourceModel {
    private readonly fQ: number;

    public readonly bounds: Rect;

    constructor(q: number) {
        this.fQ = q;
        this.bounds = Rect.MakeWH(10, 10);
    }

    public draw(canvas: ui.CkCanvas): void {
        const paint = new ui.CkPaint();
        paint.setStyle(ui.Constants.PAINT_STYLE_FILL);
        paint.setColor4f([0, 0, 1, 1]);
        paint.setAntiAlias(true);
        canvas.drawCircle(5, 5, 5, paint);
    }

    public probe(r: Vector2f): Maybe<Vector2f> {
        if (r.lengthSquared() <= 25) {
            return Maybe.None();
        }
        const modulo = (FIELD_K * this.fQ) / r.lengthSquared();
        return Maybe.Ok(r.normalize().mul(modulo));
    }
}

interface PositionedSource {
    pos: Vector2f;
    source: FieldSourceModel;
}

const SOURCES: PositionedSource[] = [
    {
        pos: new Vector2f(100, 300),
        source: new PointCharge(10)
    },
    {
        pos: new Vector2f(400, 300),
        source: new PointCharge(-5)
    },
    {
        pos: new Vector2f(200, 100),
        source: new PointCharge(-1)
    }
];

const WINDOW_WIDTH = 1024, WINDOW_HEIGHT = 768;

async function render(window: ui.Surface): Promise<void> {
    const recorder = new ui.CkPictureRecorder();
    const canvas = recorder.beginRecording([0, 0, WINDOW_WIDTH, WINDOW_HEIGHT]);

    canvas.clear([1, 1, 1, 1]);

    for (const source of SOURCES) {
        canvas.save();
        canvas.translate(source.pos.x, source.pos.y);
        source.source.draw(canvas);
        canvas.restore();
    }

    const paint = new ui.CkPaint();
    paint.setAntiAlias(true);
    paint.setStyle(ui.Constants.PAINT_STYLE_STROKE);
    paint.setStrokeWidth(2);

    const rotateL = ui.CkMatrix.RotateRad(Math.PI / 4, [0, 0]);
    const rotateR = ui.CkMatrix.RotateRad(-Math.PI / 4, [0, 0]);

    for (let gx = 10; gx < WINDOW_WIDTH; gx += 50) {
        for (let gy = 10; gy < WINDOW_HEIGHT; gy += 50) {
            const pos = new Vector2f(gx, gy);
            let skip = false;
            let E = new Vector2f(0, 0);
            for (const source of SOURCES) {
                const model = source.source;
                const maybeE = model.probe(pos.sub(source.pos).sub(model.bounds.center));
                if (!maybeE.has()) {
                    skip = true;
                    break;
                }
                E = E.add(maybeE.unwrap());
            }
            if (skip) {
                continue;
            }

            paint.setColor4f(Color4f.FromHSV([360 * Math.exp(-E.length()), 1, 1], 1).toCkColor4f());

            const endp = E.normalize().mul(20).add(pos);
            const headp = E.normalize().mul(-5);

            const path = new ui.CkPath();
            path.moveTo(pos.x, pos.y);
            path.lineTo(endp.x, endp.y);

            const rhead = rotateR.mapPoint(headp.toCkPoint()),
                  lhead = rotateL.mapPoint(headp.toCkPoint());
            path.rLineTo(rhead[0], rhead[1]);
            path.moveTo(endp.x, endp.y);
            path.rLineTo(lhead[0], lhead[1]);

            canvas.drawPath(path, paint);
        }
    }

    const picture = recorder.finishRecordingAsPicture();
    const scene = new ui.SceneBuilder(WINDOW_WIDTH, WINDOW_HEIGHT)
        .pushOffset(0, 0)
        .addPicture(picture, true)
        .build();

    await window.contentAggregator.update(scene);
}

async function main(): Promise<void> {
    const thread = await ui.PresentThread.Start();
    const display = await thread.createDisplay();
    const window = await display.createRasterSurface(WINDOW_WIDTH, WINDOW_HEIGHT);

    let px = 0, py = 0;
    let hitSource: PositionedSource = null;

    window.addListener('pointer-motion', (x: number, y: number) => {
        px = x;
        py = y;

        if (hitSource) {
            hitSource.pos = new Vector2f(px, py);
            render(window);
        }
    });

    window.addListener('pointer-button', (button: ui.PointerButton, pressed: boolean) => {
        if (button != ui.Constants.POINTER_BUTTON_LEFT) {
            return;
        }

        if (pressed) {
            for (const source of SOURCES) {
                const bounds = source.source.bounds.makeOffset(
                    source.pos.x, source.pos.y);
                if (bounds.contains(px, py)) {
                    hitSource = source;
                    break;
                }
            }
            return;
        } else {
            hitSource = null;
        }
    });

    await render(window);
}

await main();