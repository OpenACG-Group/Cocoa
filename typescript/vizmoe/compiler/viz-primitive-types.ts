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

export enum VizNumericValueUnit {
    kNone,
    kPx,
    kEm,
    kRem,
    kVw,
    kVh,
    kPercent
}

export enum VizNumericValueType {
    kInteger,
    kFloat
}

export class VizNumericValue {
    constructor(public value: number, public unit: VizNumericValueUnit,
                public type: VizNumericValueType) {}
    
    public isInteger(): boolean {
        return (this.type == VizNumericValueType.kInteger);
    }

    public isFloat(): boolean {
        return (this.type == VizNumericValueType.kFloat);
    }
}

/**
 * A hexadecimal ARGB value; each component has a value in [0, 255]
 */
export class VizHexARGBValue {
    constructor(public alpha: number, public red: number, public green: number,
                public blue: number) {}
}
