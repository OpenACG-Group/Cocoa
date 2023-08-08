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
    LayoutConstraint,
    RenderPositioner,
    BoxModelWidgetBase,
    WidgetType,
    BoxModelWidgetArgs
} from './widget-base';
import { PaintRenderNode, RenderNode } from './render-node';
import { Rect } from './rectangle';
import { Color4f } from './color';
import * as gl from 'glamor';

export interface SolidWidgetArgs extends BoxModelWidgetArgs {
    color: Color4f;
}

/**
 * `Solid` fills parent's canvas with a specified color.
 * `Solid` always takes the maximum width and height.
 */
export class Solid extends BoxModelWidgetBase {
    private fColor: Color4f;
    private readonly fPaintNode: PaintRenderNode;
    private fDirty: boolean;

    constructor(args: SolidWidgetArgs) {
        super(WidgetType.kSolid, args);
        this.fColor = args.color;
        this.fPaintNode = new PaintRenderNode();
        this.fDirty = false;
    }

    protected onLayoutInContentBox(constraint: LayoutConstraint): Rect {
        const size = Rect.MakeWH(constraint.maxWidth, constraint.maxHeight);
        if (!this.fLastLayoutBounds.equalTo(size)) {
            this.fDirty = true;
        }
        return size;
    }

    protected onBoxModelWidgetRender(positioner: RenderPositioner): RenderNode {
        if (this.fDirty) {
            const paint = new gl.CkPaint();
            paint.setStyle(gl.Constants.PAINT_STYLE_FILL);
            paint.setColor4f(this.fColor.toGLType());

            this.fPaintNode.update(positioner.inParent.margin, C => {
                C.drawRect(positioner.self.content.toGLType(), paint);
            });
            this.fDirty = false;
        }
        return this.fPaintNode;
    }

    protected onBoxModelWidgetDiffUpdate(other: Solid): void {
        if (!this.fColor.equalTo(other.fColor)) {
            this.fDirty = true;
            this.fColor = other.fColor;
        }
    }
}
