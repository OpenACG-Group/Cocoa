// %scope UserExecute:forbidden UserImport:forbidden SysExecute:allowed

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
'use strict';

(function(natives) {

    /**
     * Returns false if any of the element in `scalars` is infinity or NaN, otherwise,
     * returns true.
     */
    function ScalarsAreFinite(scalars) {
        for (let i = 0; i < scalars.length; i++) {
            if (!Number.isFinite(scalars[i]) || Number.isNaN(scalars[i])) {
                return false;
            }
        }
        return true;
    }

    const kNEARLY_ZERO = 1.0 / (1 << 12);
    function ScalarNearlyZero(x, tolerance = kNEARLY_ZERO) {
        if (tolerance < 0) {
            throw RangeError('Tolerance of `ScalarNearlyZero` must not be < 0');
        }
        return Math.abs(x) <= tolerance;
    }

    //! TSDecl: @class Vec2
    class Vec2 {
        // Memory layout: [f32 x, f32 y]
        __mem__

        //! TSDecl: @constructor(x: f32, y: f32)
        constructor(x, y) {
            this.__mem__ = new Float32Array(2)
            this.__mem__[0] = x
            this.__mem__[1] = y
        }

        //! TSDecl: @method @static Dot(u: Vec2, v: Vec2): f32
        static Dot(u, v) {
            return (u.__mem__[0] * v.__mem__[0] + u.__mem__[1] * v.__mem__[1]);
        }

        //! TSDecl: @method @static Cross(u: Vec2, v: Vec2): f32
        static Cross(u, v) {
            return (u.__mem__[0] * v.__mem__[1] - v.__mem__[0] * u.__mem__[1]);
        }

        //! TSDecl: @method @static AngleCos(u: Vec2, v: Vec2): f32
        static AngleCos(u, v) {
            return (Vec2.Dot(u, v) / (u.length() * v.length()));
        }

        //! TSDecl: @method @static AngleSin(u: Vec2, v: Vec2): f32
        static AngleSin(u, v) {
            const C = Vec2.AngleCos(u, v);
            return Math.sqrt(1 - C * C);
        }

        //! TSDecl: @property @readonly x: f32
        get x() {
            return this.__mem__[0];
        }

        //! TSDecl: @property @readonly y: f32
        get y() {
            return this.__mem__[1];
        }

        //! TSDecl: @method length(): f32
        length() {
            return Math.sqrt(this.__mem__[0] * this.__mem__[0] + this.__mem__[1] * this.__mem__[1]);
        }

        //! TSDecl: @method lengthSquared(): f32
        lengthSquared() {
            return (this.__mem__[0] * this.__mem__[0] + this.__mem__[1] * this.__mem__[1]);
        }

        //! TSDecl: @method add(v: Vec2): Vec2
        add(v) {
            return new Vec2(this.__mem__[0] + v.__mem__[0], this.__mem__[1] + v.__mem__[1]);
        }

        //! TSDecl: @method sub(v: Vec2): Vec2
        sub(v) {
            return new Vec2(this.__mem__[0] - v.__mem__[0], this.__mem__[1] - v.__mem__[1]);
        }

        //! TSDecl: @method dot(v: Vec2): f32
        dot(v) {
            return Vec2.Dot(this, v);
        }

        //! TSDecl: @method cross(v: Vec2): f32
        cross(v) {
            return Vec2.Cross(this, v);
        }

        //! TSDecl: @method normalize(): Vec2
        normalize() {
            const L = this.length();
            return new Vec2(this.__mem__[0] / L, this.__mem__[1] / L);
        }

        //! TSDecl: @method neg(): Vec2
        neg() {
            return new Vec2(-this.__mem__[0], -this.__mem__[1]);
        }

        //! TSDecl: @method mul(lambda: f32): Vec2
        mul(lambda) {
            return new Vec2(lambda * this.__mem__[0], lambda * this.__mem__[1]);
        }

        //! TSDecl: @method rotate(rad: f32): Vec2
        rotate(rad) {
            return new Vec2(this.__mem__[0] * Math.cos(rad) - this.__mem__[1] * Math.sin(rad),
                this.__mem__[0] * Math.sin(rad) + this.__mem__[1] * Math.cos(rad));
        }

        //! TSDecl: @method angleCos(v: Vec2): Vec2
        angleCos(v) {
            return Vec2.AngleCos(this, v);
        }

        //! TSDecl: @method angleSin(v: Vec2): Vec2
        angleSin(v) {
            return Vec2.AngleSin(this, v);
        }

        //! TSDecl: @method equalTo(other: Vec2): boolean
        equalTo(other) {
            return this.__mem__[0] === other.__mem__[0] && this.__mem__[1] === other.__mem__[1];
        }

        //! TSDecl: @method clone(): Vec2
        clone() {
            return new Vec2(this.__mem__[0], this.__mem__[1]);
        }
    }
    //! TSDecl: @end

    //! TSDecl: @enum VecDegenerate
    const VecDegenerate = Object.freeze({
        //! TSDecl: @enumitem Discard
        Discard: 0,
        //! TSDecl: @enumitem Div
        Div: 1,
        //! TSDecl: @enumitem Mul
        Mul: 2,
        __proto__: null
    });
    //! TSDecl: @end

    //! TSDecl: @class Vec3
    class Vec3 {
        // Memory layout: [f32 x, f32 y, f32 z]
        __mem__

        //! TSDecl: @constructor (x: f32, y: f32, z: f32)
        constructor(x, y, z) {
            this.__mem__ = new Float32Array(3)
            this.__mem__[0] = x
            this.__mem__[1] = y
            this.__mem__[2] = z
        }

        //! TSDecl: @method @static Dot(u: Vec3, v: Vec3): f32
        static Dot(u, v) {
            return (u.__mem__[0] * v.__mem__[0] + u.__mem__[1] * v.__mem__[1] + u.__mem__[2] * v.__mem__[2]);
        }

        //! TSDecl: @method @static Cross(u: Vec3, v: Vec3): Vec3
        static Cross(u, v) {
            return new Vec3(u.__mem__[1] * v.__mem__[2] - v.__mem__[1] * u.__mem__[2],
                            v.__mem__[0] * u.__mem__[2] - u.__mem__[0] * v.__mem__[2],
                            u.__mem__[0] * v.__mem__[1] - v.__mem__[0] * u.__mem__[1]);
        }

        //! TSDecl: @method @static AngleCos(u: Vec3, v: Vec3): f32
        static AngleCos(u, v) {
            return (Vec3.Dot(u, v) / (u.length() * v.length()));
        }

        //! TSDecl: @method @static AngleSin(u: Vec3, v: Vec3): f32
        static AngleSin(u, v) {
            const C = Vec3.AngleCos(u, v);
            return Math.sqrt(1 - C * C);
        }

        //! TSDecl: @property @readonly x: f32
        get x() {
            return this.__mem__[0];
        }

        //! TSDecl: @property @readonly y: f32
        get y() {
            return this.__mem__[1];
        }

        //! TSDecl: @property @readonly z: f32
        get z() {
            return this.__mem__[2];
        }

        //! TSDecl: @method length(): f32
        length() {
            return Math.sqrt(Vec3.Dot(this, this));
        }

        //! TSDecl: @method lengthSquared(): f32
        lengthSquared() {
            return Vec3.Dot(this, this);
        }

        //! TSDecl: @method add(v: Vec3): Vec3
        add(v) {
            return new Vec3(this.__mem__[0] + v.__mem__[0],
                            this.__mem__[1] + v.__mem__[1],
                            this.__mem__[2] + v.__mem__[2]);
        }

        //! TSDecl: @method sub(v: Vec3): Vec3
        sub(v) {
            return new Vec3(this.__mem__[0] - v.__mem__[0],
                            this.__mem__[1] - v.__mem__[1],
                            this.__mem__[2] - v.__mem__[2]);
        }

        //! TSDecl: @method dot(v: Vec3): f32
        dot(v) {
            return Vec3.Dot(this, v);
        }

        //! TSDecl: @method cross(v: Vec3): Vec3
        cross(v) {
            return Vec3.Cross(this, v);
        }

        //! TSDecl: @method normalize(): Vec3
        normalize() {
            const L = this.length();
            return new Vec3(this.__mem__[0] / L, this.__mem__[1] / L, this.__mem__[2] / L);
        }

        //! TSDecl: @method neg(): Vec3
        neg() {
            return new Vec3(-this.__mem__[0], -this.__mem__[1], -this.__mem__[2]);
        }

        //! TSDecl: @method mul(lambda: f32): Vec3
        mul(lambda) {
            return new Vec3(lambda * this.__mem__[0],
                            lambda * this.__mem__[1],
                            lambda * this.__mem__[2]);
        }

        //! TSDecl: @method angleCos(v: Vec3): f32
        angleCos(v) {
            return Vec3.AngleCos(this, v);
        }

        //! TSDecl: @method angleSin(v: Vec3): f32
        angleSin(v) {
            return Vec3.AngleSin(this, v);
        }

        //! TSDecl: @method equalTo(other: Vec3): boolean
        equalTo(other) {
            return (this.__mem__[0] === other.__mem__[0] &&
                    this.__mem__[1] === other.__mem__[1] &&
                    this.__mem__[2] === other.__mem__[2]);
        }

        //! TSDecl: @method clone(): Vec3
        clone() {
            return new Vec3(this.__mem__[0], this.__mem__[1], this.__mem__[2]);
        }

        //! TSDecl: @method degenerate(opt: VecDegenerate): Vec2
        degenerate(opt) {
            if (opt === VecDegenerate.Discard) {
                return new Vec2(this.__mem__[0], this.__mem__[1]);
            } else if (opt === VecDegenerate.Div) {
                const k = 1 / this.__mem__[2];
                return new Vec2(k * this.__mem__[0], k * this.__mem__[1]);
            } else if (opt === VecDegenerate.Mul) {
                return new Vec2(this.__mem__[0] * this.__mem__[2], this.__mem__[1] * this.__mem__[2]);
            } else {
                throw RangeError('invalid enumeration value for `VecDegenerate`');
            }
        }
    }
    //! TSDecl: @end

    //! TSDecl: @class Vec4
    class Vec4 {
        // Memory layout: [f32 x, f32 y, f32 z, f32 w]
        __mem__

        //! TSDecl: @constructor (x: f32, y: f32, z: f32, w: f32)
        constructor(x, y, z, w) {
            this.__mem__ = new Float32Array(4)
            this.__mem__[0] = x
            this.__mem__[1] = y
            this.__mem__[2] = z
            this.__mem__[3] = w
        }

        //! TSDecl: @method @static Dot(a: f32, b: f32): f32
        static Dot(a, b) {
            return (a.__mem__[0] * b.__mem__[0] +
                    a.__mem__[1] * b.__mem__[1] +
                    a.__mem__[2] * b.__mem__[2] +
                    a.__mem__[3] * b.__mem__[3]);
        }

        //! TSDecl: @property @readonly x: f32
        get x() {
            return this.__mem__[0];
        }

        //! TSDecl: @property @readonly y: f32
        get y() {
            return this.__mem__[1];
        }

        //! TSDecl: @property @readonly z: f32
        get z() {
            return this.__mem__[2];
        }

        //! TSDecl: @property @readonly w: f32
        get w() {
            return this.__mem__[3];
        }

        //! TSDecl: @method clone(): Vec4
        clone() {
            return new Vec4(this.__mem__[0], this.__mem__[1], this.__mem__[2], this.__mem__[3]);
        }

        //! TSDecl: @method add(v: Vec4): Vec4
        add(v) {
            return new Vec4(this.__mem__[0] + v.__mem__[0],
                            this.__mem__[1] + v.__mem__[1],
                            this.__mem__[2] + v.__mem__[2],
                            this.__mem__[3] + v.__mem__[3]);
        }

        //! TSDecl: @method sub(v: Vec4): Vec4
        sub(v) {
            return new Vec4(this.__mem__[0] - v.__mem__[0],
                            this.__mem__[1] - v.__mem__[1],
                            this.__mem__[2] - v.__mem__[2],
                            this.__mem__[3] - v.__mem__[3]);
        }

        //! TSDecl: @method neg(): Vec4
        neg() {
            return new Vec4(-this.__mem__[0], -this.__mem__[1], -this.__mem__[2], -this.__mem__[3]);
        }

        //! TSDecl: @method mul(lambda: f32): Vec4
        mul(lambda) {
            return new Vec4(
                this.__mem__[0] * lambda,
                this.__mem__[1] * lambda,
                this.__mem__[2] * lambda,
                this.__mem__[3] * lambda
            );
        }

        //! TSDecl: @method lengthSquared(): f32
        lengthSquared() {
            return (this.__mem__[0] * this.__mem__[0] +
                    this.__mem__[1] * this.__mem__[1] +
                    this.__mem__[2] * this.__mem__[2] +
                    this.__mem__[3] * this.__mem__[3]);
        }

        //! TSDecl: @method length(): f32
        length() {
            return Math.sqrt(this.lengthSquared());
        }

        //! TSDecl: @method normalize(): Vec4
        normalize() {
            const f = 1.0 / this.length();
            return new Vec4(
                this.__mem__[0] * f,
                this.__mem__[1] * f,
                this.__mem__[2] * f,
                this.__mem__[3] * f
            );
        }

        //! TSDecl: @method dot(v: Vec4): f32
        dot(v) {
            return Vec4.Dot(this, v);
        }

        //! TSDecl: @method equalTo(v: Vec4): boolean
        equalTo(v) {
            return (this.__mem__[0] === v.__mem__[0] &&
                    this.__mem__[1] === v.__mem__[1] &&
                    this.__mem__[2] === v.__mem__[2] &&
                    this.__mem__[3] === v.__mem__[3]);
        }

        //! TSDecl: @method degenerate(opt: VecDegenerate): Vec3
        degenerate(opt) {
            if (opt === VecDegenerate.Discard) {
                return new Vec3(this.__mem__[0], this.__mem__[1], this.__mem__[2]);
            } else if (opt === VecDegenerate.Div) {
                const k = 1 / this.__mem__[3];
                return new Vec3(k * this.__mem__[0], k * this.__mem__[1], k * this.__mem__[2]);
            } else if (opt === VecDegenerate.Mul) {
                const p = this.__mem__[3];
                return new Vec3(p * this.__mem__[0], p * this.__mem__[1], p * this.__mem__[2]);
            } else {
                throw RangeError('invalid enumeration value for `VecDegenerate`');
            }
        }
    }
    //! TSDecl: @end

    //! TSDecl: @class @nonconstructible Rect
    class Rect {
        // Memory layout: [f32 left, f32 top, f32 right, f32 bottom]
        __mem__

        //! TSDecl: @method @static MakeXYWH(x: f32, y: f32, w: f32, h: f32): Rect
        static MakeXYWH(x, y, w, h) {
            return new Rect(x, y, x + w, y + h);
        }

        //! TSDecl: @method @static MakeLTRB(l: f32, t: f32, r: f32, b: f32): Rect
        static MakeLTRB(l, t, r, b) {
            return new Rect(l, t, r, b);
        }

        //! TSDecl: @method @static MakeWH(w: f32, h: f32): Rect
        static MakeWH(w, h) {
            return new Rect(0, 0, w, h);
        }

        //! TSDecl: @method @static MakeEmpty(): Rect
        static MakeEmpty() {
            return new Rect(0, 0, 0, 0);
        }

        //! TSDecl: @method @static Clone(from: Rect): Rect
        static Clone(from) {
            return new Rect(from.__mem__[0], from.__mem__[1], from.__mem__[2], from.__mem__[3]);
        }

        //! TSDecl: @method @static Union(a: Rect, b: Rect): Rect
        static Union(a, b) {
            if (a.isEmpty() && b.isEmpty()) {
                return Rect.MakeEmpty();
            }
            return new Rect(
                a.__mem__[0] < b.__mem__[0] ? a.__mem__[0] : b.__mem__[0],
                a.__mem__[1] < b.__mem__[1] ? a.__mem__[1] : b.__mem__[1],
                a.__mem__[2] > b.__mem__[2] ? a.__mem__[2] : b.__mem__[2],
                a.__mem__[3] > b.__mem__[3] ? a.__mem__[3] : b.__mem__[3]
            );
        }

        //! TSDecl: @method @static Intersect(a: Rect, b: Rect): Rect
        static Intersect(a, b) {
            if (a.isEmpty() || b.isEmpty()) {
                return Rect.MakeEmpty();
            }

            if (a.__mem__[0] > b.__mem__[0]) {
                const t = a;
                a = b;
                b = t;
            }
            const x = b.__mem__[0], w = a.__mem__[2] - b.__mem__[0];
            if (w <= 0) {
                return this.MakeEmpty();
            }

            if (a.__mem__[1] > b.__mem__[1]) {
                const t = a;
                a = b;
                b = t;
            }
            const y = b.__mem__[1], h = a.__mem__[3] - b.__mem__[1];
            if (h <= 0) {
                return this.MakeEmpty();
            }

            return this.MakeXYWH(x, y, w, h);
        }

        constructor(L, T, R, B) {
            this.__mem__ = new Float32Array(4);
            this.__mem__[0] = L
            this.__mem__[1] = T
            this.__mem__[2] = R
            this.__mem__[3] = B
        }

        //! TSDecl: @property @readonly top: f32
        get top() {
            return this.__mem__[1];
        }

        //! TSDecl: @property @readonly left: f32
        get left() {
            return this.__mem__[0];
        }

        //! TSDecl: @property @readonly right: f32
        get right() {
            return this.__mem__[2];
        }

        //! TSDecl: @property @readonly bottom: f32
        get bottom() {
            return this.__mem__[3];
        }

        //! TSDecl: @property @readonly x: f32
        get x() {
            return this.__mem__[0];
        }

        //! TSDecl: @property @readonly y: f32
        get y() {
            return this.__mem__[1];
        }

        //! TSDecl: @property @readonly width: f32
        get width() {
            return (this.__mem__[2] - this.__mem__[0]);
        }

        //! TSDecl: @property @readonly height: f32
        get height() {
            return (this.__mem__[3] - this.__mem__[1]);
        }

        //! TSDecl: @property @readonly center: Vec2
        get center() {
            return new Vec2(this.__mem__[0] / 2 + this.__mem__[2] / 2,
                                this.__mem__[1] / 2 + this.__mem__[3] / 2);
        }

        //! TSDecl: @property @readonly quadUpperLeft: Vec2
        get quadUpperLeft() {
            return new Vec2(this.__mem__[0], this.__mem__[1]);
        }

        //! TSDecl: @property @readonly quadUpperRight: Vec2
        get quadUpperRight() {
            return new Vec2(this.__mem__[2], this.__mem__[1]);
        }

        //! TSDecl: @property @readonly quadLowerLeft: Vec2
        get quadLowerLeft() {
            return new Vec2(this.__mem__[0], this.__mem__[3]);
        }

        //! TSDecl: @property @readonly quadLowerRight: Vec2
        get quadLowerRight() {
            return new Vec2(this.__mem__[2], this.__mem__[3]);
        }

        //! TSDecl: @method equalTo(other: Rect): boolean
        equalTo(other) {
            if (this.isEmpty() || other.isEmpty()) {
                return false;
            }
            return (this.__mem__[0] === other.__mem__[0] &&
                    this.__mem__[1] === other.__mem__[1] &&
                    this.__mem__[2] === other.__mem__[2] &&
                    this.__mem__[3] === other.__mem__[3]);
        }

        //! TSDecl: @method isEmpty(): boolean
        isEmpty() {
            return (this.width === 0 && this.height === 0);
        }

        //! TSDecl: @method makeWH(): Rect
        makeWH() {
            return Rect.MakeWH(this.width, this.height);
        }

        //! TSDecl: @method clone(): Rect
        clone() {
            return Rect.Clone(this);
        }

        //! TSDecl: @method union(other: Rect): Rect
        union(other) {
            return Rect.Union(this, other);
        }

        //! TSDecl: @method makeOffset(dx: f32, dy: f32): Rect
        makeOffset(dx, dy) {
            return new Rect(this.__mem__[0] + dx,
                            this.__mem__[1] + dy,
                            this.__mem__[2] + dx,
                            this.__mem__[3] + dy);
        }

        //! TSDecl: @method makeOffsetv(offset: Vec2): Rect
        makeOffsetv(offset) {
            return this.makeOffset(offset.x, offset.y);
        }

        //! TSDecl: @method makeInset(dx: f32, dy: f32): Rect
        makeInset(dx, dy) {
            return new Rect(this.__mem__[0] + dx,
                            this.__mem__[1] + dy,
                            this.__mem__[2] - dx,
                            this.__mem__[3] - dy);
        }

        //! TSDecl: @method makeOutset(dx: f32, dy: f32): Rect
        makeOutset(dx, dy) {
            return new Rect(this.__mem__[0] - dx,
                            this.__mem__[1] - dy,
                            this.__mem__[2] + dx,
                            this.__mem__[3] + dy);
        }

        //! TSDecl: @method contains(x: f32, y: f32): boolean
        contains(x, y) {
            return (x >= this.__mem__[0] && x < this.__mem__[2] && y >= this.__mem__[1] && y < this.__mem__[3]);
        }

        //! TSDecl: @method containsRect(other: Rect): boolean
        containsRect(other) {
            return (!this.isEmpty() && !other.isEmpty() &&
                    this.__mem__[0] <= other.__mem__[0] && this.__mem__[1] <= other.__mem__[1] &&
                    this.__mem__[2] >= other.__mem__[2] && this.__mem__[3] >= other.__mem__[3]);
        }

        //! TSDecl: @method intersect(other: Rect): Rect
        intersect(other) {
            return Rect.Intersect(this, other);
        }
    }
    //! TSDecl: @end
    
    // Throw a `RangeError` if `i` is not an integer in [l, h)
    function CheckIntIndex(i, l, h) {
        if (!Number.isInteger(i) || i < l || i >= h) {
            throw RangeError(`Invalid index value: non-integer or out of range [${l}, ${h})`);
        }
    }
    
    // Taken from the gl-matrix project:
    // https://github.com/toji/gl-matrix/blob/master/src/mat3.js
    function MultiplyMat3x3F32(out, a, b) {
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
    
    function Mat3x3DeterminantF32(a) {
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
    function InvertMat3x3F32(out, a) {
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
    
    function TransposeMat3x3F32(out, a) {
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
    
    //! TSDecl: @class @nonconstructible Mat3x3
    class Mat3x3 {
        // Stored in column-major:
        // Indices:
        // | 0 3 6 |
        // | 1 4 7 |
        // | 2 5 8 |
        __mem__

        //! TSDecl: @method @static Identity(): Mat3x3
        static Identity() {
            return new Mat3x3().#refill(
                1, 0, 0,
                0, 1, 0,
                0, 0, 1
            );
        }

        //! TSDecl: @method @static RowMajor(r: @generic(ArrayLike, f32)): Mat3x3
        static RowMajor(r) {
            return new Mat3x3().#refill(
                r[0], r[1], r[2],
                r[3], r[4], r[5],
                r[6], r[7], r[8]
            );
        }

        //! TSDecl: @method @static ColMajor(r: @generic(ArrayLike, f32)): Mat3x3
        static ColMajor(r) {
            return new Mat3x3().#refill(
                r[0], r[3], r[6],
                r[1], r[4], r[7],
                r[2], r[5], r[8]
            );
        }

        //! TSDecl: @method @static Rows(r0: Vec3, r1: Vec3, r2: Vec3): Mat3x3
        static Rows(r0, r1, r2) {
            return new Mat3x3().#refill(
                r0.x, r0.y, r0.z,
                r1.x, r1.y, r1.z,
                r2.x, r2.y, r2.z
            );
        }

        //! TSDecl: @method @static Cols(c0: Vec3, c1: Vec3, c2: Vec3): Mat3x3
        static Cols(c0, c1, c2) {
            return new Mat3x3().#refill(
                c0.x, c1.x, c2.x,
                c0.y, c1.y, c2.y,
                c0.z, c1.z, c2.z
            );
        }

        //! TSDecl: @method @static Translate(x: f32, y: f32): Mat3x3
        static Translate(x, y) {
            return new Mat3x3().setTranslate(x, y);
        }

        //! TSDecl: @method @static Scale(sx: f32, sy: f32): Mat3x3
        static Scale(sx, sy) {
            return new Mat3x3().setScale(sx, sy);
        }

        //! TSDecl: @method @static Rotate(center: Vec2, rad: f32): Mat3x3
        static Rotate(center, rad) {
            return new Mat3x3().setRotate(center, rad);
        }

        //! TSDecl: @method @static Concat(a: Mat3x3, b: Mat3x3): Mat3x3
        static Concat(a, b) {
            return new Mat3x3().setConcat(a, b);
        }
    
        constructor(array = null) {
            if (array == null) {
                this.__mem__ = new Float32Array(9);
            } else {
                this.__mem__ = array;
            }
        }
    
        #refill(m11, m12, m13, m21, m22, m23, m31, m32, m33) {
            const m = this.__mem__;
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

        //! TSDecl: @property @readonly underlyingArray: @mem(f32)
        get underlyingArray() {
            return this.__mem__;
        }

        //! TSDecl: @method equalTo(other: Mat3x3): boolean
        equalTo(other) {
            const a = this.__mem__, b = other.__mem__;
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

        //! TSDecl: @method clone(): Mat3x3
        clone() {
            return new Mat3x3(new Float32Array([...this.__mem__]));
        }

        //! TSDecl: @method at(r: i32, c: i32): f32
        at(r, c) {
            CheckIntIndex(r, 0, 3);
            CheckIntIndex(c, 0, 3);
            return this.__mem__[r + 3 * c];
        }

        //! TSDecl: @method setAt(r: i32, c: i32, value: f32): void
        setAt(r, c, value) {
            CheckIntIndex(r, 0, 3);
            CheckIntIndex(c, 0, 3);
            this.__mem__[r + 3 * c] = value;
        }

        //! TSDecl: @method row(i: i32): Vec3
        row(i) {
            CheckIntIndex(i, 0, 3);
            const m = this.__mem__;
            return new Vec3(m[i], m[i + 3], m[i + 6]);
        }

        //! TSDecl: @method setRow(i: i32, r: Vec3): void
        setRow(i, r) {
            CheckIntIndex(i, 0, 3);
            this.__mem__[i] = r.x;
            this.__mem__[i + 3] = r.y;
            this.__mem__[i + 6] = r.z;
        }

        //! TSDecl: @method col(i: i32): Vec3
        col(i) {
            CheckIntIndex(i, 0, 3);
            const m = this.__mem__;
            const j = i * 3;
            return new Vec3(m[j], m[j + 1], m[j + 2]);
        }

        //! TSDecl: @method setCol(i: i32, c: Vec3): void
        setCol(i, c) {
            CheckIntIndex(i, 0, 3);
            const j = i * 3;
            this.__mem__[j] = c.x;
            this.__mem__[j + 1] = c.y;
            this.__mem__[j + 2] = c.z;
        }

        //! TSDecl: @method determinant(): f32
        determinant() {
            return Mat3x3DeterminantF32(this.__mem__);
        }

        //! TSDecl: @method setIdentity(): Mat3x3
        setIdentity() {
            return this.#refill(
                1, 0, 0,
                0, 1, 0,
                0, 0, 1
            );
        }

        //! TSDecl: @method setTranslate(x: f32, y: f32): Mat3x3
        setTranslate(x, y) {
            return this.#refill(
                1, 0, x,
                0, 1, y,
                0, 0, 1
            );
        }

        //! TSDecl: @method setScale(x: f32, y: f32): Mat3x3
        setScale(x, y) {
            return this.#refill(
                x, 0, 0,
                0, y, 0,
                0, 0, 1
            );
        }

        //! TSDecl: @method setRotate(center: Vec2, rad: f32): Mat3x3
        setRotate(center, rad) {
            const s = Math.sin(rad);
            const c = Math.cos(rad);
            const t = 1 - c;
            const x = center.x;
            const y = center.y;
    
            // Rotate matrix:
            // | cosA, -sinA,  ysinA + x(1 - cosA) |
            // | sinA,  cosA, -xsinA + y(1 - cosA) |
            // |    0,     0,                    1 |
            return this.#refill(
                c, -s,  s * y + t * x,
                s,  c, -s * x + t * y,
                0, 0, 1
            );
        }

        //! TSDecl: @method setConcat(a: Mat3x3, b: Mat3x3): Mat3x3
        // Compose this T = A x B
        setConcat(a, b) {
            MultiplyMat3x3F32(this.__mem__, a.__mem__, b.__mem__);
            return this;
        }

        //! TSDecl: @method preConcat(m: Mat3x3): Mat3x3
        // Compose this T = T x M
        preConcat(m) {
            return this.setConcat(this, m);
        }

        //! TSDecl: @method postConcat(m: Mat3x3): Mat3x3
        // Compose this T = M x T
        postConcat(m) {
            return this.setConcat(m, this);
        }

        //! TSDecl: @method isFinite(): boolean
        isFinite() {
            return ScalarsAreFinite(this.__mem__);
        }

        //! TSDecl: @method hasPerspective(): boolean
        hasPerspective() {
            return !(this.__mem__[2] === 0 && this.__mem__[5] === 0 && this.__mem__[8] === 1);
        }

        //! TSDecl: @method invert(): @union(Mat3x3, null)
        invert() {
            const result = new Mat3x3();
            if (InvertMat3x3F32(result.__mem__, this.__mem__) === 0) {
                return null;
            }
            return result;
        }

        //! TSDecl: @method transpose(): Mat3x3
        transpose() {
            const result = new Mat3x3();
            TransposeMat3x3F32(result.__mem__, this.__mem__);
            return result;
        }

        //! TSDecl: @method transposeSelf(): Mat3x3
        transposeSelf() {
            TransposeMat3x3F32(this.__mem__, this.__mem__);
            return this;
        }

        //! TSDecl: @method map(x: f32, y: f32, z: f32): Vec3
        map(x, y, z) {
            const m = this.__mem__;
            return new Vec3(
                x * m[0] + y * m[3] + z * m[6],
                x * m[1] + y * m[4] + z * m[7],
                x * m[2] + y * m[5] + z * m[8]
            );
        }

        //! TSDecl: @method mapVec3(v: Vec3): Vec3
        // v' = M x v
        mapVec3(v) {
            return this.map(v.x, v.y, v.z);
        }

        //! TSDecl: @method mapVec2(v: Vec2): Vec2
        // v' = (x, y, 0)
        // v'' = v' x M
        mapVec2(v) {
            return this.map(v.x, v.y, 0).degenerate(VecDegenerate.Discard);
        }

        //! TSDecl: @method mapAffineVec2(v: Vec2): Vec2
        // Map a homogeneous vector, but perspective is discarded (z = 1)
        mapAffineVec2(v) {
            const x = v.x, y = v.y;
            const m = this.__mem__;
            return new Vec2(
                x * m[0] + y * m[3] + m[6],
                x * m[1] + y * m[4] + m[7]
            );
        }

        //! TSDecl: @method mapPoint(v: Vec2): Vec2
        mapPoint(v) {
            const m = this.__mem__;
            const x = v.x, y = v.y;
            const px = x * m[0] + y * m[3] + m[6],
                  py = x * m[1] + y * m[4] + m[7],
                  w = x * m[2] + y * m[5] + m[8];
            return new Vec2(px / w, py / w);
        }

        //! TSDecl: @method mapAffineRect(src: Rect): Rect
        mapAffineRect(src) {
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
    }
    //! TSDecl: @end


    // Taken from the gl-matrix project:
    // https://github.com/toji/gl-matrix/blob/master/src/mat4.js
    function MultiplyMat4x4F32(out, a, b) {
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
    
    function Mat4x4DeterminantF32(a) {
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
    
    function InvertMat4x4F32(out, a) {
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
    
    function TransposeMat4x4F32(dst, src) {
        dst[0]  = src[0]; dst[1]  = src[4]; dst[2]  = src[8];  dst[3]  = src[12];
        dst[4]  = src[1]; dst[5]  = src[5]; dst[6]  = src[9];  dst[7]  = src[13];
        dst[8]  = src[2]; dst[9]  = src[6]; dst[10] = src[10]; dst[11] = src[14];
        dst[12] = src[3]; dst[13] = src[7]; dst[14] = src[11]; dst[15] = src[15];
    }

    //! TSDecl: @class @nonconstructible Mat4x4
    class Mat4x4 {
        // Matrix is stored in column-major:
        // Indices:
        //  | 0  4  8  12 |
        //  | 1  5  9  13 |
        //  | 2  6 10  14 |
        //  | 3  7 11  15 |
        __mem__

        //! TSDecl: @method @static Identity(): Mat4x4
        static Identity() {
            return new Mat4x4().#refill(
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            );
        }

        //! TSDecl: @method @static RowMajor(r: @generic(ArrayLike, f32)): Mat4x4
        static RowMajor(r) {
            if (r.length < 16) {
                throw Error('The array-like object must have at least 16 elements');
            }
            return new Mat4x4().#refill(
                r[ 0], r[ 1], r[ 2], r[ 3],
                r[ 4], r[ 5], r[ 6], r[ 7],
                r[ 8], r[ 9], r[10], r[11],
                r[12], r[13], r[14], r[15]
            );
        }

        //! TSDecl: @method @static ColMajor(c: @generic(ArrayLike, f32)): Mat4x4
        static ColMajor(c) {
            if (c.length < 16) {
                throw Error('The array-like object must have at least 16 elements');
            }
            return new Mat4x4().#refill(
                c[ 0], c[ 4], c[ 8], c[12],
                c[ 1], c[ 5], c[ 9], c[13],
                c[ 2], c[ 6], c[10], c[14],
                c[ 3], c[ 7], c[11], c[15]
            );
        }

        //! TSDecl: @method @static Rows(r0: Vec4, r1: Vec4, r2: Vec4, r3: Vec4): Mat4x4
        static Rows(r0, r1, r2, r3) {
            const m = new Mat4x4();
            m.setRow(0, r0);
            m.setRow(1, r1);
            m.setRow(2, r2);
            m.setRow(3, r3);
            return m;
        }

        //! TSDecl: @method @static Cols(c0: Vec4, c1: Vec4, c2: Vec4, c3: Vec4): Mat4x4
        static Cols(c0, c1, c2, c3) {
            const m = new Mat4x4();
            m.setCol(0, c0);
            m.setCol(1, c1);
            m.setCol(2, c2);
            m.setCol(3, c3);
            return m;
        }

        //! TSDecl: @method @static Translate(x: f32, y: f32, z: f32): Mat4x4
        static Translate(x, y, z) {
            return new Mat4x4().#refill(
                1, 0, 0, x,
                0, 1, 0, y,
                0, 0, 1, z,
                0, 0, 0, 1
            );
        }

        //! TSDecl: @method @static Scale(x: f32, y: f32, z: f32): Mat4x4
        static Scale(x, y, z) {
            return new Mat4x4().#refill(
                x, 0, 0, 0,
                0, y, 0, 0,
                0, 0, z, 0,
                0, 0, 0, 1
            );
        }
    
        /**
         * Scales and translates `src` to fill `dst` exactly.
         */
        //! TSDecl: @method @static RectToRect(src: Rect, dst: Rect): Mat4x4
        static RectToRect(src, dst) {
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
            return new Mat4x4().#refill(
                sx,  0,  0, tx,
                 0, sy,  0, ty,
                 0,  0,  1,  0,
                 0,  0,  0,  1
            );
        }

        //! TSDecl: @method @static Rotate(axis: Vec3, radians: f32): Mat4x4
        static Rotate(axis, radians) {
            return new Mat4x4().setRotate(axis, radians);
        }

        //! TSDecl: @method @static LookAt(eye: Vec3, center: Vec3, up: Vec3): Mat4x4
        static LookAt(eye, center, up) {
            const normalize = (v) => {
                const len = v.length();
                if (ScalarNearlyZero(len)) {
                    return v;
                }
                return v.mul(1.0 / len);
            };
    
            const v4 = (v3, w) => {
                return new Vec4(v3.x, v3.y, v3.z, w);
            };
    
            const f = normalize(center.sub(eye));
            const u = normalize(up);
            const s = normalize(f.cross(u));
    
            const m = Mat4x4.Cols(v4(s, 0), v4(s.cross(f), 0), v4(f.neg(), 0), v4(eye, 1)).invert();
            if (m == null) {
                return Mat4x4.Identity();
            }
            return m;
        }

        //! TSDecl: @method @static Perspective(near: f32, far: f32, angle: f32): Mat4x4
        static Perspective(near, far, angle) {
            const denomInv = 1.0 / (far - near);
            const halfAngle = angle * 0.5;
            const cot = 1.0 / Math.tan(0.5 * angle);
            return new Mat4x4().#refill(
                cot, 0, 0, 0,
                0, cot, 0, 0,
                0, 0, (far + near) * denomInv, 2 * far * near * denomInv,
                0, 0, -1, 1
            );
        }

        //! TSDecl: @method @static Concat(a: Mat4x4, b: Mat4x4): Mat4x4
        static Concat(a, b) {
            return new Mat4x4().setConcat(a, b);
        }
    
        constructor(array = null) {
            if (array == null) {
                this.__mem__ = new Float32Array(16);
            } else {
                this.__mem__ = array;
            }
        }

        //! TSDecl: @property @readonly underlyingArray: @mem(f32)
        get underlyingArray() {
            return this.__mem__;
        }
    
        #refill(m11, m12, m13, m14, m21, m22, m23, m24, m31, m32, m33, m34, m41, m42, m43, m44)
        {
            const m = this.__mem__;
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

        //! TSDecl: @method equalTo(other: Mat4x4): boolean
        equalTo(other) {
            const a = this.__mem__, b = other;
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

        //! TSDecl: @method clone(): Mat4x4
        clone() {
            return new Mat4x4(new Float32Array([...this.__mem__]));
        }

        // Get an element at the r'th row and the c'th column.
        // Note that `r` and `c` are indices begin from 0.
        //! TSDecl: @method at(r: i32, c: i32): f32
        at(r, c) {
            CheckIntIndex(r, 0, 4);
            CheckIntIndex(c, 0, 4);
            return this.__mem__[c * 4 + r];
        }

        //! TSDecl: @method setAt(r: i32, c: i32, value: f32): void
        setAt(r, c, value) {
            CheckIntIndex(r, 0, 4);
            CheckIntIndex(c, 0, 4);
            this.__mem__[c * 4 + r] = value;
        }

        //! TSDecl: @method row(i: i32): Vec4
        row(i) {
            CheckIntIndex(i, 0, 4);
            const m = this.__mem__;
            return new Vector4f(m[i], m[i + 4], m[i + 8], m[i + 12]);
        }

        //! TSDecl: @method col(i: i32): Vec4
        col(i) {
            CheckIntIndex(i, 0, 4);
            const m = this.__mem__;
            const j = i * 4;
            return new Vector4f(m[j], m[j + 1], m[j + 2], m[j + 3]);
        }

        //! TSDecl: @method setRow(i: i32, v: Vec4): void
        setRow(i, v) {
            CheckIntIndex(i, 0, 4);
            const m = this.__mem__;
            m[i] = v.x;
            m[i + 4] = v.y;
            m[i + 8] = v.z;
            m[i + 12] = v.w;
        }

        //! TSDecl: @method setCol(i: i32, v: Vec4): void
        setCol(i, v) {
            CheckIntIndex(i, 0, 4);
            const j = i * 4;
            const m = this.__mem__;
            m[j] = v.x;
            m[j + 1] = v.y;
            m[j + 2] = v.z;
            m[j + 3] = v.w;
        }

        //! TSDecl: @method setIdentity(): Mat4x4
        setIdentity() {
            this.#refill(
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            );
            return this;
        }

        //! TSDecl: @method setTranslate(x: f32, y: f32, z: f32): Mat4x4
        setTranslate(x, y, z) {
            this.#refill(
                1, 0, 0, x,
                0, 1, 0, y,
                0, 0, 1, z,
                0, 0, 0, 1
            );
            return this;
        }

        //! TSDecl: @method setScale(x: f32, y: f32, z: f32): Mat4x4
        setScale(x, y, z) {
            this.#refill(
                x, 0, 0, 0,
                0, y, 0, 0,
                0, 0, z, 0,
                0, 0, 0, 1
            );
            return this;
        }

        //! TSDecl: @method setRotateUnitSinCos(axis: Vec3, sinA: f32, cosA: f32): Mat4x4
        setRotateUnitSinCos(axis, sinA, cosA) {
            // Taken from "Essential Mathematics for Games and Interactive Applications"
            //             James M. Van Verth and Lars M. Bishop -- third edition
            const x = axis.x;
            const y = axis.y;
            const z = axis.z;
            const c = cosA;
            const s = sinA;
            const t = 1 - c;
            this.#refill(
                t*x*x + c,   t*x*y - s*z, t*x*z + s*y, 0,
                t*x*y + s*z, t*y*y + c,   t*y*z - s*x, 0,
                t*x*z - s*y, t*y*z + s*x, t*z*z + c,   0,
                0,           0,           0,           1
            );
            return this;
        }

        //! TSDecl: @method setRotateUnit(axis: Vec3, radians: f32): Mat4x4
        setRotateUnit(axis, radians) {
            this.setRotateUnitSinCos(axis, Math.sin(radians), Math.cos(radians));
            return this;
        }

        //! TSDecl: @method setRotate(axis: Vec3, radians: f32): Mat4x4
        setRotate(axis, radians) {
            this.setRotateUnit(axis.normalize(), radians);
            return this;
        }
    
        // Compose this T = A x B
        //! TSDecl: @method setConcat(a: Mat4x4, b: Mat4x4): Mat4x4
        setConcat(a, b) {
            MultiplyMat4x4F32(this.__mem__, a.__mem__, b.__mem__);
            return this;
        }
    
        // Compose this T = T x M
        //! TSDecl: @method preConcat(m: Mat4x4): Mat4x4
        preConcat(m) {
            this.setConcat(this, m);
            return this;
        }
    
        // Compose this T = M x T
        //! TSDecl: @method postConcat(m: Mat4x4): Mat4x4
        postConcat(m) {
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
        //! TSDecl: @method normalizePerspective(): void
        normalizePerspective() {
            if (this.__mem__[15] === 1 || this.__mem__[15] === 0 || this.__mem__[3] !== 0 &&
                this.__mem__[7] !== 0 || this.__mem__[11] !== 0) {
                return;
            }
            const inv = 1.0 / this.__mem__[15];
            const m = this.__mem__;
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
        //! TSDecl: @method invert(): @union(Mat4x4, null)
        invert() {
            const result = new Mat4x4();
            if (InvertMat4x4F32(result.__mem__, this.__mem__) === 0) {
                return null;
            }
            return result;
        }
    
        // A transposed matrix is returned, and the original matrix is not touched.
        //! TSDecl: @method transpose(): Mat4x4
        transpose() {
            const result = new Mat4x4();
            TransposeMat4x4F32(result.__mem__, this.__mem__);
            return result;
        }

        //! TSDecl: @method transposeSelf(): Mat4x4
        transposeSelf() {
            TransposeMat4x4F32(this.__mem__, this.__mem__);
            return this;
        }

        //! TSDecl: @method toMat3x3(): Mat3x3
        toMat3x3() {
            const m = this.__mem__;
            return Mat3x3.ColMajor([
                m[0], m[1], m[3], m[4], m[5], m[7], m[12], m[13], m[15]
            ]);
        }

        //! TSDecl: @method map(x: f32, y: f32, z: f32, w: f32): Vec4
        map(x, y, z, w) {
            const m = this.__mem__;
            return new Vec4(
                m[0] * x + m[4] * y + m[8] * z + m[12] * w,
                m[1] * x + m[5] * y + m[9] * z + m[13] * w,
                m[2] * x + m[6] * y + m[10] * z + m[14] * w,
                m[3] * x + m[7] * y + m[11] * z + m[15] * w,
            );
        }

        //! TSDecl: @method mapVec3(v: Vec3): Vec3
        mapVec3(v) {
            return this.map(v.x, v.y, v.z, 0).degenerate(VecDegenerate.Discard);
        }

        //! TSDecl: @method mapVec4(v: Vec4): Vec4
        mapVec4(v) {
            return this.map(v.x, v.y, v.z, v.w);
        }

        //! TSDecl: @method isFinite(): boolean
        isFinite() {
            return ScalarsAreFinite(this.__mem__);
        }

        //! TSDecl: @method determinant(): f32
        determinant() {
            return Mat4x4DeterminantF32(this.__mem__);
        }
    }
    //! TSDecl: @end


    //! TSDecl: @class RuntimeEffectBuilder
    class RuntimeEffectBuilder {
        #effect;
        #uniformStore;
        #uniformView;
        #childrenStore;
        #uniformInfoMap;
        #childInfoMap;

        //! TSDecl: @constructor(effect: RuntimeEffect)
        constructor(effect) {
            this.#effect = effect;
            if (effect.uniformsByteSize === 0) {
                this.#uniformStore = null;
                this.#uniformView = null;
            } else {
                this.#uniformStore = new ArrayBuffer(effect.uniformsByteSize);
                this.#uniformView = new DataView(this.#uniformStore);
            }
            this.#childrenStore = new Array(effect.children.length);

            // Cache reflection information of the SkSL program
            this.#uniformInfoMap = new Map();
            for (const uniform of this.#effect.uniforms) {
                this.#uniformInfoMap.set(uniform.name, {
                    name: uniform.name,
                    offset: uniform.offset,
                    sizeInBytes: uniform.sizeInBytes,
                    type: uniform.type,
                    count: uniform.count,
                    flags: uniform.flags,
                    isArray: !!(uniform.flags & natives.SkSLUniformFlags.Array),
                    isColor: !!(uniform.flags & natives.SkSLUniformFlags.Color),
                    // To indicate how many times the uniform has been set. It is useful when
                    // we perform the uniform-absence check.
                    filled: 0
                });
            }

            this.#childInfoMap = new Map();
            for (const child of this.#effect.children) {
                this.#childInfoMap.set(child.name, {
                    name: child.name,
                    type: child.type,
                    index: child.index,

                    // To indicate how many times the child has been set. If is useful when
                    // we perform the child-absence check.
                    filled: 0
                });
            }
        }

        #makeArtifact(callback) {
            for (const uniform of this.#uniformInfoMap.values()) {
                if (uniform.filled === 0) {
                    throw Error(`cannot instantiate effect: missing uniform variable '${uniform.name}'`);
                }
            }
            for (const child of this.#childInfoMap.values()) {
                if (child.filled === 0) {
                    throw Error(`cannot instantiate effect: missing child '${child.name}'`);
                }
            }

            const effect = this.#effect;
            const uniforms = this.#uniformStore ? new Uint8Array(this.#uniformStore) : null;
            const children = this.#childrenStore;
            this.#effect = null;
            this.#uniformStore = null;
            this.#uniformView = null;
            this.#childrenStore = null;
            this.#uniformInfoMap = null;
            this.#childInfoMap = null;
            return callback(effect, uniforms, children);
        }

        #findUniformChecked(name, expectType, isArray, arraySize) {
            const uniform = this.#uniformInfoMap.get(name);
            if (uniform === undefined) {
                throw Error(`uniform variable '${name}' is not defined`);
            }
            if (uniform.type !== expectType ||  uniform.isArray !== isArray) {
                throw TypeError(`uniform variable '${name}': mismatched data type`);
            }
            if (isArray && uniform.count !== arraySize) {
                throw TypeError(`uniform variable '${name}': mismatched array length`);
            }
            return uniform;
        }

        #findChildChecked(name, expectType) {
            const child = this.#childInfoMap.get(name);
            if (child === undefined) {
                throw Error(`child '${name}' is not defined`);
            }
            if (child.type !== expectType) {
                throw TypeError(`child '${name}' does not match the expected type`);
            }
            return child;
        }

        //! TSDecl: @method setChild(name: string, v: SkSLChild): RuntimeEffectBuilder
        setChild(name, v) {
            let type;
            if (v instanceof natives.Shader) {
                type = natives.SkSLChildType.Shader;
            } else if (v instanceof natives.ColorFilter) {
                type = natives.SkSLChildType.ColorFilter;
            } else if (v instanceof natives.Blender) {
                type = natives.SkSLChildType.Blender;
            } else {
                throw TypeError(`unrecognized child type to set SkSL child ${name}`);
            }
            const child = this.#findChildChecked(name, type);
            this.#childrenStore[child.index] = v;
            child.filled++;
            return this;
        }

        #fillFloatToUniform(startOffset, array) {
            let offset = startOffset;
            for (const element of array) {
                this.#uniformView.setFloat32(offset, element, true);
                offset += 4;
            }
        }

        #fillIntToUniform(startOffset, array) {
            let offset = startOffset;
            for (const element of array) {
                this.#uniformView.setInt32(offset, element, true);
                offset += 4;
            }
        }

        //! TSDecl: @method setUniformFloat(name: string, v: f32): RuntimeEffectBuilder
        setUniformFloat(name, v) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Float, false);
            this.#uniformView.setFloat32(uniform.offset, v, true);
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformFloatN(name: string, v: @generic(ArrayLike, f32)): RuntimeEffectBuilder
        setUniformFloatN(name, v) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Float, true, v.length);
            this.#fillFloatToUniform(uniform.offset, v);
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformFloat2(name: string, x: f32, y: f32): RuntimeEffectBuilder
        setUniformFloat2(name, x, y) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Float2, false);
            this.#uniformView.setFloat32(uniform.offset, x, true);
            this.#uniformView.setFloat32(uniform.offset + 4, y, true);
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformFloat2v(name: string, v: Vec2): RuntimeEffectBuilder
        setUniformFloat2v(name, v) {
            return this.setUniformFloat2(name, v.x, v.y);
        }

        //! TSDecl: @method setUniformFloat2N(name: string, v: @generic(ArrayLike, f32)): RuntimeEffectBuilder
        setUniformFloat2N(name, v) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Float2, true, v.length / 2)
            this.#fillFloatToUniform(uniform.offset, v);
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformFloat2vN(name: string, v: @generic(ArrayLike, Vec2)): RuntimeEffectBuilder
        setUniformFloat2vN(name, v) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Float2, true, v.length);
            let offset = uniform.offset;
            for (const element of v) {
                this.#uniformView.setFloat32(offset, element.x, true);
                this.#uniformView.setFloat32(offset + 4, element.y, true);
                offset += 8;
            }
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformFloat3(name: string, x: f32, y: f32, z: f32): RuntimeEffectBuilder
        setUniformFloat3(name, x, y, z) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Float3, false);
            this.#uniformView.setFloat32(uniform.offset, x, true);
            this.#uniformView.setFloat32(uniform.offset + 4, y, true);
            this.#uniformView.setFloat32(uniform.offset + 8, z, true);
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformFloat3v(name: string, v: Vec3): RuntimeEffectBuilder
        setUniformFloat3v(name, v) {
            return this.setUniformFloat3(name, v.x, v.y, v.z);
        }

        //! TSDecl: @method setUniformFloat3N(name: string, v: @generic(ArrayLike, f32)): RuntimeEffectBuilder
        setUniformFloat3N(name, v) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Float3, true, v.length / 3)
            this.#fillFloatToUniform(uniform.offset, v);
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformFloat3vN(name: string, v: @generic(ArrayLike, Vec3)): RuntimeEffectBuilder
        setUniformFloat3vN(name, v) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Float3, true, v.length);
            let offset = uniform.offset;
            for (const element of v) {
                this.#uniformView.setFloat32(offset, element.x, true);
                this.#uniformView.setFloat32(offset + 4, element.y, true);
                this.#uniformView.setFloat32(offset + 8, element.z, true);
                offset += 12;
            }
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformFloat4(name: string, x: f32, y: f32, z: f32, w: f32): RuntimeEffectBuilder
        setUniformFloat4(name, x, y, z, w) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Float4, false);
            this.#uniformView.setFloat32(uniform.offset, x, true);
            this.#uniformView.setFloat32(uniform.offset + 4, y, true);
            this.#uniformView.setFloat32(uniform.offset + 8, z, true);
            this.#uniformView.setFloat32(uniform.offset + 12, w, true);
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformFloat4v(name: string, v: Vec4): RuntimeEffectBuilder
        setUniformFloat4v(name, v) {
            return this.setUniformFloat4(name, v.x, v.y, v.z, v.w);
        }

        //! TSDecl: @method setUniformFloat4N(name: string, v: @generic(ArrayLike, f32)): RuntimeEffectBuilder
        setUniformFloat4N(name, v) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Float4, true, v.length / 4);
            this.#fillFloatToUniform(uniform.offset, v);
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformFloat4vN(name: string, v: @generic(ArrayLike, Vec4)): RuntimeEffectBuilder
        setUniformFloat4vN(name, v) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Float4, true, v.length);
            let offset = uniform.offset;
            for (const element of v) {
                this.#uniformView.setFloat32(offset, element.x);
                this.#uniformView.setFloat32(offset + 4, element.y);
                this.#uniformView.setFloat32(offset + 8, element.z);
                this.#uniformView.setFloat32(offset + 12, element.w);
                offset += 16;
            }
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformInt(name: string, v: i32): RuntimeEffectBuilder
        setUniformInt(name, v) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Int, false);
            this.#uniformView.setInt32(uniform.offset, v, true);
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformIntN(name: string, v: @generic(ArrayLike, i32)): RuntimeEffectBuilder
        setUniformIntN(name, v) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Int, true, v.length);
            this.#fillIntToUniform(uniform.offset, v);
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformInt2(name: string, x: i32, y: i32): RuntimeEffectBuilder
        setUniformInt2(name, x, y) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Int2, false);
            this.#uniformView.setInt32(uniform.offset, x, true);
            this.#uniformView.setInt32(uniform.offset + 4, y, true);
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformInt2N(name: string, v: @generic(ArrayLike, i32)): RuntimeEffectBuilder
        setUniformInt2N(name, v) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Int2, true, v.length / 2)
            this.#fillIntToUniform(uniform.offset, v);
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformInt3(name: string, x: i32, y: i32, z: i32): RuntimeEffectBuilder
        setUniformInt3(name, x, y, z) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Int3, false);
            this.#uniformView.setInt32(uniform.offset, x, true);
            this.#uniformView.setInt32(uniform.offset + 4, y, true);
            this.#uniformView.setInt32(uniform.offset + 8, z, true);
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformInt3N(name: string, v: @generic(ArrayLike, i32)): RuntimeEffectBuilder
        setUniformInt3N(name, v) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Int3, true, v.length / 3)
            this.#fillIntToUniform(uniform.offset, v);
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformInt4(name: string, x: i32, y: i32, z: i32, w: i32): RuntimeEffectBuilder
        setUniformInt4(name, x, y, z, w) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Int4, false);
            this.#uniformView.setInt32(uniform.offset, x, true);
            this.#uniformView.setInt32(uniform.offset + 4, y, true);
            this.#uniformView.setInt32(uniform.offset + 8, z, true);
            this.#uniformView.setInt32(uniform.offset + 12, w, true);
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformInt4N(name: string, v: @generic(ArrayLike, i32)): RuntimeEffectBuilder
        setUniformInt4N(name, v) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Int4, true, v.length / 4);
            this.#fillIntToUniform(uniform.offset, v);
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformFloat2x2(name: string, r1c1: f32, r1c2: f32, r2c1: f32, r2c2: f32): RuntimeEffectBuilder
        setUniformFloat2x2(name, r1c1, r1c2, r2c1, r2c2) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Float2x2, false);
            // Matrix is stored in column-major order
            this.#uniformView.setFloat32(uniform.offset, r1c1, true);
            this.#uniformView.setFloat32(uniform.offset + 4, r2c1, true);
            this.#uniformView.setFloat32(uniform.offset + 8, r1c2, true);
            this.#uniformView.setFloat32(uniform.offset + 12, r2c2, true);
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformFloat3x3(name: string, mat: Mat3x3): RuntimeEffectBuilder
        setUniformFloat3x3(name, mat) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Float3x3, false);
            this.#fillFloatToUniform(uniform.offset, mat.__mem__);
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformFloat4x4(name: string, mat: Mat4x4): RuntimeEffectBuilder
        setUniformFloat4x4(name, mat) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Float4x4, false);
            this.#fillFloatToUniform(uniform.offset, mat.__mem__);
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformFloat2x2N(name: string, v: @generic(ArrayLike, f32)): RuntimeEffectBuilder
        setUniformFloat2x2N(name, v) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Float2x2, true, v.length / 4);
            this.#fillFloatToUniform(uniform.offset, v);
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformFloat3x3N(name: string, mat: @array(Mat3x3)): RuntimeEffectBuilder
        setUniformFloat3x3N(name, mat) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Float3x3, true, mat.length);
            let offset = uniform.offset;
            for (const element of mat) {
                this.#fillFloatToUniform(offset, element.__mem__);
                offset += 4 * 3 * 3;
            }
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method setUniformFloat4x4N(name: string, mat: @array(Mat4x4)): RuntimeEffectBuilder
        setUniformFloat4x4N(name, mat) {
            const uniform = this.#findUniformChecked(name, natives.SkSLUniformType.Float4x4, true, mat.length);
            let offset = uniform.offset;
            for (const element of mat) {
                this.#fillFloatToUniform(offset, element.__mem__);
                offset += 4 * 4 * 4;
            }
            uniform.filled++;
            return this;
        }

        //! TSDecl: @method makeShader(localMatrix: @union(null, Mat3x3)): Shader
        makeShader(localMatrix) {
            return this.#makeArtifact((effect, uniforms, children) => {
                return effect.makeShader(uniforms, children, localMatrix);
            });
        }

        //! TSDecl: @method makeColorFilter(): ColorFilter
        makeColorFilter() {
            return this.#makeArtifact((effect, uniforms, children) => {
                return effect.makeColorFilter(uniforms, children);
            });
        }

        //! TSDecl: @method makeBlender(): Blender
        makeBlender() {
            return this.#makeArtifact((effect, uniforms, children) => {
                return effect.makeBlender(uniforms, children);
            });
        }
    }
    //! TSDecl: @end

    /*
     * RSXform is a compressed matrix format that only represents rotation+scale+translation:
     * [ SCos     -SSin     Tx ]
     * [ SSin      SCos     Ty ]
     * [    0         0      1 ]
     *
     * When a bunch of matrices that do not contain perspective need to be provided for canvas API,
     * `RSXformArray` is more efficient than using an array of Mat3x3.
     */

    //! TSDecl: @class RSXformArray
    class RSXformArray {
        // Memory layout: [f32 SCos1, f32 SSin1, f32 Tx1, f32 Ty1, ...     ]
        //                 ^-------------------------------------  ^-------
        //                          RSXform1                         other RSXforms
        __mem__

        //! TSDecl: @property @readonly count: i32
        get count() {
            return this.__mem__.length >> 2;
        }

        //! TSDecl: @constructor(count: i32)
        constructor(count) {
            if (count <= 0) {
                throw RangeError('invalid size of RSXform array (must be greater than zero)');
            }
            this.__mem__ = new Float32Array(count * 4);
            // Set everyone to identity
            for (let i = 0; i < count; i++) {
                this.__mem__[i * 4] = 1;
            }
        }

        #checkIndexRange(index) {
            if (index < 0 || index >= this.count) {
                throw RangeError('index is out of range');
            }
        }

        //! TSDecl: @method set(index: i32, scos: f32, ssin: f32, tx: f32, ty: f32): void
        set(index, scos, ssin, tx, ty) {
            this.#checkIndexRange(index);
            const m = this.__mem__;
            m[index * 4] = scos;
            m[index * 4 + 1] = ssin;
            m[index * 4 + 2] = tx;
            m[index * 4 + 3] = ty;
        }

        //! TSDecl: @method setFromRadians(index: f32, scale: f32, rad: f32, tx: f32, ty: f32,
        //! TSDecl:                        centerX: f32, centerY: f32): void
        setFromRadians(index, scale, rad, tx, ty, centerX, centerY) {
            this.#checkIndexRange(index);
            const m = this.__mem__;
            const s = Math.sin(rad) * scale, c = Math.cos(rad) * scale;
            m[index * 4] = c;
            m[index * 4 + 1] = s;
            m[index * 4 + 2] = tx + (-c * centerX) + s * centerY;
            m[index * 4 + 3] = ty + (-s * centerX) - c * centerY;
        }

        //! TSDecl: @method setIdentity(index: f32): void
        setIdentity(index) {
            this.set(index, 1, 0, 0, 0);
        }
    }
    //! TSDecl: @end

    return {
        Vec2: Vec2,
        VecDegenerate: VecDegenerate,
        Vec3: Vec3,
        Vec4: Vec4,
        Rect: Rect,
        Mat3x3: Mat3x3,
        Mat4x4: Mat4x4,
        RuntimeEffectBuilder: RuntimeEffectBuilder,
        RSXformArray: RSXformArray,

        __proto__: null
    };
});
