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

import { SceneBuilder } from 'glamor';
import {
    GskAttributeStateGroupOp,
    GskIStatefulAttribute,
    GskNodeAttributeStateGroup
} from './GskNodeAttribute';

export abstract class GskRenderNode {
    private fAttributesStateGroup: GskNodeAttributeStateGroup;

    protected constructor(attributeStates: GskIStatefulAttribute[]) {
        this.fAttributesStateGroup = new GskNodeAttributeStateGroup(
            attributeStates, GskAttributeStateGroupOp.kOr);
    }

    protected updateAttributeStates(): void {
        this.fAttributesStateGroup.updateChangeState();
    }

    public hasChangedAttribute(): boolean {
        return this.fAttributesStateGroup.hasChanged();
    }

    protected abstract onBuildScene(builder: SceneBuilder): void;
}
