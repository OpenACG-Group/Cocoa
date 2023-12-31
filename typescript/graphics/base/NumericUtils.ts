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

// The minimum difference between two float numbers when
// comparing their equality.
export const kEPSILON = 0.000001;

export const kNEARLY_ZERO = 1.0 / (1 << 12);

/**
 * Compare the equality of two float numbers by using an absolute or relative
 * tolerance defined by the constant `kEPSILON`. The absolute tolerance is used
 * when `|a| or |b| <= 1.0`, otherwise, the relative tolerance is used.
 */
export function IsFloatEq(a: number, b: number): boolean {
    return Math.abs(a - b) <= kEPSILON * Math.max(1.0, Math.abs(a), Math.abs(b));
}

/**
 * Returns false if any of the element in `scalars` is infinity or NaN, otherwise,
 * returns true.
 */
export function ScalarsAreFinite(scalars: ArrayLike<number>): boolean {
    for (let i = 0; i < scalars.length; i++) {
        if (!Number.isFinite(scalars[i]) || Number.isNaN(scalars[i])) {
            return false;
        }
    }
    return true;
}

export function ScalarNearlyZero(x: number, tolerance: number = kNEARLY_ZERO): boolean {
    if (tolerance < 0) {
        throw RangeError('Tolerance of `ScalarNearlyZero` must not be < 0');
    }
    return Math.abs(x) <= tolerance;
}
