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

import { Rect } from './rectangle';
import { Vector2f } from './vector';
import { EdgeInsets } from './edge-insets';
import { LinkedList } from '../../core/linked_list';
import { CompositeRenderNode, RenderNode } from './render-node';
import { Decoration } from './widget-decoration';

export enum WidgetType {
    // [Trivial Containers: event-propagate]
    //
    // Trivial containers are containers whose self do not render
    // anything on the canvas, but only help place child nodes or
    // give size constraints to child nodes.
    kView,
    kCenter,
    kAlign,
    kConstraintBox,
    kVBoxLayout,
    kHBoxLayout,

    // [BoxModel Containers: drawable,event-handle,event-propagate]
    //
    // BoxModel Containers implement the "Box Layout Model", which is
    // similar to the box model of CSS, to give user a more flexible
    // layout style. These containers can draw contents on the canvas
    // (like background color/image, border, etc.).
    kDecoratedBox,

    // Helpers
    kSpace,

    // [Leaf Widgets: drawable,event-handle]
    kSolid
}

export class LayoutConstraint {
    public readonly minWidth: number;
    public readonly minHeight: number;
    public readonly maxWidth: number;
    public readonly maxHeight: number;

    public static MakeStrict(width: number, height: number): LayoutConstraint {
        return new LayoutConstraint(width, height, width, height);
    }

    public static MakeTolerance(width: number, height: number, tolerance: Vector2f): LayoutConstraint {
        return new LayoutConstraint(width - tolerance.x,
                                    height - tolerance.y,
                                    width + tolerance.x,
                                    height + tolerance.y);
    }

    constructor(minWidth: number, minHeight: number, maxWidth: number, maxHeight: number) {
        this.minWidth = minWidth;
        this.minHeight = minHeight;
        this.maxWidth = maxWidth;
        this.maxHeight = maxHeight;
    }

    public clone(): LayoutConstraint {
        return new LayoutConstraint(
            this.minWidth,
            this.minHeight,
            this.maxWidth,
            this.maxHeight
        );
    }

    public isStrict(): boolean {
        return (this.minWidth == this.maxWidth &&
                this.minHeight == this.maxHeight);
    }

    public isStrictWidth(): boolean {
        return (this.minWidth == this.maxWidth);
    }

    public isStrictHeight(): boolean {
        return (this.minHeight == this.maxHeight);
    }

    public tolerance(): Vector2f {
        return new Vector2f((this.maxWidth - this.minWidth) / 2,
                            (this.maxHeight - this.minHeight) / 2);
    }

    public centralSize(): Vector2f {
        return new Vector2f((this.minWidth + this.maxWidth) / 2,
                            (this.minHeight + this.maxHeight) / 2);
    }

    public isTighterThan(other: LayoutConstraint): boolean {
        return (this.minWidth > other.minWidth && this.minHeight > other.minHeight &&
                this.maxWidth < other.maxWidth && this.maxHeight < other.maxHeight);
    }

    public allows(width: number, height: number): boolean {
        return (width <= this.maxWidth && width >= this.minWidth &&
                height <= this.maxHeight && height >= this.minHeight);
    }
}

/**
 * Widget is immutable object. In each frame, user should construct
 * a new widget tree, which will be compared with the old tree to
 * compute diffs. Computed diffs will be applied to the old widget
 * tree to update it, and then the corresponding render tree will be
 * generated (after re-layout) and submitted to the native layer.
 */
export abstract class WidgetBase {
    readonly type: WidgetType;
    protected fLastLayoutBounds: Rect;

    protected constructor(type: WidgetType) {
        this.type = type;
        this.fLastLayoutBounds = Rect.MakeEmpty();
    }

    public get lastLayoutBounds(): Rect {
        return this.fLastLayoutBounds;
    }

    public diffUpdate(other: WidgetBase): void {
        this.onDiffUpdate(other);
    }

    public discard(): void {
        this.onDiscard();
    }

    /**
     * Layout principles:
     * 1. Constraint goes down (from parent to child), and layout result
     *    goes up (from child to parent).
     * 2. Size is decided by child, and position is decided by parent.
     * 3. Children do not need to care where they are placed. Instead,
     *    they only need to care their own size.
     * 4. So drawable widgets draw in their own coordinate system, which
     *    means that they should assume the origin of canvas is at the
     *    left-top corner of their layout rectangle, and the size of
     *    canvas is the same to their layout rectangle.
     */
    public layout(constraint: LayoutConstraint): Rect {
        this.fLastLayoutBounds = this.onLayout(constraint);
        return this.fLastLayoutBounds;
    }

    public render(offsetInParent: Vector2f): RenderNode {
        return this.onRender(offsetInParent);
    }

    protected abstract onDiffUpdate(other: WidgetBase): void;
    protected onDiscard(): void {}
    protected abstract onLayout(constraint: LayoutConstraint): Rect;
    protected abstract onRender(offsetInParent: Vector2f): RenderNode;
}

interface TrivialPositionedChild {
    node: WidgetBase;
    position: Vector2f | null;
}

export abstract class TrivialContainerBase extends WidgetBase {
    protected fChildren: LinkedList<TrivialPositionedChild>;

    protected constructor(type: WidgetType, children: Iterable<WidgetBase>) {
        super(type);

        this.fChildren = new LinkedList<TrivialPositionedChild>();
        for (const child of children) {
            this.fChildren.push({ node: child, position: null });
        }
    }

    protected onDiffUpdate(other: TrivialContainerBase) {
        const updateList = new LinkedList<TrivialPositionedChild>();
        for (let newChild of other.fChildren) {
            // Reuse the old widget node as much as possible
            let foundReplacement = false;
            this.fChildren.forEachNode((node) => {
                if (node.value.node.type != newChild.node.type) {
                    return true;
                }

                foundReplacement = true;
                updateList.push({ node: node.value.node, position: null });
                this.fChildren.removeNode(node);
                node.value.node.diffUpdate(newChild.node);
                return false;
            });

            if (!foundReplacement) {
                updateList.push(newChild);
            }
        }

        for (let discarded of this.fChildren) {
            discarded.node.discard();
        }

        this.fChildren = updateList;
        this.onTrivialContainerDiffUpdateAttrs(other);
    }

    protected onRender(offsetInParent: Vector2f): RenderNode {
        if (this.fChildren.isEmpty()) {
            return null;
        }
        if (this.fChildren.first() == this.fChildren.back()) {
            // If the container only has one child node, there is no
            // need to create a composite render node, which means we
            // can render the child node directly as if the container
            // node did not exist.
            const child = this.fChildren.first().value;
            return child.node.render(child.position.add(offsetInParent));
        }

        // If the container has multiple children nodes, a composite
        // node must be created to aggregate their contents.
        const composite = new CompositeRenderNode();
        composite.attributes.setOffset(offsetInParent);

        for (const child of this.fChildren) {
            composite.appendChild(child.node.render(child.position));
        }

        return composite;
    }

    protected abstract onTrivialContainerDiffUpdateAttrs(other: TrivialContainerBase): void;
}

export interface BoxModelWidgetArgs {
    padding?: EdgeInsets;
    border?: EdgeInsets;
    margin?: EdgeInsets;
    decoration?: Decoration;
}

export interface RenderPositioner {
    inParent: {
        offset: Vector2f;
        margin: Rect;
        border: Rect;
        padding: Rect;
        content: Rect;
    }

    self: {
        margin: Rect;
        border: Rect;
        padding: Rect;
        content: Rect;
    }
}

/**
 * Box Model:
 *   +-----------------------+ <----- Margin Box (Layout Box)
 *   |        Margin         |
 *   | +-------------------+<+------- Border Box (where `border` decoration is drawn)
 *   | |      Border       | |
 *   | | +---------------+<+-+------- Padding Box
 *   | | |    Padding    | | |
 *   | | | +-----------+<+-+-+------- Content Box
 *   | | | |  Content  | | | |
 *   | | | +-----------+ | | |
 *   | | +---------------+ | |
 *   | +-------------------+ |
 *   +-----------------------+
 *
 * Margin box is equivalent to the layout box of a trivial (non-box-model)
 * widget when we perform layout.
 */
export abstract class BoxModelWidgetBase extends WidgetBase {
    protected fPadding: EdgeInsets;
    protected fBorder: EdgeInsets;
    protected fMargin: EdgeInsets;
    protected fDecoration: Decoration;

    protected constructor(type: WidgetType, args: BoxModelWidgetArgs) {
        super(type);

        function FromMaybe<T>(v: T | undefined, def: T): T {
            return (v != undefined ? v : def);
        }

        this.fPadding = FromMaybe(args.padding, EdgeInsets.None());
        this.fBorder = FromMaybe(args.border, EdgeInsets.None());
        this.fMargin = FromMaybe(args.margin, EdgeInsets.None());
        this.fDecoration = FromMaybe(args.decoration, Decoration.None());
    }

    public onDiffUpdate(other: BoxModelWidgetBase): void {
        if (other.type != this.type) {
            throw TypeError('Diff nodes that have different types');
        }
        this.fPadding = other.fPadding;
        this.fBorder = other.fBorder;
        this.fMargin = other.fMargin;
        this.fDecoration.diffUpdate(other.fDecoration);
        this.onBoxModelWidgetDiffUpdate(other);
    }

    public onLayout(constraint: LayoutConstraint): Rect {
        // `constraint` is a constraint for the layout bounds
        // (content+padding+border+margin), so we should compute
        // content constraint first before compute content bounds.

        // Copy constraint datas to a mutable object
        const contentConstraint = {
            minWidth: constraint.minWidth,
            minHeight: constraint.minHeight,
            maxWidth: constraint.maxWidth,
            maxHeight: constraint.maxHeight
        };

        const outerContent = this.fPadding.add(this.fBorder.add(this.fMargin));

        if (contentConstraint.maxWidth < outerContent.width ||
            contentConstraint.maxHeight < outerContent.height)
        {
            throw Error('Layout Error: constraint is too tight to hold box');
        }

        contentConstraint.maxWidth -= outerContent.width;
        if (constraint.isStrictWidth()) {
            contentConstraint.minWidth -= outerContent.width;
        } else if (contentConstraint.maxWidth < constraint.minWidth) {
            throw Error('Layout Error: constraint is too tight to hold box');
        }

        contentConstraint.maxHeight -= outerContent.height;
        if (constraint.isStrictHeight()) {
            contentConstraint.minHeight -= outerContent.height;
        } else if (contentConstraint.maxHeight < constraint.minHeight) {
            throw Error('Layout Error: constraint is too tight to hold box');
        }

        // Now we can compute content bounds
        const contentBounds = this.onLayoutInContentBox(new LayoutConstraint(
            contentConstraint.minWidth,
            contentConstraint.minHeight,
            contentConstraint.maxWidth,
            contentConstraint.maxHeight
        ));

        return outerContent.makeOutset(contentBounds).makeWH();
    }

    public onRender(offsetInParent: Vector2f): RenderNode {
        // Compute positioner
        const selfMarginBox = this.fLastLayoutBounds;
        const selfBorderBox = this.fMargin.makeInset(selfMarginBox);
        const selfPaddingBox = this.fBorder.makeInset(selfBorderBox);
        const selfContentBox = this.fPadding.makeInset(selfPaddingBox);
        const positioner = {
            inParent: {
                offset: offsetInParent,
                margin: selfMarginBox.makeOffsetv(offsetInParent),
                border: selfBorderBox.makeOffsetv(offsetInParent),
                padding: selfPaddingBox.makeOffsetv(offsetInParent),
                content: selfContentBox.makeOffsetv(offsetInParent)
            },
            self: {
                margin: selfMarginBox,
                border: selfBorderBox,
                padding: selfPaddingBox,
                content: selfContentBox
            }
        };

        // `Decoration` optionally adds some `CompositeRenderNode`
        // to aggregate the decoration canvas and widget's content
        // canvas. Decorations are also drawn by `aggregateDecoration`.
        return this.fDecoration.aggregateDecoration(
            positioner,
            this.onBoxModelWidgetRender(positioner)
        );
    }

    protected abstract onLayoutInContentBox(constraint: LayoutConstraint): Rect;
    protected abstract onBoxModelWidgetDiffUpdate(other: BoxModelWidgetBase): void;
    protected abstract onBoxModelWidgetRender(positioner: RenderPositioner): RenderNode;
}

export interface BoxModelPositionedChild {
    node: WidgetBase;
    posInLayoutBox: Vector2f | null;
    posInContentBox: Vector2f | null;
}

export abstract class BoxModelContainerBase extends BoxModelWidgetBase {
    protected fChildren: LinkedList<BoxModelPositionedChild>;

    protected constructor(type: WidgetType, children: Iterable<WidgetBase>, args: BoxModelWidgetArgs) {
        super(type, args);
        this.fChildren = new LinkedList<BoxModelPositionedChild>();
        for (let child of children) {
            this.fChildren.push({ node: child, posInLayoutBox: null, posInContentBox: null });
        }
    }

    protected onBoxModelWidgetDiffUpdate(other: BoxModelContainerBase) {
        const updateList = new LinkedList<BoxModelPositionedChild>();
        for (let newChild of other.fChildren) {
            // Reuse the old widget node as much as possible
            let foundReplacement = false;
            this.fChildren.forEachNode((node) => {
                if (node.value.node.type != newChild.node.type) {
                    return true;
                }

                foundReplacement = true;
                updateList.push({ node: node.value.node, posInLayoutBox: null, posInContentBox: null });
                this.fChildren.removeNode(node);
                node.value.node.diffUpdate(newChild.node);
                return false;
            });

            if (!foundReplacement) {
                updateList.push(newChild);
            }
        }

        for (let discarded of this.fChildren) {
            discarded.node.discard();
        }

        this.fChildren = updateList;
        this.onBoxModelContainerDiffUpdateAttrs(other);
    }

    private renderEachChildrenTrivially(composite: CompositeRenderNode): void {
        for (const child of this.fChildren) {
            const node = child.node.render(child.posInLayoutBox);
            if (node == null) {
                continue;
            }
            composite.appendChild(node);
        }
    }

    /**
     * This is a "trivial" implementation of `onRender` method, subclasses
     * should implement their own `onRender` method if necessary.
     * "Trivial" means that except translate no any other transformations
     * will be applied to the children nodes.
     */
    protected onBoxModelWidgetRender(positioner: RenderPositioner): RenderNode {
        const composite = new CompositeRenderNode();
        composite.attributes.setOffset(positioner.inParent.offset);
        this.renderEachChildrenTrivially(composite);
        return composite;
    }

    protected setChildPositionInContentBox(child: BoxModelPositionedChild, posInContent: Vector2f): void {
        const outerContent = this.fPadding.add(this.fBorder.add(this.fMargin));
        child.posInLayoutBox = posInContent.add(outerContent.topLeft);
        child.posInContentBox = posInContent;
    }

    protected onBoxModelContainerDiffUpdateAttrs(other: BoxModelContainerBase): void {}
}
