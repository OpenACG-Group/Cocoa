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

import {CkPoint, CkPoint3} from 'glamor';
import * as Fmt from '../../core/formatter';

/**
 * A two- or three-dimensional vector represented in
 * Cartesian Coordinate System.
 *
 * Use constructors to create vectors directly, for example:
 * @example
 *      // Default constructor makes a zero vector.
 *      let v = new Vector2f();
 *      let u = new Vector3f();
 *
 *      // Specify a coordinate
 *      let w = new Vector3f(1, 2, 1);
 */
export interface IVector<T> {
    /**
     * Get length of the vector |v|, also known as the modulus of a vector.
     */
    length(): number;

    /**
     * Get squared length of the vector, which is |v|^2.
     */
    lengthSquared(): number;

    /**
     * Get a string representation of the vector.
     *
     * @param fracDigits The number of fraction digits (precision).
     */
    toString(fracDigits?: number): string;

    /**
     * Add the vector and another specified vector `v`, and the result
     * will be returned as a new vector, leaving the vector itself unchanged.
     */
    add(v: T): T;

    /**
     * Subtract the vector from another specified vector `v`, and the result
     * will be returned as a new vector, leaving the vector itself unchanged.
     */
    sub(v: T): T;

    /**
     * Calculate the inner product (aka dot product) of the vector and another
     * specified vector `v`. Result is a scalar.
     */
    dot(v: T): number;

    /**
     * Calculate the outer product (aka cross product) of the vector and another
     * specified vector `v`, leaving the vector itself unchanged.
     * When it is applied to two-dimensional vectors, the result will be scalar which
     * represents the directed area of the parallelogram spanned by vectors.
     * When it is applied to three-dimensional vectors, the result will be a three-dimensional
     * vector which is the cross product of vectors.
     */
    cross(v: T): number | T;

    /**
     * Keep the direction of the vector, change the length to 1.
     * A new vector is returned, leaving the vector itself unchanged.
     */
    normalize(): T;

    /**
     * Invert the direction of the vector, and a new inverted vector
     * is returned, leaving the vector itself unchanged.
     */
    neg(): T;

    /**
     * Multiply the vector by `lambda`, and a new multiplied vector
     * is returned, leaving the vector itself unchanged.
     */
    mul(lambda: number): T;

    /**
     * Return the cosine of the included angle of two vectors.
     */
    angleCos(v: T): number;

    /**
     * Return the sine of the included angle of two vectors.
     */
    angleSin(v: T): number;

    equalTo(other: T): boolean;
    clone(): T;

    toGLType(): CkPoint | CkPoint3;
}

export class Vector2f implements IVector<Vector2f>, Fmt.Formattable {
    constructor(public x: number = 0, public y: number = 0) {}

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
        return (this.x == other.x && this.y == other.y);
    }

    clone(): Vector2f {
        return new Vector2f(this.x, this.y);
    }

    toGLType(): CkPoint {
        return [this.x, this.y];
    }

    public [Fmt.kObjectFormatter](ctx: Fmt.FormatterContext): Array<Fmt.TextBlock> {
        return [
            Fmt.TB(Fmt.TextBlockLayoutHint.kPrefix, [
                Fmt.TAG('Vizmoe.Vector2f')
            ]),
            Fmt.TB(Fmt.TextBlockLayoutHint.kCompoundStructureBegin, [
                Fmt.TAG('(')
            ]),
            ...Fmt.formatAnyValue(this.x, ctx),
            Fmt.TB(Fmt.TextBlockLayoutHint.kSeparator, [
                Fmt.TAG(',')
            ]),
            ...Fmt.formatAnyValue(this.y, ctx),
            Fmt.TB(Fmt.TextBlockLayoutHint.kCompoundStructureEnd, [
                Fmt.TAG(')')
            ])
        ];
    }
}

export class Vector3f implements IVector<Vector3f>, Fmt.Formattable {
    constructor(public x: number = 0,
                public y: number = 0,
                public z: number = 0) {}

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

    toGLType(): CkPoint3 {
        return [this.x, this.y, this.z];
    }

    equalTo(other: Vector3f): boolean {
        return (this.x == other.x &&
                this.y == other.y &&
                this.z == other.z);
    }

    clone(): Vector3f {
        return new Vector3f(this.x, this.y, this.z);
    }

    public [Fmt.kObjectFormatter](ctx: Fmt.FormatterContext): Array<Fmt.TextBlock> {
        return [
            Fmt.TB(Fmt.TextBlockLayoutHint.kPrefix, [
                Fmt.TAG('Vizmoe.Vector3f')
            ]),
            Fmt.TB(Fmt.TextBlockLayoutHint.kCompoundStructureBegin, [
                Fmt.TAG('(')
            ]),
            ...Fmt.formatAnyValue(this.x, ctx),
            Fmt.TB(Fmt.TextBlockLayoutHint.kSeparator, [
                Fmt.TAG(',')
            ]),
            ...Fmt.formatAnyValue(this.y, ctx),
            Fmt.TB(Fmt.TextBlockLayoutHint.kSeparator, [
                Fmt.TAG(',')
            ]),
            ...Fmt.formatAnyValue(this.z, ctx),
            Fmt.TB(Fmt.TextBlockLayoutHint.kCompoundStructureEnd, [
                Fmt.TAG(')')
            ])
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
}

/* Type aliases */
export type Point2f = Vector2f;
export type Point3f = Vector3f;
