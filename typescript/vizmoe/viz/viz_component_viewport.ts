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

import { VizComponentBase, Signal } from './viz_component_base';
import { VizPositioner } from './viz_relative_positioner';

/**
 * Viewport component is the root node of global Component Tree
 * and the layout algorithm starts from it.
 */
class VizViewportComponent extends VizComponentBase {
    public readonly onClose: Signal<undefined>;

    constructor() {
        super('Viewport', null);

        super.positioner = VizPositioner.MakeUniformMarginBox(0);
        super.positioner.left = VizPositioner.MakeConstantLayoutAttr(0);
        super.positioner.top = VizPositioner.MakeConstantLayoutAttr(0);
        // All other attributes leave in `AUTO` mode by default

        this.onClose = new Signal<undefined>('close', 10);
    }
}