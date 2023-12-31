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

import { GskConcreteType, GskInvalidationRecorder } from './GskNode';
import { GskRenderNode, RenderContext, GskScopedRenderContext } from './GskRenderNode';
import { Rect } from '../base/Rectangle';
import { Point2f } from '../base/Vector';
import { GskDisplayList } from './GskDisplayList';
import { Maybe } from '../../core/error';
import { LinkedList } from '../../core/linked_list';
import { Mat3x3 } from '../base/Matrix';

export class GskGroup extends GskRenderNode {
    private fChildren: LinkedList<GskRenderNode>;
    private fRequiresIsolation: boolean;

    constructor(children?: Array<GskRenderNode>) {
        super(GskConcreteType.kGroup, 0);
        this.fChildren = new LinkedList<GskRenderNode>();
        this.fRequiresIsolation = true;
        if (children != null) {
            children.forEach(node => this.addChild(node));
        }
    }

    public clear(): void {
        for (const child of this.fChildren) {
            this.unobserveChild(child);
        }
        this.fChildren = new LinkedList<GskRenderNode>();
    }

    public addChild(node: GskRenderNode): void {
        if (this.fChildren.findFirst(node).has()) {
            // Duplicates are not allowed
            return;
        }
        this.observeChild(node);
        this.fChildren.push(node);

        this.invalidate();
    }

    public removeChild(node: GskRenderNode): void {
        const found = this.fChildren.findFirst(node);
        if (!found.has()) {
            return;
        }
        this.fChildren.removeNode(found.unwrap()[1]);
        this.unobserveChild(node);

        this.invalidate();
    }

    protected onRevalidate(recorder: GskInvalidationRecorder, ctm: Mat3x3): Rect {
        this.ASSERT_HAS_INVL();

        let bounds = Rect.MakeEmpty();
        this.fRequiresIsolation = false;

        for (const child of this.fChildren) {
            const childBounds = child.revalidate(recorder, ctm);
            // If any of the child nodes overlap, group effects require layer isolation
            if (!this.fRequiresIsolation && !childBounds.intersect(bounds).isEmpty()) {
                this.fRequiresIsolation = true;
            }
            bounds = Rect.Union(bounds, childBounds);
        }

        return bounds;
    }

    protected onNodeAt(point: Point2f): Maybe<GskRenderNode> {
        for (const child of this.fChildren) {
            const testResult = child.nodeAt(point);
            if (testResult.has()) {
                return testResult;
            }
        }
        return Maybe.None();
    }

    protected onRender(dl: GskDisplayList, context: RenderContext): void {
        GskScopedRenderContext(dl, context, (mutator) => {
            mutator.setIsolation(this.bounds, dl.getTotalMatrix(), this.fRequiresIsolation);
            for (const child of this.fChildren) {
                child.render(dl, mutator.asRC());
            }
        });
    }
}
