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

import {
    CkCanvas,
    Scene,
    SceneBuilder,
    CkPictureRecorder,
    CkPath,
    Constants
} from 'glamor';

import { GskPaintRecord } from './GskPaintRecord';
import { Rect, RRect } from '../base/Rectangle';
import { Mat3x3 } from '../base/Matrix';


/**
 * DisplayList, an intermediate representation of draw commands between GSK's
 * scenegraph and rasterization backends.
 */
export interface GskDisplayList {
    save(): number;

    saveLayer(bounds: Rect, paintRecord: GskPaintRecord): number;

    getSaveCount(): number;

    restore(): void;

    restoreToCount(saveCount: number): void;

    getTotalMatrix(): Mat3x3;

    concatMatrix(ctm: Mat3x3): void;

    clipRect(shape: Rect, antiAlias: boolean): void;

    clipRRect(shape: RRect, antiAlias: boolean): void;

    clipPath(path: CkPath, antiAlias: boolean): void;

    get canvas(): CkCanvas;
}

interface GLSceneStateStackFrame {
    SBDepth: number;
    saveCount: number;
    matrix: Mat3x3;
    bounds: Rect;

    // A Picture is recording
    isRecording: boolean;
    recorder: CkPictureRecorder | null;
}

/**
 * An implement of DisplayList that convert the DisplayList representation
 * into the layer tree representation of `[glamor] ContentAggregator`.
 */
export class GskGLSceneDisplayList implements GskDisplayList {
    private readonly fStateStack: Array<GLSceneStateStackFrame>;
    private readonly fSB: SceneBuilder;

    constructor(viewport: { width: number, height: number }) {
        this.fStateStack = [];
        this.fSB = new SceneBuilder(viewport.width, viewport.height);

        // Layer tree requires the root node must be a container
        // node. For more details, see `//src/Glamor/Layers/LayerTree.cc` file.
        this.fSB.pushOffset(0, 0);

        // Current state
        this.fStateStack.push({
            SBDepth: 1,
            saveCount: 1,
            matrix: Mat3x3.Identity(),
            bounds: Rect.MakeWH(viewport.width, viewport.height),
            isRecording: false,
            recorder: null
        });
    }

    private currentState(): GLSceneStateStackFrame {
        if (this.fStateStack.length == 0) {
            throw Error('State stack is empty');
        }
        return this.fStateStack[this.fStateStack.length - 1];
    }

    private finishPossibleRecording(): boolean {
        const current = this.currentState();
        if (!current.isRecording) {
            return;
        }
        const picture = current.recorder.finishRecordingAsPicture();
        this.fSB.addPicture(picture, true);
        current.isRecording = false;
        current.recorder = null;
    }

    private restoreSBStateTo(saveCount: number): void {
        const top = this.currentState();
        const popCount = top.SBDepth - this.fStateStack[saveCount - 1].SBDepth;
        for (let i = 0; i < popCount; i++) {
            this.fSB.pop();
        }
    }

    public build(): Scene {
        this.finishPossibleRecording();
        return this.fSB.build();
    }

    public save(): number {
        this.finishPossibleRecording();
        const top = this.currentState();
        this.fStateStack.push({
            SBDepth: top.SBDepth,
            saveCount: top.saveCount + 1,
            matrix: top.matrix,
            bounds: top.bounds,
            isRecording: false,
            recorder: null
        });
        return top.saveCount;
    }

    public saveLayer(bounds: Rect, paintRecord: GskPaintRecord): number {
        // Finish the current picture recording, if there is. This operation is necessary
        // as the picture recording may be interrupted by `saveLayer()` calls.
        // For example (pseudocode):
        //   1. saveLayer(...)
        //   2. get canvas and draw
        //   3. saveLayer(...)
        //   4. get canvas and draw
        //   5. restore()
        //   6. get canvas and draw
        //
        // Line 2 begins a recording on layer#1, but the SaveLayer operation at line 3
        // interrupts it and draws something on layer#2, then at line 6 we draw on layer#1
        // again. Things drawn at line 2 and line 6 cannot be stored in the same Picture,
        // though they are on the same layer.
        this.finishPossibleRecording();

        let subtreeDepth = 0;

        if (!paintRecord.isOpaque()) {
            this.fSB.pushOpacity(paintRecord.color.A);
            subtreeDepth++;
        }

        if (paintRecord.imageFilter) {
            this.fSB.pushImageFilter(paintRecord.imageFilter);
            subtreeDepth++;
        }

        // TODO(sora): apply blender and colorfilter

        // Create the new "current state"
        const lastState = this.currentState();
        this.fStateStack.push({
            SBDepth: lastState.SBDepth + subtreeDepth,
            saveCount: lastState.saveCount + 1,
            matrix: lastState.matrix,
            bounds: bounds,
            isRecording: false,
            recorder: null
        });

        return lastState.saveCount;
    }

    public getSaveCount(): number {
        return this.fStateStack.length;
    }

    public restore(): void {
        this.finishPossibleRecording();
        this.restoreSBStateTo(this.fStateStack.length - 1);
        this.fStateStack.pop();
    }

    public restoreToCount(saveCount: number): void {
        this.finishPossibleRecording();
        const top = this.currentState();
        if (saveCount >= top.saveCount) {
            return;
        }

        saveCount = Math.max(1, saveCount);
        this.restoreSBStateTo(saveCount);
        this.fStateStack.splice(saveCount);
    }

    public concatMatrix(ctm: Mat3x3): void {
        this.finishPossibleRecording();
        const top = this.currentState();
        top.matrix = top.matrix.clone();
        top.matrix.preConcat(ctm);

        this.fSB.pushTransform(ctm.toCkMat3x3Array());
        top.SBDepth++;
    }

    public getTotalMatrix(): Mat3x3 {
        const top = this.currentState();
        return top.matrix.clone();
    }

    public clipRect(shape: Rect, antiAlias: boolean): void {
        this.finishPossibleRecording();
        this.currentState().SBDepth++;
        this.fSB.pushRectClip(shape.toCkArrayXYWHRect(), antiAlias);
    }

    public clipRRect(shape: RRect, antiAlias: boolean): void {
        this.finishPossibleRecording();
        this.currentState().SBDepth++;
        this.fSB.pushRRectClip(shape.toCkRRect(), antiAlias);
    }

    public clipPath(path: CkPath, antiAlias: boolean): void {
        this.finishPossibleRecording();
        this.currentState().SBDepth++;
        this.fSB.pushPathClip(path, Constants.CLIP_OP_INTERSECT, antiAlias);
    }

    public get canvas(): CkCanvas {
        const top = this.currentState();
        if (top.isRecording) {
            return top.recorder.getRecordingCanvas();
        }
        top.isRecording = true;
        top.recorder = new CkPictureRecorder();
        // TODO(sora): tighten the bounds to have more efficient caches
        return top.recorder.beginRecording(top.bounds.toCkArrayXYWHRect());
    }

    public dumpStateStack(): void {
        this.fStateStack.forEach(state => {
            introspect.print(`[${state.saveCount}] depth=${state.SBDepth} recording=${state.isRecording}\n`);
        });
    }
}
