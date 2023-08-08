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
    WidgetType,
    BoxModelContainerBase,
    LayoutConstraint,
    RenderPositioner
} from './widget-base';

import { RenderNode } from './render-node';
import { Rect } from './rectangle';
import { Color4f } from './color';

export enum BorderStyle {
    kNone,
    kSolid
}

export interface BorderSide {
    color: Color4f;
    style: BorderStyle;
    lineWidth: number;
}

export interface BorderCorner {
}

export class Border {
}

// TODO(sora): implement this class
export class Decoration {
    public static None(): Decoration {
        return null;
    }

    public diffUpdate(other: Decoration): void {
    }

    public aggregateDecoration(positioner: RenderPositioner, contentRN: RenderNode): RenderNode {
        return null;
    }
}
