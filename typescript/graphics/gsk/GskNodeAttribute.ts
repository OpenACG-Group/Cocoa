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

interface IComparable {
    equalTo(other: IComparable): boolean;
}

type AttrValueUnionT = bigint | number | boolean | string
                     | Function | IComparable;

enum AttrValueType {
    kPrimitive,
    kFunction,
    kComparable
}

export interface GskIStatefulAttribute {
    hasChanged(): boolean;
    updateChangeState(): void;
}

export class GskNodeAttribute<T extends AttrValueUnionT> implements GskIStatefulAttribute {
    private readonly fName: string;
    private fValue: NonNullable<T>;
    private fChanged: boolean;
    private readonly fValueType: AttrValueType;

    constructor(name: string, value: NonNullable<T>) {
        this.fName = name;
        this.fValue = value;
        this.fChanged = false;

        const typename = typeof value;
        if (typename === 'bigint' || typename === 'number' ||
            typename === 'boolean' || typename === 'string') {
            this.fValueType = AttrValueType.kPrimitive;
        } else if (typename === 'function') {
            this.fValueType = AttrValueType.kFunction;
        } else {
            this.fValueType = AttrValueType.kComparable;
        }
    }

    public get name(): string {
        return this.fName;
    }

    public set(value: NonNullable<T>): void {
        // Note that we don't check the type of `value` as we
        // assume that the type is constrained by TypeScript.
        if (this.fValueType == AttrValueType.kComparable) {
            this.fChanged = !(this.fValue as IComparable).equalTo((value as IComparable));
        } else {
            this.fChanged = (this.fValue === value);
        }
        this.fValue = value;
    }

    public get(): NonNullable<T> {
        return this.fValue;
    }

    public hasChanged(): boolean {
        return this.fChanged;
    }

    public updateChangeState(): void {
        this.fChanged = false;
    }
}

export enum GskAttributeStateGroupOp {
    kAnd,
    kOr
}

export class GskNodeAttributeStateGroup implements GskIStatefulAttribute {
    private readonly fAttributes: Array<GskIStatefulAttribute>;
    private readonly fOp: GskAttributeStateGroupOp;

    constructor(attributes: Array<GskIStatefulAttribute>, op: GskAttributeStateGroupOp) {
        this.fAttributes = [...attributes];
        this.fOp = op;
    }

    public hasChanged(): boolean {
        if (this.fOp == GskAttributeStateGroupOp.kAnd) {
            for (const attr of this.fAttributes) {
                if (!attr.hasChanged()) {
                    return false;
                }
            }
            return true;
        } else if (this.fOp == GskAttributeStateGroupOp.kOr) {
            for (const attr of this.fAttributes) {
                if (attr.hasChanged()) {
                    return true;
                }
            }
            return false;
        }
        throw Error('Invalid enumeration value for GskAttributeStateGroupOp');
    }

    public updateChangeState() {
        this.fAttributes.forEach(attr => attr.updateChangeState());
    }
}
