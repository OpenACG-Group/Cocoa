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

import { LinkedList } from '../../core/linked_list';
import { RenderObject, FlattenContext, RenderObjectType } from './RenderObject';
import { Vector2f } from '../graphics/Vector';

export enum TransformType {
    kRotate,
    kOffset
}

export class TransformerRenderObject extends RenderObject {
    private params_: number[];
    private type_: TransformType;

    public static MakeOffset(parent: RenderObject, offset: Vector2f): TransformerRenderObject {
        return new TransformerRenderObject(parent, TransformType.kOffset,
                                           [offset.x, offset.y]);
    }

    public static MakeRotate(parent: RenderObject, rad: number, pivot: Vector2f): TransformerRenderObject {
        return new TransformerRenderObject(parent, TransformType.kRotate,
                                           [rad, pivot.x, pivot.y]);
    }

    private constructor(parent: RenderObject, type: TransformType, params: number[]) {
        super(parent, new LinkedList(), RenderObjectType.kTransformer);
        this.type_ = type;
        this.params_ = params;
    }

    public flatten(context: FlattenContext): void {
        if (this.type_ == TransformType.kOffset) {
            context.recorder.insertPushOffset(
                new Vector2f(this.params_[0], this.params_[1]));
        } else if (this.type_ == TransformType.kRotate) {
            context.recorder.insertPushRotate(
                this.params_[0], new Vector2f(this.params_[1], this.params_[2]));
        }

        for (const child of this.children_) {
            child.flatten(context);
        }

        context.recorder.insertPop();
    }
}
