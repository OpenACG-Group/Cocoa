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

import { Maybe } from '../../core/error';
import { Rect } from './Rectangle';
import { Vector2f, Vector3f, Vector4f } from './Vector';
import * as numeric from './NumericUtils';
import { CkMat3x3, CkMatrix, Constants } from 'glamor';

// Throw a `RangeError` if `i` is not an integer in [l, h)
function CheckIntIndex(i: number, l: number, h: number): void {
    if (!Number.isInteger(i) || i < l || i >= h) {
        throw RangeError(`Invalid index value: non-integer or out of range [${l}, ${h})`);
    }
}

// Taken from the gl-matrix project:
// https://github.com/toji/gl-matrix/blob/master/src/mat3.js
function MultiplyMat3x3F32(out: Float32Array, a: Float32Array, b: Float32Array): void {
    // Note that the data stored in `mat` is float32, but the following cache variables
    // are double (for the JavaScript specification). V8 automatically promotes
    // our float32 value to double, and we always perform the calculation in
    // double, which helps us avoid prematurely losing precision along the way.

    let a00 = a[0],
        a01 = a[1],
        a02 = a[2];
    let a10 = a[3],
        a11 = a[4],
        a12 = a[5];
    let a20 = a[6],
        a21 = a[7],
        a22 = a[8];
    let b00 = b[0],
        b01 = b[1],
        b02 = b[2];
    let b10 = b[3],
        b11 = b[4],
        b12 = b[5];
    let b20 = b[6],
        b21 = b[7],
        b22 = b[8];

    out[0] = b00 * a00 + b01 * a10 + b02 * a20;
    out[1] = b00 * a01 + b01 * a11 + b02 * a21;
    out[2] = b00 * a02 + b01 * a12 + b02 * a22;
    out[3] = b10 * a00 + b11 * a10 + b12 * a20;
    out[4] = b10 * a01 + b11 * a11 + b12 * a21;
    out[5] = b10 * a02 + b11 * a12 + b12 * a22;
    out[6] = b20 * a00 + b21 * a10 + b22 * a20;
    out[7] = b20 * a01 + b21 * a11 + b22 * a21;
    out[8] = b20 * a02 + b21 * a12 + b22 * a22;
}

function Mat3x3DeterminantF32(a: Float32Array): number {
    let a00 = a[0],
        a01 = a[1],
        a02 = a[2];
    let a10 = a[3],
        a11 = a[4],
        a12 = a[5];
    let a20 = a[6],
        a21 = a[7],
        a22 = a[8];

    return (
        a00 * (a22 * a11 - a12 * a21) +
        a01 * (-a22 * a10 + a12 * a20) +
        a02 * (a21 * a10 - a11 * a20)
    );
}

// Taken from the gl-matrix project:
// the `invert` function in https://github.com/toji/gl-matrix/blob/master/src/mat3.js
function InvertMat3x3F32(out: Float32Array, a: Float32Array): number {
    let a00 = a[0],
        a01 = a[1],
        a02 = a[2];
    let a10 = a[3],
        a11 = a[4],
        a12 = a[5];
    let a20 = a[6],
        a21 = a[7],
        a22 = a[8];

    let b01 = a22 * a11 - a12 * a21;
    let b11 = -a22 * a10 + a12 * a20;
    let b21 = a21 * a10 - a11 * a20;

    let det = a00 * b01 + a01 * b11 + a02 * b21;

    if (det === 0) {
        return 0;
    }
    det = 1.0 / det;

    out[0] = b01 * det;
    out[1] = (-a22 * a01 + a02 * a21) * det;
    out[2] = (a12 * a01 - a02 * a11) * det;
    out[3] = b11 * det;
    out[4] = (a22 * a00 - a02 * a20) * det;
    out[5] = (-a12 * a00 + a02 * a10) * det;
    out[6] = b21 * det;
    out[7] = (-a21 * a00 + a01 * a20) * det;
    out[8] = (a11 * a00 - a01 * a10) * det;

    return det;
}

function TransposeMat3x3F32(out: Float32Array, a: Float32Array): void {
    out[0] = a[0];
    out[1] = a[3];
    out[2] = a[6];
    out[3] = a[1];
    out[4] = a[4];
    out[5] = a[7];
    out[6] = a[2];
    out[7] = a[5];
    out[8] = a[8];
}

export class Mat3x3 {
    // Stored in column-major:
    // Indices:
    // | 0 3 6 |
    // | 1 4 7 |
    // | 2 5 8 |
    private readonly fMat: Float32Array;

    public static Identity(): Mat3x3 {
        return new Mat3x3().refill(
            1, 0, 0,
            0, 1, 0,
            0, 0, 1
        );
    }
    
    public static RowMajor(r: ArrayLike<number>): Mat3x3 {
        return new Mat3x3().refill(
            r[0], r[1], r[2],
            r[3], r[4], r[5],
            r[6], r[7], r[8]
        );
    }

    public static ColMajor(r: ArrayLike<number>): Mat3x3 {
        return new Mat3x3().refill(
            r[0], r[3], r[6],
            r[1], r[4], r[7],
            r[2], r[5], r[8]
        );
    }

    public static Rows(r0: Vector3f, r1: Vector3f, r2: Vector3f): Mat3x3 {
        return new Mat3x3().refill(
            r0.x, r0.y, r0.z,
            r1.x, r1.y, r1.z,
            r2.x, r2.y, r2.z
        );
    }

    public static Cols(c0: Vector3f, c1: Vector3f, c2: Vector3f): Mat3x3 {
        return new Mat3x3().refill(
            c0.x, c1.x, c2.x,
            c0.y, c1.y, c2.y,
            c0.z, c1.z, c2.z
        );
    }

    public static FromCkMatrix(mat: CkMatrix): Mat3x3 {
        return new Mat3x3().refill(
            mat.scaleX, mat.skewX, mat.transX,
            mat.skewY, mat.scaleY, mat.transY,
            mat.persp0, mat.persp1, mat.persp2
        );
    }

    public static Translate(x: number, y: number): Mat3x3 {
        return new Mat3x3().setTranslate(x, y);
    }

    public static Scale(sx: number, sy: number): Mat3x3 {
        return new Mat3x3().setScale(sx, sy);
    }

    public static Rotate(center: Vector2f, rad: number): Mat3x3 {
        return new Mat3x3().setRotate(center, rad);
    }

    public static Concat(a: Mat3x3, b: Mat3x3): Mat3x3 {
        return new Mat3x3().setConcat(a, b);
    }

    private constructor(array: Float32Array = null) {
        if (array == null) {
            this.fMat = new Float32Array(9);
        } else {
            this.fMat = array;
        }
    }

    private refill(m11: number, m12: number, m13: number,
                   m21: number, m22: number, m23: number,
                   m31: number, m32: number, m33: number): Mat3x3 {
        const m = this.fMat;
        m[0] = m11;
        m[1] = m21;
        m[2] = m31;
        m[3] = m12;
        m[4] = m22;
        m[5] = m32;
        m[6] = m13;
        m[7] = m23;
        m[8] = m33;
        return this;
    }

    public get underlyingArray(): Float32Array {
        return this.fMat;
    }

    public equalTo(other: Mat3x3): boolean {
        const a = this.fMat, b = other.fMat;
        return a[0] === b[0] &&
               a[1] === b[1] &&
               a[2] === b[2] &&
               a[3] === b[3] &&
               a[4] === b[4] &&
               a[5] === b[5] &&
               a[6] === b[6] &&
               a[7] === b[7] &&
               a[8] === b[8];
    }

    public clone(): Mat3x3 {
        return new Mat3x3(new Float32Array([...this.fMat]));
    }

    public at(r: number, c: number): number {
        CheckIntIndex(r, 0, 3);
        CheckIntIndex(c, 0, 3);
        return this.fMat[r + 3 * c];
    }

    public setAt(r: number, c: number, value: number): void {
        CheckIntIndex(r, 0, 3);
        CheckIntIndex(c, 0, 3);
        this.fMat[r + 3 * c] = value;
    }

    public row(i: number): Vector3f {
        CheckIntIndex(i, 0, 3);
        const m = this.fMat;
        return new Vector3f(m[i], m[i + 3], m[i + 6]);
    }

    public setRow(i: number, r: Vector3f): void {
        CheckIntIndex(i, 0, 3);
        this.fMat[i] = r.x;
        this.fMat[i + 3] = r.y;
        this.fMat[i + 6] = r.z;
    }

    public col(i: number): Vector3f {
        CheckIntIndex(i, 0, 3);
        const m = this.fMat;
        const j = i * 3;
        return new Vector3f(m[j], m[j + 1], m[j + 2]);
    }

    public setCol(i: number, c: Vector3f): void {
        CheckIntIndex(i, 0, 3);
        const j = i * 3;
        this.fMat[j] = c.x;
        this.fMat[j + 1] = c.y;
        this.fMat[j + 2] = c.z;
    }

    public determinant(): number {
        return Mat3x3DeterminantF32(this.fMat);
    }

    public setIdentity(): Mat3x3 {
        return this.refill(
            1, 0, 0,
            0, 1, 0,
            0, 0, 1
        );
    }

    public setTranslate(x: number, y: number): Mat3x3 {
        return this.refill(
            1, 0, x,
            0, 1, y,
            0, 0, 1
        );
    }

    public setScale(x: number, y: number): Mat3x3 {
        return this.refill(
            x, 0, 0,
            0, y, 0,
            0, 0, 1
        );
    }

    public setRotate(center: Vector2f, rad: number): Mat3x3 {
        const s = Math.sin(rad);
        const c = Math.cos(rad);
        const t = 1 - c;
        const x = center.x;
        const y = center.y;

        // Rotate matrix:
        // | cosA, -sinA,  ysinA + x(1 - cosA) |
        // | sinA,  cosA, -xsinA + y(1 - cosA) |
        // |    0,     0,                    1 |
        return this.refill(
            c, -s,  s * y + t * x,
            s,  c, -s * x + t * y,
            0, 0, 1
        );
    }

    // Compose this T = A x B
    public setConcat(a: Mat3x3, b: Mat3x3): Mat3x3 {
        MultiplyMat3x3F32(this.fMat, a.fMat, b.fMat);
        return this;
    }

    // Compose this T = T x M
    public preConcat(m: Mat3x3): Mat3x3 {
        return this.setConcat(this, m);
    }

    // Compose this T = M x T
    public postConcat(m: Mat3x3): Mat3x3 {
        return this.setConcat(m, this);
    }

    public toCkMat3x3Array(): CkMat3x3 {
        return [...this.fMat];
    }

    public toCkMatrix(): CkMatrix {
        const m = this.fMat;
        return CkMatrix.All(
            m[0], m[3], m[6],
            m[1], m[4], m[7],
            m[2], m[5], m[8]
        );
    }

    public isFinite(): boolean {
        return numeric.ScalarsAreFinite(this.fMat);
    }

    public hasPerspective(): boolean {
        return !(this.fMat[2] === 0 && this.fMat[5] === 0 && this.fMat[8] === 1);
    }

    public invert(): Maybe<Mat3x3> {
        const result = new Mat3x3();
        if (InvertMat3x3F32(result.fMat, this.fMat) === 0) {
            return Maybe.None();
        }
        return Maybe.Ok(result);
    }

    public transpose(): Mat3x3 {
        const result = new Mat3x3();
        TransposeMat3x3F32(result.fMat, this.fMat);
        return result;
    }

    public transposeSelf(): Mat3x3 {
        TransposeMat3x3F32(this.fMat, this.fMat);
        return this;
    }

    public map(x: number, y: number, z: number): Vector3f {
        const m = this.fMat;
        return new Vector3f(
            x * m[0] + y * m[3] + z * m[6],
            x * m[1] + y * m[4] + z * m[7],
            x * m[2] + y * m[5] + z * m[8]
        );
    }

    // v' = M x v
    public mapVec3(v: Vector3f): Vector3f {
        return this.map(v.x, v.y, v.z);
    }

    // v' = (x, y, 0)
    // v'' = v' x M
    public mapVec2(v: Vector2f): Vector2f {
        return this.map(v.x, v.y, 0).degenerate();
    }

    // Map a homogeneous vector, but perspective is discarded (z = 1)
    public mapAffineVec2(v: Vector2f): Vector2f {
        const x = v.x, y = v.y;
        const m = this.fMat;
        return new Vector2f(
            x * m[0] + y * m[3] + m[6],
            x * m[1] + y * m[4] + m[7]
        );
    }

    public mapPoint(v: Vector2f): Vector2f {
        const m = this.fMat;
        const x = v.x, y = v.y;
        const px = x * m[0] + y * m[3] + m[6],
              py = x * m[1] + y * m[4] + m[7],
              w = x * m[2] + y * m[5] + m[8];
        return new Vector2f(px / w, py / w);
    }

    public mapRect(src: Rect, perspClip: boolean): Rect {
        if (this.hasPerspective() && perspClip) {
            const result = this.toCkMatrix().mapRect(src.toCkArrayXYWHRect(),
                                                     Constants.APPLY_PERSPECTIVE_CLIP_YES);
            return Rect.MakeFromGL(result);
        }

        const v1 = this.mapPoint(src.quadUpperLeft),
              v2 = this.mapPoint(src.quadUpperRight),
              v3 = this.mapPoint(src.quadLowerLeft),
              v4 = this.mapPoint(src.quadLowerRight);

        const x1 = v1.x, x2 = v2.x, x3 = v3.x, x4 = v4.x;
        const y1 = v1.y, y2 = v2.y, y3 = v3.y, y4 = v4.y;

        return Rect.MakeLTRB(
            Math.min(x1, x2, x3, x4),
            Math.min(y1, y2, y3, y4),
            Math.max(x1, x2, x3, x4),
            Math.max(y1, y2, y3, y4)
        );
    }

    // TODO(sora): map points (using low-level and faster implementation, like SIMD)

    // TODO(sora): complete this.
}

// Taken from the gl-matrix project:
// https://github.com/toji/gl-matrix/blob/master/src/mat4.js
function MultiplyMat4x4F32(out: Float32Array, a: Float32Array, b: Float32Array): void {
    let a00 = a[0],
        a01 = a[1],
        a02 = a[2],
        a03 = a[3];
    let a10 = a[4],
        a11 = a[5],
        a12 = a[6],
        a13 = a[7];
    let a20 = a[8],
        a21 = a[9],
        a22 = a[10],
        a23 = a[11];
    let a30 = a[12],
        a31 = a[13],
        a32 = a[14],
        a33 = a[15];

    // Cache only the current line of the second matrix
    let b0 = b[0],
        b1 = b[1],
        b2 = b[2],
        b3 = b[3];
    out[0] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
    out[1] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
    out[2] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
    out[3] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;

    b0 = b[4];
    b1 = b[5];
    b2 = b[6];
    b3 = b[7];
    out[4] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
    out[5] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
    out[6] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
    out[7] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;

    b0 = b[8];
    b1 = b[9];
    b2 = b[10];
    b3 = b[11];
    out[8] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
    out[9] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
    out[10] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
    out[11] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;

    b0 = b[12];
    b1 = b[13];
    b2 = b[14];
    b3 = b[15];
    out[12] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
    out[13] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
    out[14] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
    out[15] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;
}

function Mat4x4DeterminantF32(a: Float32Array): number {
    let a00 = a[0],
        a01 = a[1],
        a02 = a[2],
        a03 = a[3];
    let a10 = a[4],
        a11 = a[5],
        a12 = a[6],
        a13 = a[7];
    let a20 = a[8],
        a21 = a[9],
        a22 = a[10],
        a23 = a[11];
    let a30 = a[12],
        a31 = a[13],
        a32 = a[14],
        a33 = a[15];

    let b0 = a00 * a11 - a01 * a10;
    let b1 = a00 * a12 - a02 * a10;
    let b2 = a01 * a12 - a02 * a11;
    let b3 = a20 * a31 - a21 * a30;
    let b4 = a20 * a32 - a22 * a30;
    let b5 = a21 * a32 - a22 * a31;
    let b6 = a00 * b5 - a01 * b4 + a02 * b3;
    let b7 = a10 * b5 - a11 * b4 + a12 * b3;
    let b8 = a20 * b2 - a21 * b1 + a22 * b0;
    let b9 = a30 * b2 - a31 * b1 + a32 * b0;

    // Calculate the determinant
    return a13 * b6 - a03 * b7 + a33 * b8 - a23 * b9;
}

function InvertMat4x4F32(out: Float32Array, a: Float32Array): number {
    let a00 = a[0],
        a01 = a[1],
        a02 = a[2],
        a03 = a[3];
    let a10 = a[4],
        a11 = a[5],
        a12 = a[6],
        a13 = a[7];
    let a20 = a[8],
        a21 = a[9],
        a22 = a[10],
        a23 = a[11];
    let a30 = a[12],
        a31 = a[13],
        a32 = a[14],
        a33 = a[15];

    let b00 = a00 * a11 - a01 * a10;
    let b01 = a00 * a12 - a02 * a10;
    let b02 = a00 * a13 - a03 * a10;
    let b03 = a01 * a12 - a02 * a11;
    let b04 = a01 * a13 - a03 * a11;
    let b05 = a02 * a13 - a03 * a12;
    let b06 = a20 * a31 - a21 * a30;
    let b07 = a20 * a32 - a22 * a30;
    let b08 = a20 * a33 - a23 * a30;
    let b09 = a21 * a32 - a22 * a31;
    let b10 = a21 * a33 - a23 * a31;
    let b11 = a22 * a33 - a23 * a32;

    // Calculate the determinant
    let det =
        b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;

    if (det === 0) {
        return 0;
    }
    det = 1.0 / det;

    out[0] = (a11 * b11 - a12 * b10 + a13 * b09) * det;
    out[1] = (a02 * b10 - a01 * b11 - a03 * b09) * det;
    out[2] = (a31 * b05 - a32 * b04 + a33 * b03) * det;
    out[3] = (a22 * b04 - a21 * b05 - a23 * b03) * det;
    out[4] = (a12 * b08 - a10 * b11 - a13 * b07) * det;
    out[5] = (a00 * b11 - a02 * b08 + a03 * b07) * det;
    out[6] = (a32 * b02 - a30 * b05 - a33 * b01) * det;
    out[7] = (a20 * b05 - a22 * b02 + a23 * b01) * det;
    out[8] = (a10 * b10 - a11 * b08 + a13 * b06) * det;
    out[9] = (a01 * b08 - a00 * b10 - a03 * b06) * det;
    out[10] = (a30 * b04 - a31 * b02 + a33 * b00) * det;
    out[11] = (a21 * b02 - a20 * b04 - a23 * b00) * det;
    out[12] = (a11 * b07 - a10 * b09 - a12 * b06) * det;
    out[13] = (a00 * b09 - a01 * b07 + a02 * b06) * det;
    out[14] = (a31 * b01 - a30 * b03 - a32 * b00) * det;
    out[15] = (a20 * b03 - a21 * b01 + a22 * b00) * det;

    return det;
}

function TransposeMat4x4F32(dst: Float32Array, src: Float32Array): void {
    dst[0]  = src[0]; dst[1]  = src[4]; dst[2]  = src[8];  dst[3]  = src[12];
    dst[4]  = src[1]; dst[5]  = src[5]; dst[6]  = src[9];  dst[7]  = src[13];
    dst[8]  = src[2]; dst[9]  = src[6]; dst[10] = src[10]; dst[11] = src[14];
    dst[12] = src[3]; dst[13] = src[7]; dst[14] = src[11]; dst[15] = src[15];
}

export class Mat4x4 {
    // Matrix is stored in column-major:
    // Indices:
    //  | 0  4  8  12 |
    //  | 1  5  9  13 |
    //  | 2  6 10  14 |
    //  | 3  7 11  15 |
    private readonly fMat: Float32Array;

    public static Identity(): Mat4x4 {
        return new Mat4x4().refill(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }

    public static RowMajor(r: ArrayLike<number>): Mat4x4 {
        if (r.length < 16) {
            throw Error('The array-like object must have at least 16 elements');
        }
        return new Mat4x4().refill(
            r[ 0], r[ 1], r[ 2], r[ 3],
            r[ 4], r[ 5], r[ 6], r[ 7],
            r[ 8], r[ 9], r[10], r[11],
            r[12], r[13], r[14], r[15]
        );
    }

    public static ColMajor(c: ArrayLike<number>): Mat4x4 {
        if (c.length < 16) {
            throw Error('The array-like object must have at least 16 elements');
        }
        return new Mat4x4().refill(
            c[ 0], c[ 4], c[ 8], c[12],
            c[ 1], c[ 5], c[ 9], c[13],
            c[ 2], c[ 6], c[10], c[14],
            c[ 3], c[ 7], c[11], c[15]
        );
    }

    public static Rows(r0: Vector4f, r1: Vector4f, r2: Vector4f, r3: Vector4f): Mat4x4 {
        const m = new Mat4x4();
        m.setRow(0, r0);
        m.setRow(1, r1);
        m.setRow(2, r2);
        m.setRow(3, r3);
        return m;
    }

    public static Cols(c0: Vector4f, c1: Vector4f, c2: Vector4f, c3: Vector4f): Mat4x4 {
        const m = new Mat4x4();
        m.setCol(0, c0);
        m.setCol(1, c1);
        m.setCol(2, c2);
        m.setCol(3, c3);
        return m;
    }

    public static Translate(x: number, y: number, z: number = 0): Mat4x4 {
        return new Mat4x4().refill(
            1, 0, 0, x,
            0, 1, 0, y,
            0, 0, 1, z,
            0, 0, 0, 1
        );
    }

    public static Scale(x: number, y: number, z: number = 1): Mat4x4 {
        return new Mat4x4().refill(
            x, 0, 0, 0,
            0, y, 0, 0,
            0, 0, z, 0,
            0, 0, 0, 1
        );
    }

    public static FromCkMatrix(mat: CkMatrix): Mat4x4 {
        return new Mat4x4().refill(
            mat.scaleX,  mat.skewX,  0, mat.transX,
            mat.skewY,   mat.scaleY, 0, mat.transY,
            0,           0,          1, 0,
            mat.persp0,  mat.persp1, 0, mat.persp2
        );
    }

    public static FromMat3x3(mat: Mat3x3): Mat4x4 {
        const m = mat.underlyingArray;
        return new Mat4x4().refill(
            m[0], m[3], 0, m[6],
            m[1], m[4], 0, m[7],
               0,    0, 1,    0,
            m[2], m[5], 0, m[8]
        );
    }

    /**
     * Scales and translates `src` to fill `dst` exactly.
     */
    public static RectToRect(src: Rect, dst: Rect): Mat4x4 {
        if (src.isEmpty()) {
            return Mat4x4.Identity();
        }
        if (dst.isEmpty()) {
            return Mat4x4.Scale(0, 0, 0);
        }
        const sx = dst.width / src.width;
        const sy = dst.height / src.height;
        const tx = dst.left - sx * src.left;
        const ty = dst.top - sy * src.top;
        return new Mat4x4().refill(
            sx,  0,  0, tx,
             0, sy,  0, ty,
             0,  0,  1,  0,
             0,  0,  0,  1
        );
    }

    public static Rotate(axis: Vector3f, radians: number): Mat4x4 {
        return new Mat4x4().setRotate(axis, radians);
    }

    public static LookAt(eye: Vector3f, center: Vector3f, up: Vector3f): Mat4x4 {
        const normalize = (v: Vector3f): Vector3f => {
            const len = v.length();
            if (numeric.ScalarNearlyZero(len)) {
                return v;
            }
            return v.mul(1.0 / len);
        };

        const v4 = (v3: Vector3f, w: number): Vector4f => {
            return new Vector4f(v3.x, v3.y, v3.z, w);
        };

        const f = normalize(center.sub(eye));
        const u = normalize(up);
        const s = normalize(f.cross(u));

        const m = Mat4x4.Cols(v4(s, 0), v4(s.cross(f), 0), v4(f.neg(), 0), v4(eye, 1)).invert();
        if (!m.has()) {
            return Mat4x4.Identity();
        }
        return m.unwrap();
    }

    public static Perspective(near: number, far: number, angle: number): Mat4x4 {
        const denomInv = 1.0 / (far - near);
        const halfAngle = angle * 0.5;
        const cot = 1.0 / Math.tan(0.5 * angle);
        return new Mat4x4().refill(
            cot, 0, 0, 0,
            0, cot, 0, 0,
            0, 0, (far + near) * denomInv, 2 * far * near * denomInv,
            0, 0, -1, 1
        );
    }

    public static Concat(a: Mat4x4, b: Mat4x4): Mat4x4 {
        return new Mat4x4().setConcat(a, b);
    }

    private constructor(array: Float32Array = null) {
        if (array == null) {
            this.fMat = new Float32Array(16);
        } else {
            this.fMat = array;
        }
    }

    public get underlyingArray(): Float32Array {
        return this.fMat;
    }

    private refill(m11: number, m12: number, m13: number, m14: number,
                   m21: number, m22: number, m23: number, m24: number,
                   m31: number, m32: number, m33: number, m34: number,
                   m41: number, m42: number, m43: number, m44: number): Mat4x4
    {
        const m = this.fMat;
        m[0] = m11;
        m[1] = m21;
        m[2] = m31;
        m[3] = m41;
        m[4] = m12;
        m[5] = m22;
        m[6] = m32;
        m[7] = m42;
        m[8] = m13;
        m[9] = m23;
        m[10] = m33;
        m[11] = m43;
        m[12] = m14;
        m[13] = m24;
        m[14] = m34;
        m[15] = m44;
        return this;
    }

    public equalTo(other: Mat4x4): boolean {
        const a = this.fMat, b = other;
        return a[0] === b[0] &&
               a[1] === b[1] &&
               a[2] === b[2] &&
               a[3] === b[3] &&
               a[4] === b[4] &&
               a[5] === b[5] &&
               a[6] === b[6] &&
               a[7] === b[7] &&
               a[8] === b[8] &&
               a[9] === b[9] &&
               a[10] === b[10] &&
               a[11] === b[11] &&
               a[12] === b[12] &&
               a[13] === b[13] &&
               a[14] === b[14] &&
               a[15] === b[15];
    }

    public clone(): Mat4x4 {
        return new Mat4x4(new Float32Array([...this.fMat]));
    }

    // Get an element at the r'th row and the c'th column.
    // Note that `r` and `c` are indices begin from 0.
    public at(r: number, c: number): number {
        CheckIntIndex(r, 0, 4);
        CheckIntIndex(c, 0, 4);
        return this.fMat[c * 4 + r];
    }

    public setAt(r: number, c: number, value: number): void {
        CheckIntIndex(r, 0, 4);
        CheckIntIndex(c, 0, 4);
        this.fMat[c * 4 + r] = value;
    }

    public row(i: number): Vector4f {
        CheckIntIndex(i, 0, 4);
        const m = this.fMat;
        return new Vector4f(m[i], m[i + 4], m[i + 8], m[i + 12]);
    }

    public col(i: number): Vector4f {
        CheckIntIndex(i, 0, 4);
        const m = this.fMat;
        const j = i * 4;
        return new Vector4f(m[j], m[j + 1], m[j + 2], m[j + 3]);
    }

    public setRow(i: number, v: Vector4f): void {
        CheckIntIndex(i, 0, 4);
        const m = this.fMat;
        m[i] = v.x;
        m[i + 4] = v.y;
        m[i + 8] = v.z;
        m[i + 12] = v.w;
    }

    public setCol(i: number, v: Vector4f): void {
        CheckIntIndex(i, 0, 4);
        const j = i * 4;
        const m = this.fMat;
        m[j] = v.x;
        m[j + 1] = v.y;
        m[j + 2] = v.z;
        m[j + 3] = v.w;
    }

    public setIdentity(): Mat4x4 {
        this.refill(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
        return this;
    }

    public setTranslate(x: number, y: number, z: number = 0): Mat4x4 {
        this.refill(
            1, 0, 0, x,
            0, 1, 0, y,
            0, 0, 1, z,
            0, 0, 0, 1
        );
        return this;
    }

    public setScale(x: number, y: number, z: number = 1): Mat4x4 {
        this.refill(
            x, 0, 0, 0,
            0, y, 0, 0,
            0, 0, z, 0,
            0, 0, 0, 1
        );
        return this;
    }

    public setRotateUnitSinCos(axis: Vector3f, sinA: number, cosA: number): Mat4x4 {
        // Taken from "Essential Mathematics for Games and Interactive Applications"
        //             James M. Van Verth and Lars M. Bishop -- third edition
        const x = axis.x;
        const y = axis.y;
        const z = axis.z;
        const c = cosA;
        const s = sinA;
        const t = 1 - c;
        this.refill(
            t*x*x + c,   t*x*y - s*z, t*x*z + s*y, 0,
            t*x*y + s*z, t*y*y + c,   t*y*z - s*x, 0,
            t*x*z - s*y, t*y*z + s*x, t*z*z + c,   0,
            0,           0,           0,           1
        );
        return this;
    }

    public setRotateUnit(axis: Vector3f, radians: number): Mat4x4 {
        this.setRotateUnitSinCos(axis, Math.sin(radians), Math.cos(radians));
        return this;
    }

    public setRotate(axis: Vector3f, radians: number): Mat4x4 {
        this.setRotateUnit(axis.normalize(), radians);
        return this;
    }

    // Compose this T = A x B
    public setConcat(a: Mat4x4, b: Mat4x4): Mat4x4 {
        MultiplyMat4x4F32(this.fMat, a.fMat, b.fMat);
        return this;
    }

    // Compose this T = T x M
    public preConcat(m: Mat4x4): Mat4x4 {
        this.setConcat(this, m);
        return this;
    }

    // Compose this T = M x T
    public postConcat(m: Mat4x4): Mat4x4 {
        this.setConcat(m, this);
        return this;
    }

    /**
     *  A matrix is categorized as 'perspective' if the bottom row is not [0, 0, 0, 1].
     *  For most uses, a bottom row of [0, 0, 0, X] behaves like a non-perspective matrix, though
     *  it will be categorized as perspective. Calling normalizePerspective() will change the
     *  matrix such that, if its bottom row was [0, 0, 0, X], it will be changed to [0, 0, 0, 1]
     *  by scaling the rest of the matrix by 1/X.
     *
     *  | A B C D |    | A/X B/X C/X D/X |
     *  | E F G H | -> | E/X F/X G/X H/X |   for X != 0
     *  | I J K L |    | I/X J/X K/X L/X |
     *  | 0 0 0 X |    |  0   0   0   1  |
     */
    public normalizePerspective(): void {
        if (this.fMat[15] === 1 || this.fMat[15] === 0 || this.fMat[3] !== 0 &&
            this.fMat[7] !== 0 || this.fMat[11] !== 0) {
            return;
        }
        const inv = 1.0 / this.fMat[15];
        const m = this.fMat;
        m[0] *= inv;
        m[1] *= inv;
        m[2] *= inv;
        m[3] *= inv;
        m[4] *= inv;
        m[5] *= inv;
        m[6] *= inv;
        m[7] *= inv;
        m[8] *= inv;
        m[9] *= inv;
        m[10] *= inv;
        m[11] *= inv;
        m[12] *= inv;
        m[13] *= inv;
        m[14] *= inv;
        m[15] *= inv;
    }

    // An inverted matrix is returned (if it is invertible), and the original
    // matrix is not touched.
    public invert(): Maybe<Mat4x4> {
        const result = new Mat4x4();
        if (InvertMat4x4F32(result.fMat, this.fMat) === 0) {
            return Maybe.None();
        }
        return Maybe.Ok(result);
    }

    // A transposed matrix is returned, and the original matrix is not touched.
    public transpose(): Mat4x4 {
        const result = new Mat4x4();
        TransposeMat4x4F32(result.fMat, this.fMat);
        return result;
    }

    public transposeSelf(): Mat4x4 {
        TransposeMat4x4F32(this.fMat, this.fMat);
        return this;
    }

    /**
     * As the Glamor CkMatrix is 3x3 matrix, the third row and column is dropped
     * when converting from Mat4x4 to CkMatrix:
     * | A B C D |        | A B D |
     * | E F G H |   ->   | E F H |
     * | I J K L |        | M N P |
     * | M N O P |
     */
    public toCkMatrix(): CkMatrix {
        const m = this.fMat;
        return CkMatrix.All(m[0], m[4], m[12], m[1], m[5], m[13], m[3], m[7], m[15]);
    }

    public toMat3x3(): Mat3x3 {
        const m = this.fMat;
        return Mat3x3.ColMajor([
            m[0], m[1], m[3], m[4], m[5], m[7], m[12], m[13], m[15]
        ]);
    }

    public map(x: number, y: number, z: number, w: number): Vector4f {
        const m = this.fMat;
        return new Vector4f(
            m[0] * x + m[4] * y + m[8] * z + m[12] * w,
            m[1] * x + m[5] * y + m[9] * z + m[13] * w,
            m[2] * x + m[6] * y + m[10] * z + m[14] * w,
            m[3] * x + m[7] * y + m[11] * z + m[15] * w,
        );
    }

    public mapVec3(v: Vector3f): Vector3f {
        return this.map(v.x, v.y, v.z, 0).degenerate();
    }

    public mapVec4(v: Vector4f): Vector4f {
        return this.map(v.x, v.y, v.z, v.w);
    }

    public isFinite(): boolean {
        return numeric.ScalarsAreFinite(this.fMat);
    }

    public determinant(): number {
        return Mat4x4DeterminantF32(this.fMat);
    }
}
