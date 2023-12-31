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

import { Rect } from '../base/Rectangle';
import { LinkedList } from '../../core/linked_list';
import { Mat3x3 } from '../base/Matrix';

export enum GskConcreteType {
    kGroup = 'GskGroup',
    kGeometryRect = 'GskRect',
    kImage = 'GskImage',
    kDraw = 'GskDraw',
    kMaterialColor = 'GskMaterialColor',
    kMaterialShader = 'GskMaterialShader',

    kOpacityEffect = 'GskOpacityEffect',
    kImageFilterEffect = 'GskImageFilterEffect',
    kTransformEffect = 'GskTransformEffect',
    kImageFilter = 'GskImageFilter',

    kMatrix = 'GskMatrix',
    // Not exported, use `GskTransform.Concat()` to create it
    kConcat = 'GskConcat',
    // Not exported, use `GskTransform.Inverse()` to create it
    kInverse = 'GskInverse'
}

export class GskNodeError extends Error {
    readonly node: GskNode;

    constructor(node: GskNode, what: string) {
        super(what);
        this.node = node;
    }

    public static Throw(node: GskNode, what: string): void {
        throw new GskNodeError(node, what);
    }
}

let nodeIdCounter = 0;

enum NodeFlags {
    kInvalidated = 1 << 1,
    kDamage = 1 << 2,
    kInTraversal = 1 << 3
}

export enum NodeTrait {
    kNone = 0,
    kOverrideDamage = 1 << 1,
    kBubbleDamage = 1 << 2
}

export class GskInvalidationRecorder {
    public add(bounds: Rect, ctm: Mat3x3): void {

    }
}

interface IComparable<T> {
    equalTo(other: T): boolean;
}
type ComparableProperty = number | string | boolean | bigint | Function | object;
type PropertyComparator<T> = (a: T, b: T) => boolean;

function getPropertyComparator<T extends ComparableProperty>(value: T): PropertyComparator<T> {
    if (value == null) {
        return null;
    }
    if (typeof value != 'object' || !Reflect.has(value as object, 'equalTo')) {
        return (a: T, b: T): boolean => { return (a === b); };
    }

    return (a: T, b: T): boolean => {
        return (a as IComparable<T>).equalTo(b);
    };
}

/**
 * Class property decorator for observed property.
 * Property decorated by it is an "observed" property. When an observed property
 * changes, the node will be invalidated. Note that observed properties are managed
 * by the `GskNode` object, and any subclasses must not touch them except reading
 * values. They also cannot be deleted or redefined (unconfigurable).
 *
 * @param initialValue      An optional initial value for the decorated property.
 *                          If not provided, `null` is used.
 *
 * @param comparator        A function that will be called to test the equality of
 *                          the old value and new value. If not provided: use `equalTo`
 *                          method if the value is an object and has that method, otherwise
 *                          use "===" operator.
 */
export function GskProperty<Type extends ComparableProperty, Target extends GskNode>(
    initialValue: Type = null,
    comparator: PropertyComparator<Type> = null
) {
    return function(originalMethod: any, context: ClassFieldDecoratorContext<Target, Type>): void {
        const propertyName = String(context.name);

        if (context.private || context.static) {
            throw TypeError(`@GskProperty: decorator cannot decorate a ` +
                            `private or static property "${propertyName}"`);
        }

        // Never use an arrow function here because the initializer
        // has a `this` bound to the object instance.
        context.addInitializer(function() {
            const self: Target = this;

            // The real value of the property is stored at `__gsk_store_name`
            const storeName = `__gsk_store_${propertyName}`;
            Reflect.defineProperty(this, storeName, {
                value: initialValue == null ? null : initialValue,
                enumerable: false,
                configurable: true,
                writable: true
            });

            // Possibly we can get the comparator if an initial value is provided.
            // If not, the comparator will be got when a valid value is set.
            if (initialValue != null && comparator == null) {
                comparator = getPropertyComparator(initialValue);
            }

            // Redefine the property. Replace it with our own setters and getters
            // so that we can detect its change.
            const success = Reflect.defineProperty(this, propertyName, {
                set(updateValue: Type) {
                    if (updateValue == null) {
                        return;
                    }
                    if (comparator == null) {
                        comparator = getPropertyComparator(updateValue);
                    }
                    if (!comparator(self[storeName] as Type, updateValue)) {
                        self[storeName] = updateValue;
                        self.invalidate();
                    }
                },

                get(): Type {
                    return self[storeName] as Type;
                },

                enumerable: false,
                configurable: false
            });

            if (!success) {
                throw TypeError(`@GskProperty failed to redefine property "${propertyName}"`);
            }
        });
    }
}

/**
 * Base class of nodes in the scene graph.
 * Each node in the scene graph mainly implements three things:
 * rendering, tracking damage regions (invalidated regions), hit-test.
 */
export abstract class GskNode {
    private readonly fId: number;
    private readonly fType: GskConcreteType;
    private readonly fTrait: number;
    private readonly fObservedInv: LinkedList<GskNode>;
    private fFlags: number;
    private fBounds: Rect;

    protected constructor(type: GskConcreteType, trait: number) {
        this.fId = ++nodeIdCounter;
        this.fType = type;
        this.fFlags = NodeFlags.kInvalidated;
        this.fTrait = trait;
        this.fBounds = Rect.MakeEmpty();
        this.fObservedInv = new LinkedList<GskNode>();
    }

    public get id(): number {
        return this.fId;
    }

    public get type(): GskConcreteType {
        return this.fType;
    }

    public get bounds(): Rect {
        return this.fBounds;
    }

    public hasInvalid(): boolean {
        return !!(this.fFlags & NodeFlags.kInvalidated);
    }

    public invalidate(damage: boolean = true): void {
        // To detect circular traversal
        if (this.fFlags & NodeFlags.kInTraversal) {
            return;
        }
        this.fFlags |= NodeFlags.kInTraversal;

        if (this.hasInvalid() && (!damage || (this.fFlags & NodeFlags.kDamage))) {
            // All done.
            this.fFlags &= ~NodeFlags.kInTraversal;
            return;
        }

        if (damage && !(this.fTrait & NodeTrait.kBubbleDamage)) {
            // Found a damage observer
            this.fFlags |= NodeFlags.kDamage;
            damage = false;
        }

        this.fFlags |= NodeFlags.kInvalidated;
        this.fObservedInv.forEach((node) => {
            node.invalidate(damage);
            return true;
        });
        this.fFlags &= ~NodeFlags.kInTraversal;
    }

    public revalidate(recorder: GskInvalidationRecorder, ctm: Mat3x3): Rect {
        // To detect circular traversal
        if (this.fFlags & NodeFlags.kInTraversal) {
            return this.fBounds;
        }
        this.fFlags |= NodeFlags.kInTraversal;

        if (!this.hasInvalid()) {
            return this.fBounds;
        }

        const flags = this.fFlags;
        const generateDamage = (recorder != null)
                && ((flags & NodeFlags.kDamage) || (this.fTrait & NodeTrait.kOverrideDamage));
        if (generateDamage) {
            // Trivial transitive revalidation.
            this.fBounds = this.onRevalidate(recorder, ctm);
        } else {
            // Revalidate and emit damage for old-bounds, new-bounds
            const prevBounds = this.fBounds.clone();
            if (this.fTrait & NodeTrait.kOverrideDamage) {
                // Clear the recorder to allow descendants to override the
                // damage area.
                this.fBounds = this.onRevalidate(null, ctm);
            } else {
                this.fBounds = this.onRevalidate(recorder, ctm);
            }

            recorder.add(prevBounds, ctm);
            if (!prevBounds.equalTo(this.fBounds)) {
                recorder.add(this.fBounds, ctm);
            }
        }

        this.fFlags &= ~(NodeFlags.kInTraversal | NodeFlags.kInvalidated | NodeFlags.kDamage);

        return this.fBounds;
    }

    protected abstract onRevalidate(recorder: GskInvalidationRecorder, ctm: Mat3x3): Rect;

    protected observeChild(node: GskNode): void {
        this.fObservedInv.push(node);
    }

    protected unobserveChild(node: GskNode): void {
        this.fObservedInv.removeIf(value => node === value);
    }

    protected ASSERT_REVALIDATED(): void {
        if (this.hasInvalid()) {
            GskNodeError.Throw(this, 'Node has not been revalidated');
        }
    }

    protected ASSERT_HAS_INVL(): void {
        if (!this.hasInvalid()) {
            GskNodeError.Throw(this, 'Node should be invalidated');
        }
    }
}
