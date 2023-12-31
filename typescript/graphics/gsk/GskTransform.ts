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

import {
    GskConcreteType,
    GskNode,
    GskInvalidationRecorder,
    GskProperty,
    NodeTrait
} from './GskNode';

import { Mat3x3 } from '../base/Matrix';
import { Rect } from '../base/Rectangle';

export abstract class GskTransform extends GskNode {
    public static Concat(a: GskTransform, b: GskTransform): GskTransform {
        return new GskConcat(a, b);
    }

    public static Inverse(t: GskTransform): GskTransform {
        return new GskInverse(t);
    }

    protected constructor(type: GskConcreteType) {
        // Transform nodes don't generate damage on their own, but via ancestor
        // TransformEffects.
        super(type, NodeTrait.kBubbleDamage);
    }

    public asMat3x3(): Mat3x3 {
        return this.onTransformGetMatrix();
    }

    protected abstract onTransformGetMatrix(): Mat3x3;
}

export class GskMatrix extends GskTransform {
    @GskProperty<Mat3x3, GskMatrix>(Mat3x3.Identity())
    public matrix: Mat3x3;

    constructor() {
        super(GskConcreteType.kMatrix);
    }

    protected onRevalidate(): Rect {
        return Rect.MakeEmpty();
    }

    protected onTransformGetMatrix(): Mat3x3 {
        return this.matrix;
    }
}

class GskConcat extends GskTransform {
    private readonly fA: GskTransform;
    private readonly fB: GskTransform;
    private readonly fComposed: Mat3x3;

    constructor(a: GskTransform, b: GskTransform) {
        super(GskConcreteType.kConcat);
        this.fA = a;
        this.fB = b;
        this.fComposed = Mat3x3.Identity();
        this.observeChild(a);
        this.observeChild(b);
    }

    protected onRevalidate(recorder: GskInvalidationRecorder, ctm: Mat3x3): Rect {
        this.fA.revalidate(recorder, ctm);
        this.fB.revalidate(recorder, ctm);
        this.fComposed.setConcat(this.fA.asMat3x3(), this.fB.asMat3x3());
        return Rect.MakeEmpty();
    }

    protected onTransformGetMatrix(): Mat3x3 {
        this.ASSERT_REVALIDATED();
        return this.fComposed;
    }
}

class GskInverse extends GskTransform {
    private readonly fT: GskTransform;
    private fInv: Mat3x3;

    constructor(t: GskTransform) {
        super(GskConcreteType.kInverse);
        this.fT = t;
        this.fInv = Mat3x3.Identity();
        this.observeChild(t);
    }

    protected onRevalidate(recorder: GskInvalidationRecorder, ctm: Mat3x3): Rect {
        this.fT.revalidate(recorder, ctm);

        const inv = this.fT.asMat3x3().invert();
        if (!inv.has()) {
            this.fInv = Mat3x3.Identity();
            return Rect.MakeEmpty();
        }

        this.fInv = inv.unwrap();
        return Rect.MakeEmpty();
    }

    protected onTransformGetMatrix(): Mat3x3 {
        this.ASSERT_REVALIDATED();
        return this.fInv;
    }
}
