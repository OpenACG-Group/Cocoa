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

import { CkCanvas } from 'glamor';
import { LinkedList } from '../../core/linked_list';
import { Rect } from './rectangle';
import { CompositeAttributes } from "./render-node-composite-attributes";
import { Recorder } from './render-node-linearization';

export enum RenderNodeType {
    kCompositeNode,
    kPaintNode,
    kStaticTextureNode,
    kVideoTextureNode
}

let nodeIdCounter = 0;

export abstract class RenderNode {
    private readonly fType: RenderNodeType;
    private readonly fUniqueNodeId: number;

    protected constructor(type: RenderNodeType) {
        this.fType = type;
        this.fUniqueNodeId = ++nodeIdCounter;
    }

    public get type(): RenderNodeType {
        return this.fType;
    }

    public get uniqueNodeId(): number {
        return this.fUniqueNodeId;
    }

    public abstract compose(recorder: Recorder): void;
}

export class CompositeRenderNode extends RenderNode {
    private fChildrenList: LinkedList<RenderNode>;
    private readonly fAttributes: CompositeAttributes;

    constructor() {
        super(RenderNodeType.kCompositeNode);
        this.fChildrenList = new LinkedList();
        this.fAttributes = new CompositeAttributes();
    }

    public get attributes(): CompositeAttributes {
        return this.fAttributes;
    }

    public appendChild(node: RenderNode): void {
        let hasNode = false;
        this.fChildrenList.forEach((child) => {
            if (node == child) {
                hasNode = true;
            }
        });
        if (hasNode) {
            return;
        }
        this.fChildrenList.push(node);
    }

    public removeChild(node: RenderNode): void {
        this.fChildrenList.removeIf(child => child == node);
    }

    public isEmpty(): boolean {
        return this.fChildrenList.isEmpty();
    }

    public clearChildren(): void {
        this.fChildrenList = new LinkedList();
    }

    public compose(recorder: Recorder): void {
        this.fAttributes.makeScope(recorder, () => {
            for (const childNode of this.fChildrenList) {
                childNode.compose(recorder);
            }
        });
    }
}

export class PaintRenderNode extends RenderNode {
    private fPaintCallback: (canvas: CkCanvas) => void;
    private fPaintBounds: Rect;
    private fNeedsRepaint: boolean;

    constructor() {
        super(RenderNodeType.kPaintNode);
        this.fPaintCallback = null;
        this.fPaintBounds = Rect.MakeEmpty();
        this.fNeedsRepaint = false;
    }

    public isDirty(): boolean {
        return this.fNeedsRepaint;
    }

    public getBounds(): Rect {
        return this.fPaintBounds;
    }

    public paint(canvas: CkCanvas): void {
        if (this.fPaintCallback != null) {
            this.fPaintCallback(canvas);
            this.fNeedsRepaint = false;
        }
    }

    public update(bounds: Rect, callback: (canvas: CkCanvas) => void): void {
        if (bounds == null || callback == null) {
            throw TypeError('Argument `bounds` and `callback` can never be null');
        }
        this.fPaintCallback = callback;
        this.fPaintBounds = bounds;
        this.fNeedsRepaint = true;
    }

    public compose(recorder: Recorder): void {
        recorder.drawPaintNode(this);
    }
}
