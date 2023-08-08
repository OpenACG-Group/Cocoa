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

import { Vector2f } from './vector';
import { Rect } from './rectangle';

type UnionToIntersection<U> = (U extends any ? (k: U) => void : never) extends ((k: infer I) => void)
                            ? I : never;
type IsUnion<T> = [T] extends [UnionToIntersection<T>] ? false : true;
type SingleKey<T> = IsUnion<keyof T> extends true ? never : {} extends T ? never : T;

type OnlyKeys = 'top' | 'left' | 'right' | 'bottom';

export class EdgeInsets {
    public readonly top: number;
    public readonly left: number;
    public readonly right: number;
    public readonly bottom: number;
    public readonly width: number;
    public readonly height: number;

    public readonly topLeft: Vector2f;
    public readonly topRight: Vector2f;
    public readonly bottomLeft: Vector2f;
    public readonly bottomRight: Vector2f;

    public static None(): EdgeInsets {
        return new EdgeInsets(0, 0, 0, 0);
    }

    public static Uniform(value: number): EdgeInsets {
        return new EdgeInsets(value, value, value, value);
    }

    public static Only<T extends Partial<Record<OnlyKeys, number>>>(args: SingleKey<T>): EdgeInsets {
        if (args.top != undefined) {
            return new EdgeInsets(args.top, 0, 0, 0);
        }
        if (args.left != undefined) {
            return new EdgeInsets(0, args.left, 0, 0);
        }
        if (args.right != undefined) {
            return new EdgeInsets(0, 0, args.right, 0);
        }
        if (args.bottom != undefined) {
            return new EdgeInsets(0, 0, 0, args.bottom);
        }
        throw TypeError('Invalid argument for `Only` factory function');
    }

    public static Symmetric(horizontal: number, vertical: number): EdgeInsets {
        return new EdgeInsets(horizontal, vertical, vertical, horizontal);
    }

    public static LTRB(left: number, top: number, right: number, bottom: number): EdgeInsets {
        return new EdgeInsets(top, left, right, bottom);
    }

    private constructor(top: number, left: number, right: number, bottom: number) {
        this.top = top;
        this.left = left;
        this.right = right;
        this.bottom = bottom;
        this.width = left + right;
        this.height = top + bottom;
        this.topLeft = new Vector2f(left, top);
        this.topRight = new Vector2f(-right, top);
        this.bottomLeft = new Vector2f(left, -bottom);
        this.bottomRight = new Vector2f(-right, -bottom);
    }

    public equalTo(other: EdgeInsets): boolean {
        return (this.left == other.left && this.top == other.top &&
                this.right == other.right && this.bottom == other.bottom);
    }

    public add(other: EdgeInsets): EdgeInsets {
        return new EdgeInsets(
            this.top + other.top,
            this.left + other.left,
            this.right + other.right,
            this.bottom + other.bottom
        );
    }

    public makeInset(outset: Rect): Rect {
        return Rect.MakeLTRB(
            outset.left + this.left,
            outset.top + this.top,
            outset.right - this.right,
            outset.bottom - this.bottom
        );
    }

    public makeOutset(inset: Rect): Rect {
        return Rect.MakeLTRB(
            inset.left - this.left,
            inset.top - this.top,
            inset.right + this.right,
            inset.bottom + this.bottom
        );
    }
}
