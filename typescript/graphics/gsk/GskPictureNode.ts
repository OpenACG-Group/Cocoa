/**
 * This file is part of Vizmoe.
 *
 * Vizmoe is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Vizmoe is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vizmoe. If not, see <https://www.gnu.org/licenses/>.
 */

import { GskRenderNode } from './GskRenderNode';
import { GskNodeAttribute } from './GskNodeAttribute';
import { Rect } from '../base/Rectangle';
import { Vector2f, VecBasis } from '../base/Vector';
import { CkCanvas, SceneBuilder, CkPicture, CkPictureRecorder } from 'glamor';

export type GskPictureDrawCallback = (canvas: CkCanvas, bounds: Rect) => void;
const kEmptyDrawCallback = () => {};

export class GskPictureNode extends GskRenderNode {
    private fDrawBounds: GskNodeAttribute<Rect>;
    private fDrawCallback: GskNodeAttribute<GskPictureDrawCallback>;
    private fOffset: NonNullable<Vector2f>;
    private fApplyClip: boolean;
    private fCachedPicture: CkPicture | null;

    constructor() {
        const drawBounds = new GskNodeAttribute('draw-bounds', Rect.MakeEmpty());
        const drawCallback = new GskNodeAttribute('draw-callback', kEmptyDrawCallback);
        super([ drawBounds, drawCallback ]);
        this.fDrawBounds = drawBounds;
        this.fDrawCallback = drawCallback;
        this.fCachedPicture = null;
        this.fOffset = VecBasis.kVec2_zero;
        this.fApplyClip = true;
    }

    /**
     * #[ObservedAttribute(draw-bounds)]
     * A rectangle that constrains the drawing operations. If attribute `apply-clip` is
     * true, the rectangle will be used to clip drawing operations. Otherwise, drawing
     * operations out of the rectangle may not be discarded.
     */
    public setDrawBounds(bounds: NonNullable<Rect>): void {
        this.fDrawBounds.set(bounds);
    }

    /**
     * #[ObservedAttribute(draw-callback)]
     * A callback function that will be called whenever repaint is needed.
     */
    public setDrawCallback(callback: NonNullable<GskPictureDrawCallback>): void {
        this.fDrawCallback.set(callback);
    }

    /**
     * #[FreeAttribute(offset)]
     * A vector that specifies an offset of Picture's top-left corner in parent's
     * coordinate space. Default is (0, 0).
     */
    public setOffset(offset: NonNullable<Vector2f>): void {
        this.fOffset = offset;
    }

    /**
     * #[FreeAttribute(apply-clip)]
     * Whether to clip the Picture with the rectangle specified by attribute `draw-bounds`.
     * Default is true.
     */
    public setApplyClip(clip: boolean): void {
        this.fApplyClip = clip;
    }

    protected onBuildScene(builder: SceneBuilder) {
        if (this.hasChangedAttribute()) {
            // The Picture will be repainted when attributes changes,
            // and the cached Picture should be discarded.
            this.fCachedPicture = null;
            this.updateAttributeStates();
        }

        // Repaint the Picture.
        if (this.fCachedPicture == null) {
            const bounds = this.fDrawBounds.get();
            const callback = this.fDrawCallback.get();
            const recorder = new CkPictureRecorder();
            callback(recorder.beginRecording(bounds.toCkArrayXYWHRect()), bounds);
            this.fCachedPicture = recorder.finishRecordingAsPicture();
        }

        builder.pushOffset(this.fOffset.x, this.fOffset.y);
        builder.addPicture(this.fCachedPicture, this.fApplyClip);
        builder.pop();
    }
}
