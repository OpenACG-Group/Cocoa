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

import { GskConcreteType, GskInvalidationRecorder, GskProperty } from './GskNode';
import { GskRenderNode, RenderContext, GskScopedRenderContext } from './GskRenderNode';
import { GskPaintRecord } from './GskPaintRecord';
import { GskSampling } from './GskSampling';
import { CkImage } from 'glamor';
import { Rect } from '../base/Rectangle';
import { Point2f } from '../base/Vector';
import { GskDisplayList } from './GskDisplayList';
import { Mat3x3 } from '../base/Matrix';
import { Maybe } from '../../core/error';

export class GskImage extends GskRenderNode {
    @GskProperty<CkImage, GskImage>()
    public image: CkImage;

    @GskProperty<GskSampling, GskImage>(GskSampling.kLinear)
    public sampling: GskSampling;

    @GskProperty<boolean, GskImage>(true)
    public antiAlias: boolean;

    constructor() {
        super(GskConcreteType.kImage, 0);
    }

    protected onRevalidate(recorder: GskInvalidationRecorder, ctm: Mat3x3): Rect {
        if (this.image == null) {
            return Rect.MakeEmpty();
        }
        return Rect.MakeWH(this.image.width, this.image.height);
    }

    protected onNodeAt(point: Point2f): Maybe<GskRenderNode> {
        // ImageNode covers a rectangle area, so the hit-testing has been finished
        // through the bounds-rejection by `GskRenderNode.nodeAt()`.
        return Maybe.Ok(this);
    }

    protected onRender(dl: GskDisplayList, context: RenderContext): void {
        if (this.image == null) {
            return;
        }

        GskScopedRenderContext(dl, context, (mutator) => {
            const paintRec = new GskPaintRecord();
            paintRec.antiAlias = this.antiAlias;
            if (context != null) {
                if (context.fMaskShader != null) {
                    // Mask shaders cannot be applied via drawImage - we need layer isolation
                    mutator.setIsolation(this.bounds, dl.getTotalMatrix(), true);
                }
                mutator.asRC().modulatePaint(dl.getTotalMatrix(), paintRec, false);
            }
            dl.canvas.drawImage(
                this.image, 0, 0, this.sampling, paintRec.instantiatePaint());
        });
    }
}
