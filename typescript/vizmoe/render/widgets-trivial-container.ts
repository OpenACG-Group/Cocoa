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
    LayoutConstraint,
    WidgetType,
    TrivialContainerBase,
    WidgetBase
} from './widget-base';

import {Rect} from './rectangle';
import {Vector2f} from './vector';

export interface ViewWidgetArgs {
    child: WidgetBase;
}

/**
 * Root widget of the whole widget tree.
 * `View` only accepts one child node, applying no constraint on it
 * (maximum size is the viewport size, and minimum size is zero).
 */
export class View extends TrivialContainerBase {
    public constructor(args: ViewWidgetArgs) {
        super(WidgetType.kView, [ args.child ]);
    }

    protected onLayout(constraint: LayoutConstraint): Rect {
        // `constraint` is computed by layout engine and should keep
        // the same to the size of viewport. View itself does not
        // apply any constraints to the child node.
        const child = this.fChildren.first().value;
        child.position = new Vector2f(0, 0);
        return child.node.layout(constraint);
    }

    protected onTrivialContainerDiffUpdateAttrs(other: TrivialContainerBase) {
    }
}

export enum CenterAnchor {
    kHorizontal,
    kVertical,
    kBidirectional
}

export interface CenterWidgetArgs {
    child: WidgetBase;
    anchor: CenterAnchor;
}

/**
 * `Center` keeps its child widget placed at the center of parent's available
 * canvas.
 * `Center` has various layout behaviours, depending on what
 * `CenterAnchor` is used:
 *
 * 1. For `kHorizontal` mode, Center takes the maximum allowed width, and
 *    keeps the same height to its child's;
 * 2. For `kVertical` mode, Center takes the maximum allowed height, and
 *    keeps the same width to its child's;
 * 3. For `kBidirectional` mode, Center takes the maximum width and height.
 */
export class Center extends TrivialContainerBase {
    private fAnchor: CenterAnchor;

    public constructor(args: CenterWidgetArgs) {
        super(WidgetType.kCenter, [ args.child ]);
        this.fAnchor = args.anchor;
    }

    protected onTrivialContainerDiffUpdateAttrs(other: Center): void {
        this.fAnchor = other.fAnchor;
    }

    protected onLayout(constraint: LayoutConstraint): Rect {
        let childConstraint: LayoutConstraint = null;
        if (this.fAnchor == CenterAnchor.kHorizontal) {
            // `Center` takes the maximum allowed width, so the minimum width
            // of child can have no constraint.
            childConstraint = new LayoutConstraint(
                0,
                constraint.minHeight,
                constraint.maxWidth,
                constraint.maxHeight
            );
        } else if (this.fAnchor == CenterAnchor.kVertical) {
            // `Center` takes the maximum allowed height, so the minimum height
            // of child can have no constraint.
            childConstraint = new LayoutConstraint(
                constraint.minWidth,
                0,
                constraint.maxWidth,
                constraint.maxHeight
            );
        } else if (this.fAnchor == CenterAnchor.kBidirectional) {
            // `Center` takes the maximum allowed width and height, so the minimum
            // width and height of child can have no constraint.
            childConstraint = new LayoutConstraint(
                0,
                0,
                constraint.maxWidth,
                constraint.maxHeight
            );
        } else {
            throw RangeError('Invalid enumeration value for `CenterAnchor`');
        }

        const child = this.fChildren.first();
        const childRect = child.value.node.layout(childConstraint);
        let childX = 0, childY = 0;
        let resultRect: Rect = null;

        if (this.fAnchor == CenterAnchor.kHorizontal) {
            // For horizontal center mode, Center takes the maximum allowed
            // width, and keeps the same height to its child's.
            childX = (constraint.maxWidth - childRect.width) / 2;
            resultRect = Rect.MakeWH(constraint.maxWidth, childRect.height);
        } else if (this.fAnchor == CenterAnchor.kVertical) {
            // For vertical center mode, Center takes the maximum allowed
            // height, and keeps the same width to its child's.
            childY = (constraint.maxHeight - childRect.height) / 2;
            resultRect = Rect.MakeWH(childRect.width, constraint.maxHeight);
        } else if (this.fAnchor == CenterAnchor.kBidirectional) {
            // For bidirectional mode, Center takes the maximum allowed
            // width and height.
            childX = (constraint.maxWidth - childRect.width) / 2;
            childY = (constraint.maxHeight - childRect.height) / 2;
            resultRect = Rect.MakeWH(constraint.maxWidth, constraint.maxHeight);
        } else {
            throw RangeError('Invalid enumeration value for `CenterAnchor`');
        }

        child.value.position = new Vector2f(childX, childY);
        return resultRect;
    }
}

export enum AlignMode {
    kLeft,
    kRight,
    kTop,
    kBottom
}

export interface AlignWidgetArgs {
    mode: AlignMode;
    child: WidgetBase;
}

/**
 * `Align` places its child at the specified alignment baseline.
 * `Align` has various layout behaviours, depending on what `AlignMode`
 * is used:
 *
 * 1. For `k{Left,Right}` modes, `Align` takes the maximum allowed width,
 *    and keeps the same height to its child's;
 * 2. For `k{Top,Bottom}` modes, `Align` takes the maximum allowed height,
 *    and keeps the same width to its child's.
 *
 * Use `Center` container to achieve center alignment.
 */
export class Align extends TrivialContainerBase {
    private fMode: AlignMode;

    constructor(args: AlignWidgetArgs) {
        super(WidgetType.kAlign, [ args.child ]);
        this.fMode = args.mode;
    }

    protected onTrivialContainerDiffUpdateAttrs(other: Align): void {
        this.fMode = other.fMode;
    }

    protected onLayout(constraint: LayoutConstraint): Rect {
        let childConstraint: LayoutConstraint;
        if (this.fMode == AlignMode.kRight || this.fMode == AlignMode.kLeft) {
            // `Align` takes the maximum width, so the minimum width of child
            // can have no constraint.
            childConstraint = new LayoutConstraint(
                0,
                constraint.minHeight,
                constraint.maxWidth,
                constraint.maxHeight
            );
        } else {
            // `Align` takes the maximum height, so the minimum height of child
            // can have no constraint.
            childConstraint = new LayoutConstraint(
                constraint.minWidth,
                0,
                constraint.maxWidth,
                constraint.maxHeight
            );
        }

        const child = this.fChildren.first().value;
        const childRect = child.node.layout(childConstraint);
        let x = 0, y = 0;
        let resultRect: Rect = null;

        if (this.fMode == AlignMode.kLeft) {
            resultRect = Rect.MakeWH(constraint.maxWidth, childRect.height);
        } else if (this.fMode == AlignMode.kRight) {
            x = constraint.maxWidth - childRect.width;
            resultRect = Rect.MakeWH(constraint.maxWidth, childRect.height);
        } else if (this.fMode == AlignMode.kTop) {
            resultRect = Rect.MakeWH(childRect.width, constraint.maxHeight);
        } else if (this.fMode == AlignMode.kBottom) {
            y = constraint.maxHeight - childRect.height;
            resultRect = Rect.MakeWH(childRect.width, constraint.maxHeight);
        }

        child.position = new Vector2f(x, y);
        return resultRect;
    }
}

enum ConstraintBoxMode {
    // Relative scaling. Use `{width,height}Scalar` to compute the
    // layout constraint of child node. The computed constraint is
    // a strict constraint (zero tolerance).
    kRelative,

    // Absolute constraint. Use `width` and `height` to give strict
    // (zero tolerance) layout constraint to child node.
    kAbsolute,

    // Overwrite the original constraint. This is similar to absolute
    // constraint, but it uses `{max,min}{Height,Width}` to overwrite
    // the original constraint instead of giving a strict constraint.
    kOverwrite
}

export interface ConstraintBoxWidgetArgs {
    child: WidgetBase;
    widthScalar?: number;
    heightScalar?: number;
    maxWidth?: number;
    minWidth?: number;
    maxHeight?: number;
    minHeight?: number;
    width?: number;
    height?: number;
}

/**
 * `ConstraintBox` changes the layout constraint of its child node.
 * The only principle is that the new constraint is either tighter
 * than or the same to the original constraint, which means it definitely
 * cannot be out of the original constraint. Otherwise, the new
 * constraint will NOT be applied.
 */
export class ConstraintBox extends TrivialContainerBase {
    private fMode: ConstraintBoxMode;
    private fWidthScalar: number;
    private fHeightScalar: number;
    private fWidth: number;
    private fHeight: number;
    private fMaxWidth: number;
    private fMinWidth: number;
    private fMaxHeight: number;
    private fMinHeight: number;

    constructor(args: ConstraintBoxWidgetArgs) {
        super(WidgetType.kConstraintBox, [ args.child ]);
        const Have = (...vs: any[]) => {
            for (let v of vs) {
                if (v == undefined) {
                    return false;
                }
                if (typeof v != 'number') {
                    return false;
                }
            }
            return true;
        };

        if (Have(args.width, args.height)) {
            this.fMode = ConstraintBoxMode.kAbsolute;
            this.fWidth = args.width;
            this.fHeight = args.height;
        } else if (Have(args.minWidth, args.maxWidth, args.minHeight, args.maxHeight)) {
            this.fMode = ConstraintBoxMode.kOverwrite;
            this.fMinWidth = args.minWidth;
            this.fMaxWidth = args.maxWidth;
            this.fMinHeight = args.minHeight;
            this.fMaxHeight = args.maxHeight;
            if (this.fMinWidth > this.fMaxWidth || this.fMinHeight > this.fMaxHeight) {
                throw RangeError('Illegal arguments for ConstraintBox: '
                                 +'minimum constraint is larger than maximum constraint');
            }
        } else if (Have(args.widthScalar, args.heightScalar)) {
            if (args.widthScalar < 0 || args.widthScalar > 1) {
                throw RangeError('Value of `widthScalar` is out of range [0, 1]');
            }
            if (args.heightScalar < 0 || args.heightScalar > 1) {
                throw RangeError('Value of `heightScalar` is out of range [0, 1]');
            }
            this.fMode = ConstraintBoxMode.kRelative;
            this.fWidthScalar = args.widthScalar;
            this.fHeightScalar = args.heightScalar;
        } else {
            throw Error('Illegal arguments for ConstraintBox');
        }
    }

    protected onTrivialContainerDiffUpdateAttrs(other: ConstraintBox): void {
        this.fMode = other.fMode;
        this.fWidthScalar = other.fWidthScalar;
        this.fHeightScalar = other.fHeightScalar;
        this.fWidth = other.fWidth;
        this.fHeight = other.fHeight;
        this.fMinWidth = other.fMinWidth;
        this.fMaxWidth = other.fMaxWidth;
        this.fMinHeight = other.fMinHeight;
        this.fMaxHeight = other.fMaxHeight;
    }

    protected onLayout(constraint: LayoutConstraint): Rect {
        let childConstraint: LayoutConstraint = null;
        if (this.fMode == ConstraintBoxMode.kOverwrite) {
            // The overwriting constraint must be tighter than the
            // original given constraint. Otherwise, the original
            // constraint is used.
            childConstraint = new LayoutConstraint(
                this.fMinWidth,
                this.fMinHeight,
                this.fMaxWidth,
                this.fMaxHeight
            );
            if (!childConstraint.isTighterThan(constraint)) {
                childConstraint = constraint;
            }
        } else if (this.fMode == ConstraintBoxMode.kAbsolute) {
            // The new specified constraint must be allowed by the original
            // constraint. Otherwise, the original constraint is used.
            if (constraint.allows(this.fWidth, this.fHeight)) {
                childConstraint = LayoutConstraint.MakeStrict(this.fWidth, this.fHeight);
            } else {
                childConstraint = constraint;
            }
        } else if (this.fMode == ConstraintBoxMode.kRelative) {
            const dW = constraint.maxWidth - constraint.minWidth;
            const dH = constraint.maxHeight - constraint.minHeight;
            const w = constraint.minWidth + dW * this.fWidthScalar;
            const h = constraint.minHeight + dH * this.fHeightScalar;
            childConstraint = new LayoutConstraint(w, h, w, h);
        }

        const child = this.fChildren.first().value;
        child.position = new Vector2f(0, 0);
        return child.node.layout(childConstraint);
    }
}

/**
 * When used for `VBoxLayout`, it controls the width policy of `VBoxLayout`;
 * when used for `HBoxLayout`, it controls the height policy of `HBoxLayout`.
 */
export enum VHBoxExpansionPolicy {
    kExpandToMax,
    kTightBound
}

export enum VBoxLayoutOrder {
    kBottomToTop,
    kTopToBottom,

    kBTT = kBottomToTop,
    kTTB = kTopToBottom
}

export interface VBoxLayoutWidgetArgs {
    children: Iterable<WidgetBase>;
    policy?: VHBoxExpansionPolicy;
    order?: VBoxLayoutOrder;
}

export class VBoxLayout extends TrivialContainerBase {
    private fWidthPolicy: VHBoxExpansionPolicy;
    private fOrder: VBoxLayoutOrder;

    constructor(args: VBoxLayoutWidgetArgs) {
        super(WidgetType.kVBoxLayout, args.children);
        if (args.policy != undefined) {
            this.fWidthPolicy = args.policy;
        } else {
            // Compute the tight bound as its width by default.
            // `Align`, `Center` and `ConstraintBox` can be used to implement
            // a more flexible width computation.
            this.fWidthPolicy = VHBoxExpansionPolicy.kTightBound;
        }

        if (args.order != undefined) {
            this.fOrder = args.order;
        } else {
            this.fOrder = VBoxLayoutOrder.kTopToBottom;
        }
    }

    protected onTrivialContainerDiffUpdateAttrs(other: VBoxLayout): void {
        this.fWidthPolicy = other.fWidthPolicy;
        this.fOrder = other.fOrder;
    }

    protected onLayout(constraint: LayoutConstraint): Rect {
        let coveredHeight = 0;
        let childMaxWidth = constraint.minWidth;
        for (let child of this.fChildren) {
            // If all the previous widgets cannot statisfy the minimum height
            // constraint, it will be the last widget's responsibility to statisfy
            // this constraint.
            let currentMinHeight = 0;
            if (this.fChildren.back().value == child && coveredHeight < constraint.minHeight) {
                currentMinHeight = constraint.minHeight - coveredHeight;
            }

            const rect = child.node.layout(new LayoutConstraint(
                constraint.minWidth,
                currentMinHeight,
                constraint.maxWidth,
                constraint.maxHeight - coveredHeight
            ));

            if (rect.width > childMaxWidth) {
                childMaxWidth = rect.width;
            }

            // The layout order does not affect the constraints of
            // each children node, so we can assume the layout order
            // is TTB first, and adjust the coordinate later if the
            // actual layout order is BTT.
            child.position = new Vector2f(0, coveredHeight);
            coveredHeight += rect.height;
        }

        // Adjust the coordinate if the actual layout is BTT
        if (this.fOrder == VBoxLayoutOrder.kBottomToTop) {
            for (let child of this.fChildren) {
                const oldPos = child.position;
                child.position = new Vector2f(
                    0, coveredHeight - oldPos.y - child.node.lastLayoutBounds.height);
            }
        }

        let width: number;
        if (this.fWidthPolicy == VHBoxExpansionPolicy.kExpandToMax) {
            width = constraint.maxWidth;
        } else if (this.fWidthPolicy == VHBoxExpansionPolicy.kTightBound) {
            width = childMaxWidth;
        }

        return Rect.MakeWH(width, coveredHeight);
    }
}

export enum HBoxLayoutOrder {
    kLeftToRight,
    kRightToLeft,

    kLTR = kLeftToRight,
    kRTL = kRightToLeft
}

export interface HBoxLayoutWidgetArgs {
    children: Iterable<WidgetBase>;
    policy?: VHBoxExpansionPolicy;
    order?: HBoxLayoutOrder;
}

export class HBoxLayout extends TrivialContainerBase {
    private fHeightPolicy: VHBoxExpansionPolicy;
    private fOrder: HBoxLayoutOrder;

    constructor(args: HBoxLayoutWidgetArgs) {
        super(WidgetType.kHBoxLayout, args.children);
        if (args.policy != undefined) {
            this.fHeightPolicy = args.policy;
        } else {
            this.fHeightPolicy = VHBoxExpansionPolicy.kTightBound;
        }
        if (args.order != undefined) {
            this.fOrder = args.order;
        } else {
            this.fOrder = HBoxLayoutOrder.kLeftToRight;
        }
    }

    protected onTrivialContainerDiffUpdateAttrs(other: HBoxLayout): void {
        this.fHeightPolicy = other.fHeightPolicy;
        this.fOrder = other.fOrder;
    }

    protected onLayout(constraint: LayoutConstraint): Rect {
        let coveredWidth = 0;
        let childMaxHeight = constraint.minHeight;
        for (let child of this.fChildren) {
            // If all the previous widgets cannot statisfy the minimum width
            // constraint, it will be the last widget's responsibility to statisfy
            // this constraint.
            let currentMinWidth = 0;
            if (this.fChildren.back().value == child && coveredWidth < constraint.minWidth) {
                currentMinWidth = constraint.minWidth - coveredWidth;
            }

            const rect = child.node.layout(new LayoutConstraint(
                currentMinWidth,
                constraint.minHeight,
                constraint.maxWidth - coveredWidth,
                constraint.maxHeight
            ));

            if (rect.height > childMaxHeight) {
                childMaxHeight = rect.height;
            }

            // The layout order does not affect the constraints of
            // each children node, so we can assume the layout order
            // is LTR first, and adjust the coordinate later if the
            // actual layout order is RTL.
            child.position = new Vector2f(coveredWidth, 0);
            coveredWidth += rect.width;
        }

        // Adjust the coordinate if the actual layout is RTL
        if (this.fOrder == HBoxLayoutOrder.kRightToLeft) {
            for (let child of this.fChildren) {
                const oldPos = child.position;
                child.position = new Vector2f(
                    coveredWidth - oldPos.x - child.node.lastLayoutBounds.width, 0);
            }
        }

        let height: number;
        if (this.fHeightPolicy == VHBoxExpansionPolicy.kExpandToMax) {
            height = constraint.maxHeight;
        } else if (this.fHeightPolicy == VHBoxExpansionPolicy.kTightBound) {
            height = childMaxHeight;
        }

        return Rect.MakeWH(coveredWidth, height);
    }
}
