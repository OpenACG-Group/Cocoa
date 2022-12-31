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

/**
 * Geometry Primitives are mathematical basic geometry elements which can
 * be directly and only used for computation geometry. They are just geometry
 * shapes and do not have the ability to render themselves on canvas.
 */
export interface GeoPrimitiveBase {
}

/**
 * Geometry Views are renderers which are able to render Geometry Primitives
 * on a specific canvas. Most Geometry Primitives have their corresponding
 * Geometry Views to render them.
 */
export interface GeoViewBase {
}
