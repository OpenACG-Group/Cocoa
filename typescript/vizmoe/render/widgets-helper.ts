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
    BoxModelWidgetBase,
    WidgetType,
    LayoutConstraint
} from './widget-base';

import { Rect } from './rectangle';
import { RenderNode } from './render-node';

/**
 * `Space` does NOT render anything, just leaving the canvas space.
 * `Space` takes the maximum width and height, and is usually used
 * with a `ConstraintBox`.
 */
export class Space extends BoxModelWidgetBase {
    constructor() {
        super(WidgetType.kSpace, {});
    }

    protected onBoxModelWidgetDiffUpdate(other: BoxModelWidgetBase) {
    }

    protected onLayoutInContentBox(constraint: LayoutConstraint): Rect {
        return Rect.MakeWH(constraint.maxWidth, constraint.maxHeight);
    }

    protected onBoxModelWidgetRender(): RenderNode {
        return null;
    }
}
