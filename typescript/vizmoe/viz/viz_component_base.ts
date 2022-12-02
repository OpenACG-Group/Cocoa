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

import { Vec2f } from '../math';
import { LinkedList } from '../../core/linked_list';
import { VizPositioner } from './viz_relative_positioner';

export class VizComponentError {
    constructor(public what: string, public componentName: string) {}
}

export const MAX_REGISTERED_SLOTS = 256;

export class Signal<KWArgsObject> {
    #name: string;
    #signalId: number;
    #slotIdCounter: number;
    #slots: Array<{ id: number, callback: (args: KWArgsObject) => void | undefined }>;
    #slotsFreeIndexCacheStack: Array<number>;

    constructor(name: string, id: number) {
        this.#name = name;
        this.#signalId = id;
        this.#slotIdCounter = 0;
        this.#slots = [];
        this.#slotsFreeIndexCacheStack = [];
    }

    public get name(): string {
        return this.#name;
    }

    public connect(callback: (kwargs: KWArgsObject) => void): number {
        if (this.#slotIdCounter >= MAX_REGISTERED_SLOTS) {
            throw Error('Too many registered slot callbacks');
        }

        const slotIdPrefix = (this.#signalId << 16) | (this.#slotIdCounter << 8);

        // Find a possible free index to use
        if (this.#slotsFreeIndexCacheStack.length > 0) {
            const index = this.#slotsFreeIndexCacheStack.pop();
            if (index == undefined || index >= MAX_REGISTERED_SLOTS) {
                throw Error('Unexpected index value');
            }
            this.#slots[index].id = slotIdPrefix | index;
            this.#slots[index].callback = callback;

            this.#slotIdCounter++;
            return slotIdPrefix | index;
        }

        // Otherwise, append a new slot entry
        const slotId = slotIdPrefix | this.#slots.length;
        this.#slots.push({ id: slotId, callback: callback });

        this.#slotIdCounter++;
        return slotId;
    }

    public disconnect(slotId: number): void {
        if ((slotId >>> 16) != this.#signalId) {
            throw Error('Specified slot does NOT belong to this signal');
        }

        const index = slotId & 0xff;
        if (index >= this.#slots.length) {
            throw Error('Invalid index extracted from the slot ID');
        }

        if (this.#slots[index].callback == undefined) {
            throw Error('The corresponding slot has been disconnected');
        }

        this.#slots[index].callback = undefined;
        this.#slotsFreeIndexCacheStack.push(index);
        this.#slotIdCounter--;
    }

    public emit(kwargs: KWArgsObject): void {
        this.#slots.forEach((value) => {
            if (value.callback != undefined) {
                try {
                    value.callback(kwargs);
                } catch (e) {
                    // TODO(sora): report error & error handling
                }
            }
        });
    }
}

export class VizComponentBase {
    public readonly name: string;
    
    public parent: VizComponentBase | null;
    
    public children: LinkedList<VizComponentBase>;

    /**
     * Specify a relative position for the component.
     * If property `anchor` is null, it will be used at layout stage.
     */
    public positioner: VizPositioner.Box;

    // `parent` can be null if the component is the root node
    // (of the global component tree or a temporary component tree)
    constructor(name: string, parent: VizComponentBase | null) {
        this.name = name;
        this.parent = parent;
        this.children = new LinkedList<VizComponentBase>();
        this.positioner = VizPositioner.MakeAutoBox();
    }
}
