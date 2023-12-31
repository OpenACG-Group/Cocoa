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
import { Vector2f, Point2f } from '../graphics/base/Vector';

const EPSILON = 0.0001;

function floatcmp(a: number, b: number): number {
    const d = a - b;
    if (Math.abs(d) <= EPSILON) {
        return 0;
    }
    return d < 0 ? -1 : 1;
}

function isRightTurn(p1: Point2f, p2: Point2f, p3: Point2f): boolean {
    const v1 = p2.sub(p1), v2 = p3.sub(p2);
    return (v1.cross(v2) > 0);
}

// Find convex hull in O(nlogn): Graham-Andrew algorithm
function findConvexHull(pts: Array<Point2f>): Array<Point2f> {
    const sorted = [...pts].sort((p1, p2) => {
        const cx = floatcmp(p1.x, p2.x);
        if (cx != 0) {
            return cx;
        }
        return floatcmp(p1.y, p2.y);
    });
    const N = sorted.length;

    const upper: Point2f[] = [sorted[0], sorted[1]];
    for (let i = 2; i < N; i++) {
        upper.push(sorted[i]);
        let L = upper.length;
        while (upper.length >= 3 && !isRightTurn(upper[L - 3], upper[L - 2], upper[L - 1])) {
            const last = upper.pop();
            upper.pop();
            upper.push(last);
            L--;
        }
    }

    const lower: Point2f[] = [sorted[N - 1], sorted[N - 2]];
    for (let i = N - 3; i >= 0; i--) {
        lower.push(sorted[i]);
        let L = lower.length;
        while (lower.length >= 3 && !isRightTurn(lower[L - 3], lower[L - 2], lower[L - 1])) {
            const last = lower.pop();
            lower.pop();
            lower.push(last);
            L--;
        }
    }

    return upper.concat(...lower.slice(1, lower.length - 1));
}

async function main(): Promise<void> {
    const pts: Point2f[] = [];

    const thread = await ui.PresentThread.Start();
    const display = await thread.createDisplay();
    const window = await display.createRasterSurface(800, 600);

    let px = 0, py = 0;

    window.addListener('pointer-motion', (x: number, y: number) => {
        px = x;
        py = y;
    });

    window.addListener('pointer-button', (button: ui.PointerButton, pressed: boolean) => {
        if (pressed) {
            return;
        }
        pts.push(new Vector2f(px, py));

        const recorder = new ui.CkPictureRecorder();
        const canvas = recorder.beginRecording([0, 0, 800, 600]);
        canvas.clear([1, 1, 1, 1]);

        const paint = new ui.CkPaint();
        paint.setAntiAlias(true);

        if (pts.length >= 3) {
            // Draw convex-hull contour
            paint.setStrokeWidth(2);
            paint.setColor4f([1, 0, 0, 1]);
            paint.setStyle(ui.Constants.PAINT_STYLE_STROKE);

            const path = new ui.CkPath();
            const convex = findConvexHull(pts);
            path.moveTo(convex[0].x, convex[0].y);
            for (let i = 1; i < convex.length; i++) {
                path.lineTo(convex[i].x, convex[i].y);
            }
            path.close();
            canvas.drawPath(path, paint);
        }

        // Draw points
        paint.setStyle(ui.Constants.PAINT_STYLE_FILL);
        paint.setColor4f([0, 0, 1, 1]);

        for (const p of pts) {
            canvas.drawCircle(p.x, p.y, 4, paint);
        }

        window.contentAggregator.update(
            new ui.SceneBuilder(800, 600)
                .pushOffset(0, 0)
                .addPicture(recorder.finishRecordingAsPicture(), true)
                .build()
        );
    });

    window.addListener('close', () => {
        window.close().then(() => {
            return display.close();
        }).then(() => {
            thread.dispose();
        });
    });
}

await main();
