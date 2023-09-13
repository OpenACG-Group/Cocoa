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
import * as gl from 'glamor';
import { MatVector, OpenCVLib } from '../wasm/opencv/lib/opencv';
import { LoadFromProjectThirdParty } from '../wasm/wasm-loader-polyfill';
import { FFT } from './fft';
import * as GL from "glamor";

const cvPromise = LoadFromProjectThirdParty<OpenCVLib>('opencv_js.wasm', 'opencv.js');

const cv = await cvPromise;

function roundToNextPow2(int: number): number {
    int--;
    int |= int >> 1;
    int |= int >> 2;
    int |= int >> 4;
    int |= int >> 8;
    int |= int >> 16;
    int++;
    return int;
}

function slurp(a: number, b: number, t: number) {
    return (a - b) * t + a;
}

function resampleContourPoints(points: ArrayLike<number>, numSamples: number): Array<number> {
    const pointsCount = points.length / 2;
    const newPoints: number[] = [];
    for (let i = 0; i < numSamples; i ++) {
        const position = pointsCount * (i / numSamples);
        const index = Math.floor(position);
        const nextIndex = (index + 1) % pointsCount;
        const amt = position - index;
        newPoints.push(
            slurp(points[index * 2], points[nextIndex * 2], amt),
            slurp(points[index * 2 + 1], points[nextIndex * 2 + 1], amt),
        );
    }
    return newPoints;
}

interface FFTResultTriple {
    amplitude: number;
    freq: number;
    phase: number;
}

function resolveFFTResultTriples(fftResult: ArrayLike<number>, minAmplitude: number, q: number): FFTResultTriple[] {
    const L = fftResult.length / 2;

    const triples: FFTResultTriple[] = [];
    const terms = q * L;
    for (let i = 0; i < terms; i++) {
        // to reorder the frequencies a little nicer, we pick from the front and back alternatively
        const j = i % 2 == 0 ? i / 2 : L - ((i+1) / 2);
        const x = fftResult[2 * j];
        const y = fftResult[2 * j + 1];
        const freq = ((j + L / 2) % L) - L / 2;

        const amp = Math.sqrt(x * x + y * y) / L;
        if (amp < minAmplitude) {
            continue;
        }

        triples.push({
            freq: freq,
            amplitude: amp,
            phase: Math.atan2(y, x),
        });
    }

    return triples;
}

function computeBitmapContours(bitmap: gl.CkBitmap, scalar: number): MatVector {
    let src = cv.matFromArray(
        bitmap.height,
        bitmap.width,
        cv.CV_8UC4,
        new Uint8ClampedArray(bitmap.asTypedArray())
    );

    if (scalar != 1) {
        const scaled = new cv.Mat();
        cv.resize(src, scaled, new cv.Size(bitmap.width * scalar, bitmap.height * scalar));
        src.delete();
        src = scaled;
    }

    const blur = new cv.Mat();
    cv.GaussianBlur(src, blur, new cv.Size(3, 3), 0, 0);
    src.delete();

    const gray = new cv.Mat();
    cv.cvtColor(blur, gray, cv.COLOR_BGR2GRAY);
    blur.delete();

    const edges = new cv.Mat();
    cv.Canny(gray, edges, 50, 150);
    gray.delete();

    const contours = new cv.MatVector();
    const hierarchy = new cv.Mat();
    cv.findContours(edges, contours, hierarchy, cv.RETR_TREE, cv.CHAIN_APPROX_NONE);
    hierarchy.delete();

    return contours;
}

function computeFFTResultPoints(contours: MatVector, q: number): Array<Array<gl.CkPoint>> {
    const result: Array<Array<gl.CkPoint>> = [];

    for (let i = 0; i < contours.size(); i++) {
        const contour = contours.get(i);

        if (contour.rows == 1) {
            continue;
        }

        // std.print(`Contour#${i} [${contour.rows}]\n`);

        const pointNum = contour.rows;
        const roundedPointNum = roundToNextPow2(pointNum);
        const fft = new FFT(roundedPointNum);

        const fftResult = fft.createComplexArray();
        fft.transform(fftResult, resampleContourPoints(contour.data32S, roundedPointNum));

        const triples = resolveFFTResultTriples(fftResult, 0.01, q);

        const pts: gl.CkPoint[] = [];
        for (let t = 0; t < 1; t += 1 / roundedPointNum) {
            let dx = 0;
            let dy = 0;
            for (const triple of triples) {
                const w = 2 * Math.PI * triple.freq;
                dx += triple.amplitude * Math.cos(w * t + triple.phase);
                dy += triple.amplitude * Math.sin(w * t + triple.phase);
            }
            pts.push([dx, dy]);
        }

        result.push(pts);
    }

    return result;
}

interface AnimationContext {
    contours: MatVector;
    transformed: Array<Array<gl.CkPoint>>;
    currentContour: number;
    currentPointInContour: number;
    q: number;
    stop: boolean;
}

async function main(): Promise<void> {

    if (std.args.length != 2) {
        std.print('Options: <source image path> <size scalar>\n');
        return;
    }

    const bitmap = gl.CkBitmap.MakeFromEncodedFile(std.args[0]);
    // OpenCV only uses BGR color format (corresponds to RGBA8888 in Glamor),
    // and we should make sure the image is decoded into that format.
    if (bitmap.colorType != gl.Constants.COLOR_TYPE_RGBA8888) {
        throw Error('Image is decoded into an unsupported color format');
    }

    const scalar = Number.parseFloat(std.args[1]);
    if (isNaN(scalar)) {
        throw Error('Invalid size scalar');
    }

    const vpWidth = bitmap.width * scalar, vpHeight = bitmap.height * scalar;

    const animationCtx: AnimationContext = {
        contours: null,
        transformed: null,
        currentContour: 0,
        currentPointInContour: 0,
        q: 0.2,
        stop: false
    };

    animationCtx.contours = computeBitmapContours(bitmap, scalar);
    animationCtx.transformed = computeFFTResultPoints(animationCtx.contours, 1);

    std.print(`Image preprocessing is done\n`);

    const presentThread = await GL.PresentThread.Start();
    let display = await presentThread.createDisplay();
    display.addOnceListener('closed', () => {
        presentThread.dispose();
    });

    const window = await display.createRasterSurface(vpWidth, vpHeight);
    await window.setTitle('FFT Drawing');

    window.addListener('keyboard-key', (key: gl.KeyboardKey, modifiers: gl.KeyboardModifiers, pressed: boolean) => {
        if (pressed == true) {
            return;
        }

        if (key != gl.Constants.KEY_UP && key != gl.Constants.KEY_DOWN) {
            return;
        }

        if (key == gl.Constants.KEY_UP) {
            animationCtx.q += 0.05;
        } else if (key == gl.Constants.KEY_DOWN) {
            animationCtx.q -= 0.05;
        }

        if (animationCtx.q < 0) {
            animationCtx.q = 0;
        } else if (animationCtx.q > 1) {
            animationCtx.q = 1;
        }

        std.print(`q = ${animationCtx.q}\n`);
        animationCtx.transformed = computeFFTResultPoints(animationCtx.contours, animationCtx.q);
        animationCtx.currentContour = 0;
        animationCtx.currentPointInContour = 0;
    });

    window.addOnceListener('close', () => {
        animationCtx.stop = true;
        blender.dispose();
        window.close();
    });

    window.addOnceListener('closed', () => {
        display.close();
    });

    const blender = await window.createBlender();

    const paint = new gl.CkPaint();
    paint.setAntiAlias(true);
    paint.setColor4f([1, 0.14, 0.4, 1]);
    paint.setStrokeWidth(1);

    const headPaint = new gl.CkPaint();
    headPaint.setAntiAlias(true);
    headPaint.setStyle(gl.Constants.PAINT_STYLE_FILL);
    headPaint.setColor4f([0, 0, 1, 1]);

    const internalId = setInterval(() => {
        if (animationCtx.stop) {
            clearInterval(internalId);
            return;
        }

        // Prepare data
        let contourIdx = animationCtx.currentContour;
        let pointIdx = animationCtx.currentPointInContour;
        if (pointIdx >= animationCtx.transformed[contourIdx].length) {
            animationCtx.currentContour++;
            animationCtx.currentPointInContour = 0;
            contourIdx++;
            pointIdx = 0;
        }

        if (contourIdx >= animationCtx.transformed.length) {
            animationCtx.currentContour = 0;
            animationCtx.currentPointInContour = 0;
            contourIdx = 0;
            pointIdx = 0;
        }

        animationCtx.currentPointInContour += 16;

        // Draw
        const recorder = new gl.CkPictureRecorder();
        const canvas = recorder.beginRecording([0, 0, vpWidth, vpHeight]);

        canvas.clear([1, 1, 1, 1]);

        // Draw all the previous contours
        for (let i = 0; i < contourIdx; i++) {
            canvas.drawPoints(gl.Constants.CANVAS_POINT_MODE_POLYGON, animationCtx.transformed[i], paint);
        }

        // Draw current contour
        const pts = animationCtx.transformed[contourIdx].slice(0, pointIdx + 1);
        canvas.drawPoints(gl.Constants.CANVAS_POINT_MODE_POLYGON, pts, paint);

        canvas.drawCircle(pts[pts.length - 1][0], pts[pts.length - 1][1], 5, headPaint);

        const scene = new gl.SceneBuilder(vpWidth, vpHeight)
            .pushOffset(0, 0)
            .addPicture(recorder.finishRecordingAsPicture(), true, 0, 0)
            .build();

        blender.update(scene).then(() => {
            scene.dispose();
        });
    }, 30);
}

await main();
