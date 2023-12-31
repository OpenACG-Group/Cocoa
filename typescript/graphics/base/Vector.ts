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

import { CkPoint, CkPoint3 } from 'glamor';
import * as Fmt from '../../core/formatter';
import { IsFloatEq } from './NumericUtils';

// Degenerate is the operation by which an N dimension vector is transformed
// to an N-1 dimension vector. This enumeration controls how to handle the
// N-th coordinate.
export enum Degenerate {
    // Discard the N-th coordinate directly:
    // (x, y, z, w) => (x, y, z)
    kDiscard,

    // Divide the other components by the N-th component:
    // (x, y, z, w) => (x / w, y / w, z / w)
    kHomogenousNorm,
    kDiv = kHomogenousNorm,

    // Multipy the other components with the N-th component:
    // (x, y, z, w) => (x * w, y * w, z * w)
    kMul
}

export class Vector2f implements Fmt.Formattable {
    constructor(public readonly x: number = 0, public readonly y: number = 0) {}

    static Dot(u: Vector2f, v: Vector2f): number {
        return (u.x * v.x + u.y * v.y);
    }

    static Cross(u: Vector2f, v: Vector2f): number {
        return (u.x * v.y - v.x * u.y);
    }

    static AngleCos(u: Vector2f, v: Vector2f): number {
        return (Vector2f.Dot(u, v) / (u.length() * v.length()));
    }

    static AngleSin(u: Vector2f, v: Vector2f): number {
        const C = Vector2f.AngleCos(u, v);
        return Math.sqrt(1 - C * C);
    }

    length(): number {
        return Math.sqrt(this.x * this.x + this.y * this.y);
    }

    lengthSquared(): number {
        return (this.x * this.x + this.y * this.y);
    }

    toString(fracDigits: number = 2): string {
        if (fracDigits < 0) {
            fracDigits = 2;
        }
        return `(${this.x.toFixed(fracDigits)}, ${this.y.toFixed(fracDigits)})`;
    }

    add(v: Vector2f): Vector2f {
        return new Vector2f(this.x + v.x, this.y + v.y);
    }

    sub(v: Vector2f): Vector2f {
        return new Vector2f(this.x - v.x, this.y - v.y);
    }

    dot(v: Vector2f): number {
        return Vector2f.Dot(this, v);
    }

    cross(v: Vector2f): number {
        return Vector2f.Cross(this, v);
    }

    normalize(): Vector2f {
        const L = this.length();
        return new Vector2f(this.x / L, this.y / L);
    }

    neg(): Vector2f {
        return new Vector2f(-this.x, -this.y);
    }

    mul(lambda: number): Vector2f {
        return new Vector2f(lambda * this.x, lambda * this.y);
    }

    rotate(rad: number): Vector2f {
        return new Vector2f(this.x * Math.cos(rad) - this.y * Math.sin(rad),
                            this.x * Math.sin(rad) + this.y * Math.cos(rad));
    }

    angleCos(v: Vector2f): number {
        return Vector2f.AngleCos(this, v);
    }

    angleSin(v: Vector2f): number {
        return Vector2f.AngleSin(this, v);
    }

    equalTo(other: Vector2f): boolean {
        return this.x === other.x && this.y === other.y;
    }

    clone(): Vector2f {
        return new Vector2f(this.x, this.y);
    }

    toCkPoint(): CkPoint {
        return [this.x, this.y];
    }

    public [Fmt.kObjectFormatter](ctx: Fmt.FormatterContext): Array<Fmt.TextBlock> {
        return [
            Fmt.TB(Fmt.TextBlockLayoutHint.kPrefix, [Fmt.TAG('Vector2f')]),
            Fmt.TB(Fmt.TextBlockLayoutHint.kCompoundStructureBegin, [Fmt.TAG('(')]),
            ...Fmt.formatAnyValue(this.x, ctx),
            Fmt.TB(Fmt.TextBlockLayoutHint.kSeparator, [Fmt.TAG(',')]),
            ...Fmt.formatAnyValue(this.y, ctx),
            Fmt.TB(Fmt.TextBlockLayoutHint.kCompoundStructureEnd, [Fmt.TAG(')')])
        ];
    }
}

export class Vector3f implements Fmt.Formattable {
    constructor(public readonly x: number = 0,
                public readonly y: number = 0,
                public readonly z: number = 0) {}

    static Dot(u: Vector3f, v: Vector3f): number {
        return (u.x * v.x + u.y * v.y + u.z * v.z);
    }

    static Cross(u: Vector3f, v: Vector3f): Vector3f {
        return new Vector3f(u.y * v.z - v.y * u.z,
                            v.x * u.z - u.x * v.z,
                            u.x * v.y - v.x * u.y);
    }

    static AngleCos(u: Vector3f, v: Vector3f): number {
        return (Vector3f.Dot(u, v) / (u.length() * v.length()));
    }

    static AngleSin(u: Vector3f, v: Vector3f): number {
        const C = Vector3f.AngleCos(u, v);
        return Math.sqrt(1 - C * C);
    }

    length(): number {
        return Math.sqrt(Vector3f.Dot(this, this));
    }

    lengthSquared(): number {
        return Vector3f.Dot(this, this);
    }

    toString(fracDigits: number = 2): string {
        if (fracDigits < 0) {
            fracDigits = 2;
        }
        return `(${this.x.toFixed(fracDigits)}, ` +
               `${this.y.toFixed(fracDigits)}, ` +
               `${this.z.toFixed(fracDigits)})`;
    }

    add(v: Vector3f): Vector3f {
        return new Vector3f(this.x + v.x,
                            this.y + v.y,
                            this.z + v.z);
    }

    sub(v: Vector3f): Vector3f {
        return new Vector3f(this.x - v.x,
                            this.y - v.y,
                            this.z - v.z);
    }

    dot(v: Vector3f): number {
        return Vector3f.Dot(this, v);
    }

    cross(v: Vector3f): Vector3f {
        return Vector3f.Cross(this, v);
    }

    normalize(): Vector3f {
        const L = this.length();
        return new Vector3f(this.x / L, this.y / L, this.z / L);
    }

    neg(): Vector3f {
        return new Vector3f(-this.x, -this.y, -this.z);
    }

    mul(lambda: number): Vector3f {
        return new Vector3f(lambda * this.x,
                            lambda * this.y,
                            lambda * this.z);
    }

    angleCos(v: Vector3f): number {
        return Vector3f.AngleCos(this, v);
    }

    angleSin(v: Vector3f): number {
        return Vector3f.AngleSin(this, v);
    }

    toCkPoint(): CkPoint3 {
        return [this.x, this.y, this.z];
    }

    equalTo(other: Vector3f): boolean {
        return (this.x === other.x &&
                this.y === other.y &&
                this.z === other.z);
    }

    clone(): Vector3f {
        return new Vector3f(this.x, this.y, this.z);
    }

    public [Fmt.kObjectFormatter](ctx: Fmt.FormatterContext): Array<Fmt.TextBlock> {
        return [
            Fmt.TB(Fmt.TextBlockLayoutHint.kPrefix, [Fmt.TAG('Vector3f')]),
            Fmt.TB(Fmt.TextBlockLayoutHint.kCompoundStructureBegin, [Fmt.TAG('(')]),
            ...Fmt.formatAnyValue(this.x, ctx),
            Fmt.TB(Fmt.TextBlockLayoutHint.kSeparator, [Fmt.TAG(',')]),
            ...Fmt.formatAnyValue(this.y, ctx),
            Fmt.TB(Fmt.TextBlockLayoutHint.kSeparator, [Fmt.TAG(',')]),
            ...Fmt.formatAnyValue(this.z, ctx),
            Fmt.TB(Fmt.TextBlockLayoutHint.kCompoundStructureEnd, [Fmt.TAG(')')])
        ];
    }

    public degenerate(opt: Degenerate = Degenerate.kDiscard): Vector2f {
        if (opt === Degenerate.kDiscard) {
            return new Vector2f(this.x, this.y);
        } else if (opt === Degenerate.kDiv) {
            const k = 1 / this.z;
            return new Vector2f(k * this.x, k * this.y);
        } else if (opt === Degenerate.kMul) {
            return new Vector2f(this.x * this.z, this.y * this.z);
        } else {
            throw RangeError('Invalid enumeration value for `Degenerate`');
        }
    }
}

export class Vector4f implements Fmt.Formattable {
    public static Dot(a: Vector4f, b: Vector4f): number {
        return (a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w);
    }

    constructor(public readonly x: number = 0,
                public readonly y: number = 0,
                public readonly z: number = 0,
                public readonly w: number = 0) {
    }

    public clone(): Vector4f {
        return new Vector4f(this.x, this.y, this.z, this.w);
    }

    public add(v: Vector4f): Vector4f {
        return new Vector4f(this.x + v.x, this.y + v.y, this.z + v.z, this.w + v.w);
    }

    public sub(v: Vector4f): Vector4f {
        return new Vector4f(this.x - v.x, this.y - v.y, this.z - v.z, this.w - v.w);
    }

    public neg(): Vector4f {
        return new Vector4f(-this.x, -this.y, -this.z, -this.w);
    }

    public mul(lambda: number): Vector4f {
        return new Vector4f(
            this.x * lambda,
            this.y * lambda,
            this.z * lambda,
            this.w * lambda
        );
    }

    public lengthSquared(): number {
        return (this.x * this.x + this.y * this.y + this.z * this.z +
                this.w * this.w);
    }

    public length(): number {
        return Math.sqrt(this.lengthSquared());
    }

    public normalize(): Vector4f {
        const f = 1.0 / this.length();
        return new Vector4f(
            this.x * f,
            this.y * f,
            this.z * f,
            this.w * f
        );
    }

    public dot(v: Vector4f): number {
        return Vector4f.Dot(this, v);
    }

    public equalTo(v: Vector4f): boolean {
        return (this.x === v.x &&
                this.y === v.y &&
                this.z === v.z &&
                this.w === v.w);
    }

    public degenerate(opt: Degenerate = Degenerate.kDiscard): Vector3f {
        if (opt === Degenerate.kDiscard) {
            return new Vector3f(this.x, this.y, this.z);
        } else if (opt === Degenerate.kDiv) {
            const k = 1 / this.w;
            return new Vector3f(k * this.x, k * this.y, k * this.z);
        } else if (opt === Degenerate.kMul) {
            const p = this.w;
            return new Vector3f(p * this.x, p * this.y, p * this.z);
        } else {
            throw RangeError('Invalid enumeration value for `Degenerate`');
        }
    }

    public [Fmt.kObjectFormatter](ctx: Fmt.FormatterContext): Array<Fmt.TextBlock> {
        return [
            Fmt.TB(Fmt.TextBlockLayoutHint.kPrefix, [
                Fmt.TAG('Vector4f')
            ]),
            Fmt.TB(Fmt.TextBlockLayoutHint.kCompoundStructureBegin, [Fmt.TAG('(')]),
            ...Fmt.formatAnyValue(this.x, ctx),
            Fmt.TB(Fmt.TextBlockLayoutHint.kSeparator, [Fmt.TAG(',')]),
            ...Fmt.formatAnyValue(this.y, ctx),
            Fmt.TB(Fmt.TextBlockLayoutHint.kSeparator, [Fmt.TAG(',')]),
            ...Fmt.formatAnyValue(this.z, ctx),
            Fmt.TB(Fmt.TextBlockLayoutHint.kSeparator, [Fmt.TAG(',')]),
            ...Fmt.formatAnyValue(this.w, ctx),
            Fmt.TB(Fmt.TextBlockLayoutHint.kCompoundStructureEnd, [Fmt.TAG(')')])
        ];
    }
}

/* Common orthogonal basis in Cartesian coordinates */
export namespace VecBasis {
    // X-axis in 2D coordinates, basis vector |i| = 1
    export const kVec2_i = new Vector2f(1, 0);
    // Y-axis in 2D coordinates, basis vector |j| = 1
    export const kVec2_j = new Vector2f(0, 1);

    // X-axis in 3D coordinates, basis vector |i| = 1
    export const kVec3_i = new Vector3f(1, 0, 0);
    // Y-axis in 3D coordinates, basis vector |j| = 1
    export const kVec3_j = new Vector3f(0, 1, 0);
    // Z-axis in 3D coordinates, basis vector |k| = 1
    export const kVec3_k = new Vector3f(0, 0, 1);

    export const kVec2_zero = new Vector2f(0, 0);
    export const kVec3_zero = new Vector3f(0, 0, 0);
    export const kVec4_zero = new Vector4f(0, 0, 0, 0);
}

/* Type aliases */
export type Point2f = Vector2f;
export type Point3f = Vector3f;
