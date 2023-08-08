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

import { Point2f, Vector2f } from './vector';
import * as Fmt from '../../core/formatter';
import { CkArrayXYWHRect, CkRRect } from 'glamor';

/**
 * A basic 2D rectangle whose edges are parallel to the X and Y axes.
 */
export class Rect implements Fmt.Formattable {
    private readonly top_: number;
    private readonly left_: number;
    private readonly right_: number;
    private readonly bottom_: number;

    public static MakeXYWH(x: number, y: number, w: number, h: number): Rect {
        return new Rect(x, y, x + w, y + h);
    }

    public static MakeLTRB(l: number, t: number, r: number, b: number): Rect {
        return new Rect(l, t, r, b);
    }

    public static MakeWH(w: number, h: number): Rect {
        return new Rect(0, 0, w, h);
    }

    public static MakeEmpty(): Rect {
        return new Rect(0, 0, 0, 0);
    }

    public static MakeFromGL(from: CkArrayXYWHRect): Rect {
        return Rect.MakeXYWH(from[0], from[1], from[2], from[3]);
    }

    public static Clone(from: Rect): Rect {
        return new Rect(from.left_, from.top_, from.right_, from.bottom_);
    }

    public static Union(a: Rect, b: Rect): Rect {
        if (a.isEmpty() || b.isEmpty()) {
            return Rect.MakeEmpty();
        }
        return new Rect(
            a.left_ < b.left_ ? a.left_ : b.left_,
            a.top_ < b.top_ ? a.top_ : b.top_,
            a.right > b.right_ ? a.right_ : b.right_,
            a.bottom_ > b.bottom ? a.bottom_ : b.bottom_
        );
    }

    public static Intersect(a: Rect, b: Rect): Rect {
        if (a.isEmpty() || b.isEmpty()) {
            return Rect.MakeEmpty();
        }

        if (a.left_ > b.left_) {
            const t = a;
            a = b;
            b = t;
        }
        const x = b.left_, w = a.right_ - b.left_;
        if (w <= 0) {
            return this.MakeEmpty();
        }

        if (a.top_ > b.top_) {
            const t = a;
            a = b;
            b = t;
        }
        const y = b.top_, h = a.bottom_ - b.top_;
        if (h <= 0) {
            return this.MakeEmpty();
        }

        return this.MakeXYWH(x, y, w, h);
    }

    private constructor(L: number, T: number, R: number, B: number) {
        this.left_ = L;
        this.top_ = T;
        this.right_ = R;
        this.bottom_ = B;
    }

    public [Fmt.kObjectFormatter](ctx: Fmt.FormatterContext): Array<Fmt.TextBlock> {
        return [
            Fmt.TB(Fmt.TextBlockLayoutHint.kPrefix, [
                Fmt.TAG('Vizmoe.Rect')
            ]),
            ...Fmt.formatObjectValue({
                L: this.left_,
                T: this.top_,
                R: this.right_,
                B: this.bottom_
            }, ctx)
        ];
    }

    public get top(): number {
        return this.top_;
    }

    public get left(): number {
        return this.left_;
    }

    public get right(): number {
        return this.right_;
    }

    public get bottom(): number {
        return this.bottom_;
    }

    public get x(): number {
        return this.left_;
    }

    public get y(): number {
        return this.top_;
    }

    public get width(): number {
        return (this.right_ - this.left_);
    }

    public get height(): number {
        return (this.bottom_ - this.top_);
    }

    public get center(): Point2f {
        return new Vector2f(this.left_ / 2 + this.right_ / 2,
                            this.top_ / 2 + this.bottom_ / 2);
    }

    public get quadUpperLeft(): Vector2f {
        return new Vector2f(this.left_, this.top_);
    }

    public get quadUpperRight(): Vector2f {
        return new Vector2f(this.right_, this.top_);
    }

    public get quadLowerLeft(): Vector2f {
        return new Vector2f(this.left_, this.bottom_);
    }

    public get quadLowerRight(): Vector2f {
        return new Vector2f(this.right_, this.bottom_);
    }

    public equalTo(other: Rect): boolean {
        if (this.isEmpty() || other.isEmpty()) {
            return false;
        }
        return (this.left_ == other.left_ &&
                this.top_ == other.top_ &&
                this.right_ == other.right_ &&
                this.bottom_ == other.bottom_);
    }

    public isEmpty(): boolean {
        return (this.width == 0 && this.height == 0);
    }

    public makeWH(): Rect {
        return Rect.MakeWH(this.width, this.height);
    }

    public clone(): Rect {
        return Rect.Clone(this);
    }

    public union(other: Rect): Rect {
        return Rect.Union(this, other);
    }

    public makeOffset(dx: number, dy: number): Rect {
        return new Rect(this.left_ + dx,
                        this.top_ + dy,
                        this.right_ + dx,
                        this.bottom_ + dy);
    }

    public makeOffsetv(offset: Vector2f): Rect {
        return this.makeOffset(offset.x, offset.y);
    }

    public makeInset(dx: number, dy: number): Rect {
        return new Rect(this.left_ + dx,
                        this.top_ + dy,
                        this.right_ - dx,
                        this.bottom_ - dy);
    }

    public makeOutset(dx: number, dy: number): Rect {
        return new Rect(this.left_ - dx,
                        this.top_ - dy,
                        this.right_ + dx,
                        this.bottom_ + dy);
    }

    public contains(x: number, y: number): boolean {
        return (x >= this.left_ && x < this.right_ && y >= this.top_ && y < this.bottom_);
    }

    public containsRect(other: Rect): boolean {
        return (!this.isEmpty() && !other.isEmpty() &&
                this.left_ <= other.left_ && this.top_ <= other.top_ &&
                this.right_ >= other.right_ && this.bottom_ >= other.bottom_);
    }

    public toGLType(): CkArrayXYWHRect {
        return [this.x, this.y, this.width, this.height];
    }

    public intersect(other: Rect): Rect {
        return Rect.Intersect(this, other);
    }
}

export enum RRectCorner {
    kUpperLeft,
    kUpperRight,
    kLowerRight,
    kLowerLeft
}

export enum RRectType {
    kEmpty,
    kRect,
    kTrivial
}

const kZeroVector2f = new Vector2f(0, 0);
const kZeroCorners = [kZeroVector2f, kZeroVector2f, kZeroVector2f, kZeroVector2f];

export class RRect {
    private readonly rect_: Rect;
    private readonly type_: RRectType;
    // Radii order is UL, UR, LR, LL
    private readonly radii_: Vector2f[];
    private readonly is_xy_uniform_: boolean;

    public static POS_EQUAL_PRECISION = 1e-5;

    public static MakeEmpty(): RRect {
        return new RRect(Rect.MakeEmpty(), null);
    }

    public static MakeRect(rect: Rect): RRect {
        return new RRect(rect, kZeroCorners);
    }

    public static Make(rect: Rect, radii: Vector2f[]): RRect {
        if (rect.isEmpty()) {
            return RRect.MakeEmpty();
        }

        if (radii.length != 4) {
            throw TypeError('Invalid length of \'radii\' array (must be 4)');
        }

        return new RRect(rect, radii);
    }

    public static MakeEllipse(rect: Rect): RRect {
        if (rect.isEmpty()) {
            return RRect.MakeEmpty();
        }
        const v = new Vector2f(rect.width / 2, rect.height / 2);
        return new RRect(rect, [v, v, v, v]);
    }

    private constructor(rect: Rect, radii: Vector2f[]) {
        if (rect.isEmpty()) {
            this.rect_ = Rect.MakeEmpty();
            this.radii_ = [];
            this.type_ = RRectType.kEmpty;
            return;
        }

        this.rect_ = rect.clone();
        this.type_ = RRectType.kRect;
        this.radii_ = new Array<Vector2f>(radii.length);
        this.is_xy_uniform_ = true;

        for (let i = 0; i < radii.length; i++) {
            this.radii_[i] = radii[i].clone();

            if (this.radii_[i].x < 0) {
                this.radii_[i].x = 0;
            }
            if (this.radii_[i].y < 0) {
                this.radii_[i].y = 0;
            }

            if (this.radii_[i].x != 0 || this.radii_[i].y != 0) {
                this.type_ = RRectType.kTrivial;
            }

            if (Math.abs(radii[i].x - radii[i].y) > RRect.POS_EQUAL_PRECISION) {
                this.is_xy_uniform_ = false;
            }
        }
    }

    public get bounds(): Rect {
        return this.rect_.clone();
    }

    public get type(): RRectType {
        return this.type_;
    }

    public isEmpty(): boolean {
        return this.rect_.isEmpty();
    }

    public getRadii(corner: RRectCorner): Vector2f {
        return this.radii_[corner].clone();
    }

    public makeInset(dx: number, dy: number): RRect {
        const radii = new Array<Vector2f>(4);
        for (let i = 0; i < 4; i++) {
            let x = 0, y = 0;
            if (this.radii_[i].x) {
                x = Math.max(0, this.radii_[i].x - dx);
            }
            if (this.radii_[i].y) {
                y = Math.max(0, this.radii_[i].y - dy);
            }
            radii[i] = new Vector2f(x, y);
        }
        return new RRect(this.rect_.makeInset(dx, dy), radii);
    }

    public makeOffset(dx: number, dy: number): RRect {
        return new RRect(this.rect_.makeOffset(dx, dy), this.radii_);
    }

    public equalTo(other: RRect): boolean {
        if (!this.rect_.equalTo(other.rect_)) {
            return false;
        }
        for (let i = 0; i < 4; i++) {
            if (!this.radii_[i].equalTo(other.radii_[i])) {
                return false;
            }
        }
        return true;
    }

    public toGLType(): CkRRect {
        if (this.isEmpty()) {
            return {
                rect: this.rect_.toGLType(),
                uniformRadii: true,
                borderRadii: [0]
            };
        }

        let store: Float32Array = null;
        if (this.is_xy_uniform_) {
            store = new Float32Array(4);
            for (let i = 0; i < 4; i++) {
                store[i] = this.radii_[i].x;
            }
        } else {
            store = new Float32Array(8);
            for (let i = 0; i < 4; i++) {
                store[i * 2] = this.radii_[i].x;
                store[i * 2 + 1] = this.radii_[i].y;
            }
        }
        return {
            rect: this.rect_.toGLType(),
            uniformRadii: this.is_xy_uniform_,
            borderRadii: store
        };
    }
}
