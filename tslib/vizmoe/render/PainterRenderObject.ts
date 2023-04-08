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

import { CkCanvas } from 'synthetic://glamor';
import { LinkedList } from "../../core/linked_list";
import { FlattenContext, RenderObject, RenderObjectType } from "./RenderObject";
import { Rect } from '../graphics/Rect';

export class PainterRenderObject extends RenderObject {
    private bounds_: Rect;
    private paint_callback_: (canvas: CkCanvas) => void;
    private is_dirty_: boolean;

    constructor(parent: RenderObject, paint_callback: (canvas: CkCanvas) => void, bounds: Rect) {
        super(parent, new LinkedList<RenderObject>(), RenderObjectType.kPainter);
        this.paint_callback_ = paint_callback;
        this.bounds_ = bounds;
        this.is_dirty_ = false;
    }

    public get bounds(): Rect {
        return this.bounds_;
    }

    public get isDirty(): boolean {
        return this.is_dirty_;
    }

    public markDirty(): void {
        this.is_dirty_ = true;
    }

    public markDirtyWithNewBounds(bounds: Rect): void {
        this.bounds_ = bounds;
        this.is_dirty_ = true;
    }

    public paint(canvas: CkCanvas): void {
        this.paint_callback_(canvas);
        this.is_dirty_ = false;
    }

    public flatten(context: FlattenContext): void {
        context.recorder.insertDrawPainter(this);
    }
}
