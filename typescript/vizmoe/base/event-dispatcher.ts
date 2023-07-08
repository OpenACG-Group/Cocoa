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

import { LinkedList } from '../../core/linked_list';

export class Event {
    public readonly name: string;
    public readonly timestampMs: number;

    constructor(timestampMs: number = -1) {
        this.name = this.constructor.name;
        this.timestampMs = timestampMs > 0 ? timestampMs : getMillisecondTimeCounter();
    }
}

type EventConstructor<T extends Event> = new(...args: any[]) => T;

export class EventEmitter {
    private fEventRegistry: Map<Function, LinkedList<Function>>;

    protected constructor() {
        this.fEventRegistry = new Map();
    }

    protected registerEvent<T extends Event>(ctor: EventConstructor<T>): void {
        if (this.fEventRegistry.has(ctor)) {
            throw Error(`Event ${ctor.name} has been registered`);
        }
        this.fEventRegistry.set(ctor, new LinkedList<Function>());
    }

    protected emitEvent<T extends Event>(event: T): void {
        if (!this.fEventRegistry.has(event.constructor)) {
            throw TypeError(`Event ${event.constructor.name} is not registered`);
        }
        this.fEventRegistry.get(event.constructor).forEach((callback: Function) => {
            callback(event);
        });
    }

    public addEventListener<T extends Event>(ctor: EventConstructor<T>, callback: (event: T) => void): void {
        if (!this.fEventRegistry.has(ctor)) {
            throw TypeError(`Event ${ctor.name} will never be emitted by this object`);
        }
        this.fEventRegistry.get(ctor).push(callback);
    }

    public removeEventListener<T extends Event>(ctor: EventConstructor<T>, callback: (event: T) => void): void {
        if (!this.fEventRegistry.has(ctor)) {
            throw TypeError(`Event ${ctor.name} will never be emitted by this object`);
        }
        this.fEventRegistry.get(ctor).removeIf((value) => {
            return (value == callback);
        });
    }
}
