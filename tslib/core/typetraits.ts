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

function I<T>(v: T): string {
    return v[Symbol.toStringTag];
}

export function IsArrayBufferView(v: any): boolean {
    return ArrayBuffer.isView(v);
}

export function IsUint8Array(v: any): boolean {
    return I(v) == 'Uint8Array';
}

export function IsUint8ClampedArray(v: any): boolean {
    return I(v) == 'Uint8ClampedArray';
}

export function IsUint16Array(v: any): boolean {
    return I(v) == 'Uint16Array';
}

export function IsUint32Array(v: any): boolean {
    return I(v) == 'Uint32Array';
}

export function IsInt8Array(v: any): boolean {
    return I(v) == 'Int8Array';
}

export function IsInt16Array(v: any): boolean {
    return I(v) == 'Int16Array';
}

export function IsInt32Array(v: any): boolean {
    return I(v) == 'Int32Array';
}

export function IsFloat32Array(v: any): boolean {
    return I(v) == 'Float32Array';
}

export function IsFloat64Array(v: any): boolean {
    return I(v) == 'Float64Array';
}

export function IsBigInt64Array(v: any): boolean {
    return I(v) ==' BigInt64Array';
}

export function IsBigUint64Array(v: any): boolean {
    return I(v) == 'BigUint64Array';
}

export * from 'synthetic://typetraits';
