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

export interface Range<T> {
    min: T;
    max: T;
}

/**
 * Layout constraint is an object which is given by a
 * component's parent, telling the component its max/min
 * layout space in the parent's coordinate.
 */
export interface LayoutConstraint {
    width: Range<number>;
    height: Range<number>;
}

export class Base {
}
