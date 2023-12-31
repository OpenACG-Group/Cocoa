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

export class Maybe<T> {
    private fVal: T | null;
    private fHas: boolean;

    public static Ok<T>(val: T): Maybe<T> {
        return new Maybe<T>(val, true);
    }

    public static None<T>(): Maybe<T> {
        return new Maybe<T>(null, false);
    }

    constructor(val: T | null, has: boolean) {
        this.fVal = val;
        this.fHas = has;
    }

    public has(): boolean {
        return this.fHas;
    }

    public from(val: T): T {
        if (this.fHas) {
            return this.fVal;
        }
        return val;
    }

    public value(): T {
        if (!this.fHas) {
            throw Error('Empty object');
        }
        return this.fVal;
    }

    public unwrap(): T {
        return this.value();
    }
}