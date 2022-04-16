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
var _Mat4x4_mat;
function throwIfFail(cond, ctor, ...args) {
    if (!cond) {
        throw new ctor(...args);
    }
}
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
