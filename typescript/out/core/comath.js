var __classPrivateFieldSet = (this && this.__classPrivateFieldSet) || function (receiver, state, value, kind, f) {
    if (kind === "m") throw new TypeError("Private method is not writable");
    if (kind === "a" && !f) throw new TypeError("Private accessor was defined without a setter");
    if (typeof state === "function" ? receiver !== state || !f : !state.has(receiver)) throw new TypeError("Cannot write private member to an object whose class did not declare it");
    return (kind === "a" ? f.call(receiver, value) : f ? f.value = value : state.set(receiver, value)), value;
};
var __classPrivateFieldGet = (this && this.__classPrivateFieldGet) || function (receiver, state, kind, f) {
    if (kind === "a" && !f) throw new TypeError("Private accessor was defined without a getter");
    if (typeof state === "function" ? receiver !== state || !f : !state.has(receiver)) throw new TypeError("Cannot read private member from an object whose class did not declare it");
    return kind === "m" ? f : kind === "a" ? f.call(receiver) : f ? f.value : state.get(receiver);
};
var _Rect_left, _Rect_top, _Rect_right, _Rect_bottom, _Mat4x4_mat;
function throwIfFail(cond, ctor, ...args) {
    if (!cond) {
        throw new ctor(...args);
    }
}
function scalarsAreFinite(array) {
    for (let v of array) {
        if (!Number.isFinite(v))
            return false;
    }
    return true;
}
function v4(v, w) {
    return new Vector4(v.x, v.y, v.z, w);
}
export class Rect {
    constructor(left, top, right, bottom) {
        _Rect_left.set(this, void 0);
        _Rect_top.set(this, void 0);
        _Rect_right.set(this, void 0);
        _Rect_bottom.set(this, void 0);
        __classPrivateFieldSet(this, _Rect_left, left, "f");
        __classPrivateFieldSet(this, _Rect_top, top, "f");
        __classPrivateFieldSet(this, _Rect_right, right, "f");
        __classPrivateFieldSet(this, _Rect_bottom, bottom, "f");
    }
    static MakeEmpty() {
        return new Rect(0, 0, 0, 0);
    }
    static MakeWH(w, h) {
        return new Rect(0, 0, w, h);
    }
    static MakeLTRB(l, t, r, b) {
        return new Rect(l, t, r, b);
    }
    static MakeXYWH(x, y, w, h) {
        return new Rect(x, y, x + w, y + h);
    }
    static Make(other) {
        return new Rect(__classPrivateFieldGet(other, _Rect_left, "f"), __classPrivateFieldGet(other, _Rect_top, "f"), __classPrivateFieldGet(other, _Rect_right, "f"), __classPrivateFieldGet(other, _Rect_bottom, "f"));
    }
    isEmpty() {
        return !(__classPrivateFieldGet(this, _Rect_left, "f") < __classPrivateFieldGet(this, _Rect_right, "f") && __classPrivateFieldGet(this, _Rect_top, "f") < __classPrivateFieldGet(this, _Rect_bottom, "f"));
    }
    x() {
        return __classPrivateFieldGet(this, _Rect_left, "f");
    }
    y() {
        return __classPrivateFieldGet(this, _Rect_top, "f");
    }
    left() {
        return __classPrivateFieldGet(this, _Rect_left, "f");
    }
    top() {
        return __classPrivateFieldGet(this, _Rect_top, "f");
    }
    right() {
        return __classPrivateFieldGet(this, _Rect_right, "f");
    }
    bottom() {
        return __classPrivateFieldGet(this, _Rect_bottom, "f");
    }
    width() {
        return __classPrivateFieldGet(this, _Rect_right, "f") - __classPrivateFieldGet(this, _Rect_left, "f");
    }
    height() {
        return __classPrivateFieldGet(this, _Rect_bottom, "f") - __classPrivateFieldGet(this, _Rect_top, "f");
    }
}
_Rect_left = new WeakMap(), _Rect_top = new WeakMap(), _Rect_right = new WeakMap(), _Rect_bottom = new WeakMap();
export class Vector2 {
    constructor(x, y) {
        if (x != undefined && y != undefined) {
            this.x = x;
            this.y = y;
        }
        else {
            this.x = NaN;
            this.y = NaN;
        }
    }
    static Dot(a, b) {
        return (a.x * b.x + a.y * b.y);
    }
    static Cross(a, b) {
        return (a.x * b.y - a.y * b.x);
    }
    static Normalize(v) {
        return v.multiply(1.0 / v.length());
    }
    length() {
        return Math.sqrt(Vector2.Dot(this, this));
    }
    lengthSquared() {
        return Vector2.Dot(this, this);
    }
    dot(v) {
        return Vector2.Dot(this, v);
    }
    cross(v) {
        return Vector2.Cross(this, v);
    }
    normalize() {
        return Vector2.Normalize(this);
    }
    add(v) {
        return new Vector2(this.x + v.x, this.y + v.y);
    }
    sub(v) {
        return new Vector2(this.x - v.x, this.y - v.x);
    }
    multiply(k) {
        return new Vector2(this.x * k, this.y * k);
    }
}
export class Vector3 {
    constructor(x, y, z) {
        if (x != undefined && y != undefined && z != undefined) {
            this.x = x;
            this.y = y;
            this.z = z;
        }
        else {
            this.x = this.y = this.z = NaN;
        }
    }
    static Dot(a, b) {
        return (a.x * b.x + a.y * b.y + a.z * b.z);
    }
    static Cross(a, b) {
        return new Vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
    }
    static Normalize(v) {
        return v.multiply(1.0 / v.length());
    }
    length() {
        return Math.sqrt(Vector3.Dot(this, this));
    }
    lengthSquared() {
        return Vector3.Dot(this, this);
    }
    dot(v) {
        return Vector3.Dot(this, v);
    }
    cross(v) {
        return Vector3.Cross(this, v);
    }
    normalize() {
        return Vector3.Normalize(this);
    }
    add(v) {
        return new Vector3(this.x + v.x, this.y + v.y, this.z + v.z);
    }
    sub(v) {
        return new Vector3(this.x - v.x, this.y - v.y, this.z - v.z);
    }
    multiply(k) {
        return new Vector3(k * this.x, k * this.y, k * this.z);
    }
}
export class Vector4 {
    constructor(x, y, z, w) {
        if (x != undefined && y != undefined && z != undefined && w != undefined) {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }
        else {
            this.x = this.y = this.z = this.w = NaN;
        }
    }
    add(v) {
        return new Vector4(this.x + v.x, this.y + v.y, this.z + v.z, this.w + v.w);
    }
    sub(v) {
        return new Vector4(this.x - v.x, this.y - v.y, this.z - v.z, this.w - v.w);
    }
    multiply(k) {
        return new Vector4(this.x * k, this.y * k, this.z * k, this.w * k);
    }
}
/**
 * 4x4 matrix used by Canvas and other parts of rendering.
 * We assume a right-handed coordinate system:
 *      +X goes to the right
 *      +Y goes down
 *      +Z goes into the screen (away from the viewer)
 */
export class Mat4x4 {
    constructor(m) {
        /* Stored in column-major.
         *  Indices
         *  0  4  8  12        1 0 0 trans_x
         *  1  5  9  13  e.g.  0 1 0 trans_y
         *  2  6 10  14        0 0 1 trans_z
         *  3  7 11  15        0 0 0 1
         */
        _Mat4x4_mat.set(this, void 0);
        if (m != undefined) {
            if (m.length != 16) {
                throw RangeError('Mat4x4 requires a Scalar[16] array for construction');
            }
            __classPrivateFieldSet(this, _Mat4x4_mat, m, "f");
        }
        else {
            __classPrivateFieldSet(this, _Mat4x4_mat, [
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            ], "f");
        }
    }
    static FromRows(r0, r1, r2, r3) {
        let m = new Mat4x4();
        m.setRow(0, r0);
        m.setRow(1, r1);
        m.setRow(2, r2);
        m.setRow(3, r3);
        return m;
    }
    static FromCols(c0, c1, c2, c3) {
        let m = new Mat4x4();
        m.setCol(0, c0);
        m.setCol(1, c1);
        m.setCol(2, c2);
        m.setCol(3, c3);
        return m;
    }
    static FromRowMajor(m) {
        throwIfFail(m.length == 16, RangeError, 'Invalid matrix array');
        return new Mat4x4(m);
    }
    static FromColMajor(m) {
        throwIfFail(m.length == 16, RangeError, 'Invalid matrix array');
        return new Mat4x4([
            m[0], m[4], m[8], m[12],
            m[1], m[5], m[9], m[13],
            m[2], m[6], m[7], m[14],
            m[3], m[7], m[8], m[15]
        ]);
    }
    static Translate(x, y, z = 0) {
        return new Mat4x4([
            1, 0, 0, x,
            0, 1, 0, y,
            0, 0, 1, z,
            0, 0, 0, 1
        ]);
    }
    static Scale(x, y, z = 1) {
        return new Mat4x4([
            x, 0, 0, 0,
            0, y, 0, 0,
            0, 0, z, 0,
            0, 0, 0, 1
        ]);
    }
    static Rotate(axis, radians) {
        let matrix = new Mat4x4();
        matrix.setRotate(axis, radians);
        return matrix;
    }
    static Perspective(near, far, angle) {
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
    static LookAt(camera, center, up) {
        let f = center.sub(camera).normalize();
        let u = up.normalize();
        let s = f.cross(u).normalize();
        let invMatResult = Mat4x4.FromCols(v4(s, 0), v4(s.cross(f), 0), v4(f.multiply(-1), 0), v4(camera, 1)).invert();
        let matrix = invMatResult.matrix;
        if (invMatResult.determinant == 0) {
            matrix.setIdentity();
        }
        return matrix;
    }
    row(i) {
        throwIfFail(i >= 0 && i <= 3, RangeError, 'Invalid row index');
        return new Vector4(__classPrivateFieldGet(this, _Mat4x4_mat, "f")[i], __classPrivateFieldGet(this, _Mat4x4_mat, "f")[i + 4], __classPrivateFieldGet(this, _Mat4x4_mat, "f")[i + 8], __classPrivateFieldGet(this, _Mat4x4_mat, "f")[i + 12]);
    }
    col(i) {
        throwIfFail(i >= 0 && i <= 3, RangeError, 'Invalid col index');
        return new Vector4(__classPrivateFieldGet(this, _Mat4x4_mat, "f")[4 * i], __classPrivateFieldGet(this, _Mat4x4_mat, "f")[4 * i + 1], __classPrivateFieldGet(this, _Mat4x4_mat, "f")[4 * i + 2], __classPrivateFieldGet(this, _Mat4x4_mat, "f")[4 * i + 3]);
    }
    setRow(i, v) {
        throwIfFail(i >= 0 && i <= 3, RangeError, 'Mat4x4.setRow requires an index in [0, 4)');
        __classPrivateFieldGet(this, _Mat4x4_mat, "f")[i] = v.x;
        __classPrivateFieldGet(this, _Mat4x4_mat, "f")[i + 4] = v.y;
        __classPrivateFieldGet(this, _Mat4x4_mat, "f")[i + 8] = v.z;
        __classPrivateFieldGet(this, _Mat4x4_mat, "f")[i + 12] = v.w;
    }
    setCol(i, v) {
        throwIfFail(i >= 0 && i <= 3, RangeError, 'Mat4x4.setCol requires an index in [0, 4)');
        __classPrivateFieldGet(this, _Mat4x4_mat, "f")[4 * i] = v.x;
        __classPrivateFieldGet(this, _Mat4x4_mat, "f")[4 * i + 1] = v.y;
        __classPrivateFieldGet(this, _Mat4x4_mat, "f")[4 * i + 2] = v.z;
        __classPrivateFieldGet(this, _Mat4x4_mat, "f")[4 * i + 3] = v.w;
    }
    setRC(r, c, v) {
        throwIfFail(r >= 0 && r <= 3, RangeError, 'Invalid row number');
        throwIfFail(c >= 0 && c <= 3, RangeError, 'Invalid column number');
        __classPrivateFieldGet(this, _Mat4x4_mat, "f")[c * 4 + r] = v;
    }
    setIdentity() {
        __classPrivateFieldSet(this, _Mat4x4_mat, [
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        ], "f");
    }
    setRotateUnitSinCos(axis, sinAngle, cosAngle) {
        let x = axis.x, y = axis.y, z = axis.z;
        let c = cosAngle, s = sinAngle;
        let t = 1 - c;
        __classPrivateFieldSet(this, _Mat4x4_mat, [
            t * x * x + c, t * x * y - s * z, t * x * z + s * y, 0,
            t * x * y + s * z, t * y * y + c, t * y * z - s * x, 0,
            t * x * z - s * y, t * y * z + s * x, t * z * z + c, 0,
            0, 0, 0, 1
        ], "f");
    }
    setRotateUnit(axis, radians) {
        this.setRotateUnitSinCos(axis, Math.sin(radians), Math.cos(radians));
    }
    setRotate(axis, radians) {
        let len = axis.length();
        if (len > 0 && !Number.isFinite(len)) {
            this.setRotateUnit(axis.normalize(), radians);
        }
        else {
            this.setIdentity();
        }
    }
    invert() {
        let m = new Mat4x4();
        let a00 = __classPrivateFieldGet(this, _Mat4x4_mat, "f")[0];
        let a01 = __classPrivateFieldGet(this, _Mat4x4_mat, "f")[1];
        let a02 = __classPrivateFieldGet(this, _Mat4x4_mat, "f")[2];
        let a03 = __classPrivateFieldGet(this, _Mat4x4_mat, "f")[3];
        let a10 = __classPrivateFieldGet(this, _Mat4x4_mat, "f")[4];
        let a11 = __classPrivateFieldGet(this, _Mat4x4_mat, "f")[5];
        let a12 = __classPrivateFieldGet(this, _Mat4x4_mat, "f")[6];
        let a13 = __classPrivateFieldGet(this, _Mat4x4_mat, "f")[7];
        let a20 = __classPrivateFieldGet(this, _Mat4x4_mat, "f")[8];
        let a21 = __classPrivateFieldGet(this, _Mat4x4_mat, "f")[9];
        let a22 = __classPrivateFieldGet(this, _Mat4x4_mat, "f")[10];
        let a23 = __classPrivateFieldGet(this, _Mat4x4_mat, "f")[11];
        let a30 = __classPrivateFieldGet(this, _Mat4x4_mat, "f")[12];
        let a31 = __classPrivateFieldGet(this, _Mat4x4_mat, "f")[13];
        let a32 = __classPrivateFieldGet(this, _Mat4x4_mat, "f")[14];
        let a33 = __classPrivateFieldGet(this, _Mat4x4_mat, "f")[15];
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
        __classPrivateFieldGet(m, _Mat4x4_mat, "f")[0] = a11 * b11 - a12 * b10 + a13 * b09;
        __classPrivateFieldGet(m, _Mat4x4_mat, "f")[1] = a02 * b10 - a01 * b11 - a03 * b09;
        __classPrivateFieldGet(m, _Mat4x4_mat, "f")[2] = a31 * b05 - a32 * b04 + a33 * b03;
        __classPrivateFieldGet(m, _Mat4x4_mat, "f")[3] = a22 * b04 - a21 * b05 - a23 * b03;
        __classPrivateFieldGet(m, _Mat4x4_mat, "f")[4] = a12 * b08 - a10 * b11 - a13 * b07;
        __classPrivateFieldGet(m, _Mat4x4_mat, "f")[5] = a00 * b11 - a02 * b08 + a03 * b07;
        __classPrivateFieldGet(m, _Mat4x4_mat, "f")[6] = a32 * b02 - a30 * b05 - a33 * b01;
        __classPrivateFieldGet(m, _Mat4x4_mat, "f")[7] = a20 * b05 - a22 * b02 + a23 * b01;
        __classPrivateFieldGet(m, _Mat4x4_mat, "f")[8] = a10 * b10 - a11 * b08 + a13 * b06;
        __classPrivateFieldGet(m, _Mat4x4_mat, "f")[9] = a01 * b08 - a00 * b10 - a03 * b06;
        __classPrivateFieldGet(m, _Mat4x4_mat, "f")[10] = a30 * b04 - a31 * b02 + a33 * b00;
        __classPrivateFieldGet(m, _Mat4x4_mat, "f")[11] = a21 * b02 - a20 * b04 - a23 * b00;
        __classPrivateFieldGet(m, _Mat4x4_mat, "f")[12] = a11 * b07 - a10 * b09 - a12 * b06;
        __classPrivateFieldGet(m, _Mat4x4_mat, "f")[13] = a00 * b09 - a01 * b07 + a02 * b06;
        __classPrivateFieldGet(m, _Mat4x4_mat, "f")[14] = a31 * b01 - a30 * b03 - a32 * b00;
        __classPrivateFieldGet(m, _Mat4x4_mat, "f")[15] = a20 * b03 - a21 * b01 + a22 * b00;
        // If 1/det overflows to infinity (i.e. det is denormalized) or any of the inverted matrix
        // values is non-finite, return zero to indicate a non-invertible matrix.
        if (!scalarsAreFinite(__classPrivateFieldGet(m, _Mat4x4_mat, "f"))) {
            determinant = 0.0;
        }
        return { matrix: m, determinant: determinant };
    }
    toString() {
        let maxWidth = 0;
        let result = '';
        for (let v of __classPrivateFieldGet(this, _Mat4x4_mat, "f")) {
            let len = v.toString().length;
            if (len > maxWidth)
                maxWidth = len;
        }
        for (let r = 0; r < 4; r++) {
            result += '|';
            for (let c = 0; c < 4; c++) {
                let str = __classPrivateFieldGet(this, _Mat4x4_mat, "f")[c * 4 + r].toString();
                result += str;
                if (c != 3)
                    result += ' '.repeat(maxWidth - str.length + 1);
            }
            result += '|\n';
        }
        return result;
    }
}
_Mat4x4_mat = new WeakMap();
