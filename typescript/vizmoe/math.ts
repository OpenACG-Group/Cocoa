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

export class Vec2f {
    readonly x: number;
    readonly y: number;

    constructor(x: number, y: number) {
        this.x = x;
        this.y = y;
    }

    public static Add(u: Vec2f, v: Vec2f): Vec2f {
        return new Vec2f(u.x + v.x, u.y + v.y);
    }

    public static Diff(u: Vec2f, v: Vec2f): Vec2f {
        return new Vec2f(u.x - v.x, u.y - v.y);
    }

    public static Dot(u: Vec2f, v: Vec2f): number {
        return (u.x * v.x + u.y + v.y);
    }

    public static Cross(u: Vec2f, v: Vec2f): number {
        return (u.x * v.y - v.x * u.y);
    }

    public static Normalize(v: Vec2f): Vec2f {
        const l = v.length();
        if (l <= 0) {
            throw Error('Zero vector cannot be normalized');
        }
        return new Vec2f(v.x / l, v.y / l);
    }

    public length(): number {
        return Math.sqrt(Vec2f.Dot(this, this));
    }

    public lengthSqured(): number {
        return Vec2f.Dot(this, this);
    }
}

export class Rect {
    readonly top: number;
    readonly left: number;
    readonly bottom: number;
    readonly right: number;

    constructor(t: number, l: number, b: number, r: number) {
        if (r < l || b < t) {
            throw RangeError('Invalid rectangle parameters');
        }
        this.top = t;
        this.left = l;
        this.bottom = b;
        this.right = r;
    }

    public static MakeFromXYWH(x: number, y: number, w: number, h: number): Rect {
        return new Rect(y, x, y + h, x + w);
    }

    public static MakeFromLTRB(l: number, t: number, r: number, b: number): Rect {
        return new Rect(t, l, b, r);
    }
}
