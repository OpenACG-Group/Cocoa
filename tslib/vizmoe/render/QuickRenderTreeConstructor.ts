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

import {
    CompositorRenderObject,
    CompositionClip,
    CompositionFilter
} from "./CompositorRenderObject";

import { TransformerRenderObject } from "./TransformerRenderObject";
import { PainterRenderObject } from "./PainterRenderObject";
import { RenderObject } from "./RenderObject";
import { VideoTextureRenderObject } from "./VideoTextureRenderObject";
import { Vector2f } from "../graphics/Vector";
import { Rect } from "../graphics/Rect";
import { CkCanvas } from "synthetic://glamor";

export const CompRectClip = CompositionClip.MakeFromRect;
export const CompRRectClip = CompositionClip.MakeFromRRect;
export const CompFilter = CompositionFilter.MakeFromImageFilter;

function appendChildren<T extends RenderObject>(parent: RenderObject, children: RenderObject[]): T {
    for (const node of children) {
        parent.appendChild(node);
    }
    return parent as T;
}

export function Compositor(bg: CompositionFilter, fg: CompositionFilter,
                           clip: CompositionClip, ...children: RenderObject[]): CompositorRenderObject
{
    return appendChildren(new CompositorRenderObject(null, fg, bg, clip), children);
}

export function Offset(offset: Vector2f, ...children: RenderObject[]): TransformerRenderObject {
    return appendChildren(TransformerRenderObject.MakeOffset(null, offset), children);
}

export function Rotate(rad: number, pivot: Vector2f, ...children: RenderObject[]): TransformerRenderObject {
    return appendChildren(TransformerRenderObject.MakeRotate(null, rad, pivot), children);
}

export function Painter(bounds: Rect, callback: (canvas: CkCanvas) => void): PainterRenderObject {
    return new PainterRenderObject(null, callback, bounds);
}

export function VideoTexture(bounds: Rect): VideoTextureRenderObject {
    return new VideoTextureRenderObject(null, bounds);
}
