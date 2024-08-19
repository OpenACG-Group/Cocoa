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

import { Scene, SceneBuilder } from 'present';
import * as Render from 'renderer';
import * as DL from './GskDisplayList';
import { GskDLDrawOpInspect, GskDLDrawOpVerb } from './GskDLDrawOpInfo';

export namespace Optimizers {

    function eliminateRedundantChildSlices(slice: DL.DLDrawOpSlice): void {
        const removedDrawOpMark = {};

        for (const op of GskDLDrawOpInspect.Iterate(slice.fOps)) {
            if (op.verb != GskDLDrawOpVerb.kDrawSlice && op.verb != GskDLDrawOpVerb.kDrawSliceLayer) {
                continue;
            }
            const childIdx = op.args[0] as number;
            const childSlice = slice.fChildren[childIdx];
            eliminateRedundantChildSlices(childSlice);
            if (!childSlice.isEmpty()) {
                continue;
            }
            // Remove empty slice and its DrawSlice/DrawSliceLayer op
            slice.fChildren[childIdx] = null;

            // Mark the corresponding DrawOp with `removedDrawOpStub`, which helps us filter out
            // these elements later.
            slice.fOps[op.startIndex] = removedDrawOpMark;
            slice.fOps[op.startIndex + 1] = removedDrawOpMark;
            if (op.verb == GskDLDrawOpVerb.kDrawSliceLayer) {
                slice.fOps[op.startIndex + 2] = removedDrawOpMark;
            }
        }

        // Remove elements that are marked
        slice.fOps = slice.fOps.filter(value => value !== removedDrawOpMark);
    }

    export function EliminateRedundantSlices(root: DL.DLDrawOpSlice): void {
        eliminateRedundantChildSlices(root);
    }

}

export namespace Translator {
    // Slice Translation information
    export interface SliceTrctxInfo {
        // Whether the slice opens a new Picture layer.
        isPictureBoundary: boolean;
        previousPicture: Render.Picture | null;

        // Number of previous frames during which the slice keeps unchanged.
        generation: number;
    }
    
    export class Context {
        private fPersistent: DL.DLDrawOpSlice;

        constructor() {
            this.fPersistent = null;
        }

        private static MarkInitialGenerationInfo(slice: DL.DLDrawOpSlice): void {
            const info: SliceTrctxInfo = {
                isPictureBoundary: false,
                previousPicture: null,
                generation: 1
            };
            slice._internal_setTrctxInfo(info);
            for (const childSlice of slice.fChildren) {
                Context.MarkInitialGenerationInfo(childSlice);
            }
        }

        public diffUpdate(root: DL.DLDrawOpSlice): void {
            if (this.fPersistent == null) {
                this.fPersistent = root;
                Context.MarkInitialGenerationInfo(this.fPersistent);
            }

            // TODO(sora): implement this.
        }
    }

    export function Perform(rootSlice: DL.DLDrawOpSlice, trCtx: Context, totalSliceCount: number): Scene {
        const builder = new SceneBuilder(rootSlice.fBounds);

        // Create slices comparison cache table
        const comparisonCache: DL.SliceComparisonCache = new Array(totalSliceCount);
        for (let i = 0; i < totalSliceCount; i++) {
            comparisonCache[i] = new Array<boolean>(totalSliceCount).fill(null);
        }

        // TODO(sora): complete this.
        return builder.build();
    }
}
