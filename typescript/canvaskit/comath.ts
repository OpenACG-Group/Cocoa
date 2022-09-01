export type Scalar = number;
export type Int = number;

function throwIfFail(cond: boolean, ctor: ErrorConstructor, ...args): void {
    if (!cond) {
        throw new ctor(...args);
    }
}

function scalarsAreFinite(array: Scalar[]): boolean {
    for (let v of array) {
        if (!Number.isFinite(v))
            return false;
    }
    return true;
}

function v4(v: Vector3, w: Scalar): Vector4 {
    return new Vector4(v.x, v.y, v.z, w);
}

/**
 * Linear interpolate in [l, r]: `R = t*r + (1 - t)*l, while t∈[0, 1], R∈[l, r]`.
 * @returns     Value of R.
 */
export function linearInterpolate(l: Scalar, r: Scalar, t: Scalar): Scalar {
    // return t * r + (1 - t) * l;
    return l + (r - l) * t;
}

export class Rect {
    #left: Scalar;
    #top: Scalar;
    #right: Scalar;
    #bottom: Scalar;

    private constructor(left: Scalar, top: Scalar, right: Scalar, bottom: Scalar) {
        this.#left = left;
        this.#top = top;
        this.#right = right;
        this.#bottom = bottom;
    }

    public static MakeEmpty(): Rect {
        return new Rect(0, 0, 0, 0);
    }

    public static MakeWH(w: Scalar, h: Scalar): Rect {
        return new Rect(0, 0, w, h);
    }

    public static MakeLTRB(l: Scalar, t: Scalar, r: Scalar, b: Scalar): Rect {
        return new Rect(l, t, r, b);
    }

    public static MakeXYWH(x: Scalar, y: Scalar, w: Scalar, h: Scalar): Rect {
        return new Rect(x, y, x + w, y + h);
    }

    public static Make(other: Rect): Rect {
        return new Rect(other.#left, other.#top, other.#right, other.#bottom);
    }

    public isEmpty(): boolean {
        return !(this.#left < this.#right && this.#top < this.#bottom);
    }

    public x(): Scalar {
        return this.#left;
    }

    public y(): Scalar {
        return this.#top;
    }

    public left(): Scalar {
        return this.#left;
    }

    public top(): Scalar {
        return this.#top;
    }

    public right(): Scalar {
        return this.#right;
    }

    public bottom(): Scalar {
        return this.#bottom;
    }

    public width(): Scalar {
        return this.#right - this.#left;
    }

    public height(): Scalar {
        return this.#bottom - this.#top;
    }
}

export class Vector2 {
    x: Scalar;
    y: Scalar;

    constructor(x?: Scalar, y?: Scalar) {
        if (x != undefined && y != undefined) {
            this.x = x;
            this.y = y;
        } else {
            this.x = NaN;
            this.y = NaN;
        }
    }

    public static Dot(a: Vector2, b: Vector2): Scalar {
        return (a.x * b.x + a.y * b.y);
    }

    public static Cross(a: Vector2, b: Vector2): Scalar {
        return (a.x * b.y - a.y * b.x);
    }

    public static Normalize(v: Vector2): Vector2 {
        return v.multiply(1.0 / v.length());
    }

    public length(): Scalar {
        return Math.sqrt(Vector2.Dot(this, this));
    }

    public lengthSquared(): Scalar {
        return Vector2.Dot(this, this);
    }

    public dot(v: Vector2): Scalar {
        return Vector2.Dot(this, v);
    }

    public cross(v: Vector2): Scalar {
        return Vector2.Cross(this, v);
    }

    public normalize(): Vector2 {
        return Vector2.Normalize(this);
    }

    public add(v: Vector2): Vector2 {
        return new Vector2(this.x + v.x, this.y + v.y);
    }

    public sub(v: Vector2): Vector2 {
        return new Vector2(this.x - v.x, this.y - v.x);
    }

    public multiply(k: Scalar): Vector2 {
        return new Vector2(this.x * k, this.y * k);
    }
}

export class Vector3 {
    x: Scalar;
    y: Scalar;
    z: Scalar;

    constructor(x?: Scalar, y?: Scalar, z?: Scalar) {
        if (x != undefined && y != undefined && z != undefined) {
            this.x = x;
            this.y = y;
            this.z = z;
        } else {
            this.x = this.y = this.z = NaN;
        }
    }

    public static Dot(a: Vector3, b: Vector3): Scalar {
        return (a.x * b.x + a.y * b.y + a.z * b.z);
    }

    public static Cross(a: Vector3, b: Vector3): Vector3 {
        return new Vector3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }

    public static Normalize(v: Vector3): Vector3 {
        return v.multiply(1.0 / v.length());
    }

    public length(): Scalar {
        return Math.sqrt(Vector3.Dot(this, this));
    }

    public lengthSquared(): Scalar {
        return Vector3.Dot(this, this);
    }

    public dot(v: Vector3): Scalar {
        return Vector3.Dot(this, v);
    }

    public cross(v: Vector3): Vector3 {
        return Vector3.Cross(this, v);
    }

    public normalize(): Vector3 {
        return Vector3.Normalize(this);
    }

    public add(v: Vector3): Vector3 {
        return new Vector3(this.x + v.x, this.y + v.y, this.z + v.z);
    }

    public sub(v: Vector3): Vector3 {
        return new Vector3(this.x - v.x, this.y - v.y, this.z - v.z);
    }

    public multiply(k: Scalar): Vector3 {
        return new Vector3(k * this.x, k * this.y, k * this.z);
    }
}

export class Vector4 {
    x: Scalar;
    y: Scalar;
    z: Scalar;
    w: Scalar;

    constructor(x?: Scalar, y?: Scalar, z?: Scalar, w?: Scalar) {
        if (x != undefined && y != undefined && z != undefined && w != undefined) {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        } else {
            this.x = this.y = this.z = this.w = NaN;
        }
    }

    public add(v: Vector4): Vector4 {
        return new Vector4(
            this.x + v.x,
            this.y + v.y,
            this.z + v.z,
            this.w + v.w
        );
    }

    public sub(v: Vector4): Vector4 {
        return new Vector4(
            this.x - v.x,
            this.y - v.y,
            this.z - v.z,
            this.w - v.w
        );
    }

    public multiply(k: Scalar) {
        return new Vector4(
            this.x * k,
            this.y * k,
            this.z * k,
            this.w * k
        );
    }
}

export interface Matrix4x4InvertResult {
    matrix: Mat4x4;
    determinant: Scalar;
}

/**
 * 4x4 matrix used by Canvas and other parts of rendering.
 * We assume a right-handed coordinate system:
 *      +X goes to the right
 *      +Y goes down
 *      +Z goes into the screen (away from the viewer)
 */
export class Mat4x4 {
    /* Stored in column-major.
     *  Indices
     *  0  4  8  12        1 0 0 trans_x
     *  1  5  9  13  e.g.  0 1 0 trans_y
     *  2  6 10  14        0 0 1 trans_z
     *  3  7 11  15        0 0 0 1
     */
    #mat: Scalar[];

    private constructor(m?: Scalar[]) {
        if (m != undefined) {
            if (m.length != 16) {
                throw RangeError('Mat4x4 requires a Scalar[16] array for construction');
            }
            this.#mat = m;
        } else {
            this.#mat = [
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            ];
        }
    }

    public static FromRows(r0: Vector4, r1: Vector4, r2: Vector4, r3: Vector4): Mat4x4 {
        let m = new Mat4x4();
        m.setRow(0, r0);
        m.setRow(1, r1);
        m.setRow(2, r2);
        m.setRow(3, r3);
        return m;
    }

    public static FromCols(c0: Vector4, c1: Vector4, c2: Vector4, c3: Vector4): Mat4x4 {
        let m = new Mat4x4();
        m.setCol(0, c0);
        m.setCol(1, c1);
        m.setCol(2, c2);
        m.setCol(3, c3);
        return m;
    }

    public static FromRowMajor(m: Scalar[]) {
        throwIfFail(m.length == 16, RangeError, 'Invalid matrix array');
        return new Mat4x4(m);
    }

    public static FromColMajor(m: Scalar[]) {
        throwIfFail(m.length == 16, RangeError, 'Invalid matrix array');
        return new Mat4x4([
            m[0], m[4], m[8], m[12],
            m[1], m[5], m[9], m[13],
            m[2], m[6], m[7], m[14],
            m[3], m[7], m[8], m[15]
        ]);
    }

    public static Translate(x: Scalar, y: Scalar, z: Scalar = 0): Mat4x4 {
        return new Mat4x4([
            1, 0, 0, x,
            0, 1, 0, y,
            0, 0, 1, z,
            0, 0, 0, 1
        ]);
    }

    public static Scale(x: Scalar, y: Scalar, z: Scalar = 1): Mat4x4 {
        return new Mat4x4([
            x, 0, 0, 0,
            0, y, 0, 0,
            0, 0, z, 0,
            0, 0, 0, 1
        ]);
    }

    public static Rotate(axis: Vector3, radians: Scalar): Mat4x4 {
        let matrix = new Mat4x4();
        matrix.setRotate(axis, radians);
        return matrix;
    }

    public static Perspective(near: Scalar, far: Scalar, angle: Scalar): Mat4x4 {
        throwIfFail(far > near, RangeError, 'far > near assertion failed');

        let inv = 1.0 / (far - near);
        let halfAngle = angle * 0.5;
        let cot = Math.cos(halfAngle) / Math.sin(halfAngle);

        let matrix = new Mat4x4();
        matrix.setRC(0, 0, cot);
        matrix.setRC(1, 1, cot);
        matrix.setRC(2, 2, (far + near) * inv);
        matrix.setRC(2, 3, 2 * far * near * inv);
        matrix.setRC(3, 2, -1);

        return matrix;
    }

    public static LookAt(camera: Vector3, center: Vector3, up: Vector3): Mat4x4 {
        let f = center.sub(camera).normalize();
        let u = up.normalize();
        let s = f.cross(u).normalize();

        let invMatResult = Mat4x4.FromCols(
            v4(s, 0),
            v4(s.cross(f), 0),
            v4(f.multiply(-1), 0),
            v4(camera, 1)
        ).invert();

        let matrix = invMatResult.matrix;
        if (invMatResult.determinant == 0) {
            matrix.setIdentity();
        }

        return matrix;
    }

    public row(i: number): Vector4 {
        throwIfFail(i >= 0 && i <= 3, RangeError, 'Invalid row index');
        return new Vector4(this.#mat[i], this.#mat[i + 4],
                           this.#mat[i + 8], this.#mat[i + 12]);
    }

    public col(i: number): Vector4 {
        throwIfFail(i >= 0 && i <= 3, RangeError, 'Invalid col index');
        return new Vector4(this.#mat[4 * i], this.#mat[4 * i + 1],
                           this.#mat[4 * i + 2], this.#mat[4 * i + 3]);
    }

    public setRow(i: number, v: Vector4): void {
        throwIfFail(i >= 0 && i <= 3, RangeError, 'Mat4x4.setRow requires an index in [0, 4)');
        this.#mat[i    ]  = v.x;
        this.#mat[i + 4]  = v.y;
        this.#mat[i + 8]  = v.z;
        this.#mat[i + 12] = v.w;
    }

    public setCol(i: number, v: Vector4): void {
        throwIfFail(i >= 0 && i <= 3, RangeError, 'Mat4x4.setCol requires an index in [0, 4)');
        this.#mat[4 * i    ] = v.x;
        this.#mat[4 * i + 1] = v.y;
        this.#mat[4 * i + 2] = v.z;
        this.#mat[4 * i + 3] = v.w;
    }

    public setRC(r: number, c: number, v: Scalar): void {
        throwIfFail(r >= 0 && r <= 3, RangeError, 'Invalid row number');
        throwIfFail(c >= 0 && c <= 3, RangeError, 'Invalid column number');
        this.#mat[c * 4 + r] = v;
    }

    public setIdentity(): void {
        this.#mat = [
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        ];
    }

    public setRotateUnitSinCos(axis: Vector3, sinAngle: Scalar, cosAngle: Scalar): void {
        let x = axis.x, y = axis.y, z = axis.z;
        let c = cosAngle, s = sinAngle;
        let t = 1 - c;

        this.#mat = [
            t*x*x + c,   t*x*y - s*z, t*x*z + s*y, 0,
            t*x*y + s*z, t*y*y + c,   t*y*z - s*x, 0,
            t*x*z - s*y, t*y*z + s*x, t*z*z + c,   0,
            0,           0,           0,           1
        ];
    }

    public setRotateUnit(axis: Vector3, radians: Scalar): void {
        this.setRotateUnitSinCos(axis, Math.sin(radians), Math.cos(radians));
    }

    public setRotate(axis: Vector3, radians: Scalar): void {
        let len = axis.length();
        if (len > 0 && !Number.isFinite(len)) {
            this.setRotateUnit(axis.normalize(), radians);
        } else {
            this.setIdentity();
        }
    }

    public invert(): Matrix4x4InvertResult {
        let m = new Mat4x4();

        let a00 = this.#mat[0];
        let a01 = this.#mat[1];
        let a02 = this.#mat[2];
        let a03 = this.#mat[3];
        let a10 = this.#mat[4];
        let a11 = this.#mat[5];
        let a12 = this.#mat[6];
        let a13 = this.#mat[7];
        let a20 = this.#mat[8];
        let a21 = this.#mat[9];
        let a22 = this.#mat[10];
        let a23 = this.#mat[11];
        let a30 = this.#mat[12];
        let a31 = this.#mat[13];
        let a32 = this.#mat[14];
        let a33 = this.#mat[15];

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
        let determinant = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
        let invdet = 1.0 / determinant;
        b00 *= invdet;
        b01 *= invdet;
        b02 *= invdet;
        b03 *= invdet;
        b04 *= invdet;
        b05 *= invdet;
        b06 *= invdet;
        b07 *= invdet;
        b08 *= invdet;
        b09 *= invdet;
        b10 *= invdet;
        b11 *= invdet;

        m.#mat[0]  = a11 * b11 - a12 * b10 + a13 * b09;
        m.#mat[1]  = a02 * b10 - a01 * b11 - a03 * b09;
        m.#mat[2]  = a31 * b05 - a32 * b04 + a33 * b03;
        m.#mat[3]  = a22 * b04 - a21 * b05 - a23 * b03;
        m.#mat[4]  = a12 * b08 - a10 * b11 - a13 * b07;
        m.#mat[5]  = a00 * b11 - a02 * b08 + a03 * b07;
        m.#mat[6]  = a32 * b02 - a30 * b05 - a33 * b01;
        m.#mat[7]  = a20 * b05 - a22 * b02 + a23 * b01;
        m.#mat[8]  = a10 * b10 - a11 * b08 + a13 * b06;
        m.#mat[9]  = a01 * b08 - a00 * b10 - a03 * b06;
        m.#mat[10] = a30 * b04 - a31 * b02 + a33 * b00;
        m.#mat[11] = a21 * b02 - a20 * b04 - a23 * b00;
        m.#mat[12] = a11 * b07 - a10 * b09 - a12 * b06;
        m.#mat[13] = a00 * b09 - a01 * b07 + a02 * b06;
        m.#mat[14] = a31 * b01 - a30 * b03 - a32 * b00;
        m.#mat[15] = a20 * b03 - a21 * b01 + a22 * b00;

        // If 1/det overflows to infinity (i.e. det is denormalized) or any of the inverted matrix
        // values is non-finite, return zero to indicate a non-invertible matrix.
        if (!scalarsAreFinite(m.#mat)) {
            determinant = 0.0;
        }

        return { matrix: m, determinant: determinant };
    }

    public toString(): string {
        let maxWidth = 0;
        let result = '';

        for (let v of this.#mat) {
            let len = v.toString().length;
            if (len > maxWidth)
                maxWidth = len;
        }

        for (let r = 0; r < 4; r++) {
            result += '|';
            for (let c = 0; c < 4; c++) {
                let str = this.#mat[c * 4 + r].toString();
                result += str;
                if (c != 3)
                    result += ' '.repeat(maxWidth - str.length + 1);
            }
            result += '|\n';
        }

        return result;
    }
}
