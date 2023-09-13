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
import { EventEmitterBase as NativeEmitter } from 'synthetic://private/base';

export class Event {
    public readonly name: string;
    public readonly timestampMs: number;

    constructor(timestampMs: number = -1) {
        this.name = this.constructor.name;
        this.timestampMs = timestampMs > 0 ? timestampMs : getMillisecondTimeCounter();
    }
}

type EventConstructor<T extends Event> = new(...args: any[]) => T;

interface PerListener {
    listener: Function;
    nativeListener: Function | null;
}

interface PerEvent {
    listeners: LinkedList<PerListener>;
    nativeForward: null | {
        emitter: NativeEmitter;
        event: string;
        adaptor: (...args: any[]) => any;
    };
}

export class EventEmitter {
    private fEventRegistry: Map<Function, PerEvent>;

    protected constructor() {
        this.fEventRegistry = new Map();
    }

    protected registerEvent<T extends Event>(ctor: EventConstructor<T>): void {
        if (this.fEventRegistry.has(ctor)) {
            throw Error(`Event ${ctor.name} has been registered`);
        }
        this.fEventRegistry.set(ctor, {
            listeners: new LinkedList<PerListener>(),
            nativeForward: null
        });
    }

    /**
     * Register an event forwarding an event emitted by native emitter.
     * Native emitters are native objects who inherit (extend) the native
     * `EventEmitterBase` (defined in `//typescript/synthetics/private/base.d.ts`).
     *
     * Adding or removing listeners for events registered by `forwardNative()`
     * is equivalent to do the same operation on the native emitter. That is,
     * if a listener `F` is added to the event by `addEventListener()`, then
     * a listener `G` is also added to the native emitter simultaneously.
     * But note that `G` is a different callback function from `F`. `G` is a
     * wrapper function of `F`, and `F` is called by calling `G`, so the real
     * listener `F` will be called when native emitter calls `G`.
     *
     * @param nativeEmitter   The native emitter.
     * @param event           Event name defined by native emitter.
     * @param ctor            Constructor of event object (same to `registerEvent()`).
     * @param adaptor         A callback that will be called when the native
     *                        emitter emits the event. It converts arguments
     *                        given by the native emitter to an event object,
     *                        which will be emitted immediately.
     */
    protected forwardNative<T extends Event>(nativeEmitter: NativeEmitter,
                                             event: string,
                                             ctor: EventConstructor<T>,
                                             adaptor: (...args: any[]) => T): void
    {
        if (this.fEventRegistry.has(ctor)) {
            throw Error(`Event ${ctor.name} has been registered`);
        }

        this.fEventRegistry.set(ctor, {
            listeners: new LinkedList<PerListener>(),
            nativeForward: {
                emitter: nativeEmitter,
                event: event,
                adaptor: adaptor
            }
        });
    }

    protected emitEvent<T extends Event>(event: T): void {
        if (!this.fEventRegistry.has(event.constructor)) {
            throw TypeError(`Event ${event.constructor.name} is not registered`);
        }

        const perEvent = this.fEventRegistry.get(event.constructor);
        if (perEvent.nativeForward) {
            throw Error(`Native forward event ${event.constructor.name} could not be emitted by emitEvent()`);
        }

        perEvent.listeners.forEach((listener: PerListener) => {
            listener.listener(event);
            return true;
        });
    }

    public addEventListener<T extends Event>(ctor: EventConstructor<T>, callback: (event: T) => void): void {
        if (!this.fEventRegistry.has(ctor)) {
            throw TypeError(`Event ${ctor.name} is not registered on this emitter`);
        }

        const perEvent = this.fEventRegistry.get(ctor);
        if (perEvent.nativeForward) {
            const native = perEvent.nativeForward;

            // `__native_event_trampoline` is a recognizable name for
            // stack backtrace.
            const __native_event_trampoline = (...args: any[]) => {
                const event = native.adaptor(...args) as T;
                if (event != undefined) {
                    callback(event);
                }
            };

            // Add `handler` as a listener to the native EventEmitter.
            // `handler` will be called when the event is emitted by native emitter,
            // then `handler` calls the real listener `callback()`.
            native.emitter.addListener(native.event, __native_event_trampoline);

            perEvent.listeners.push({
                listener: callback,
                nativeListener: __native_event_trampoline
            });
        } else {
            // Normal listeners
            perEvent.listeners.push({
                listener: callback,
                nativeListener: null
            });
        }
    }

    public removeEventListener<T extends Event>(ctor: EventConstructor<T>, callback: (event: T) => void): void {
        if (!this.fEventRegistry.has(ctor)) {
            throw TypeError(`Event ${ctor.name} is not registered on this emitter`);
        }

        const perEvent = this.fEventRegistry.get(ctor);
        perEvent.listeners.removeIf((perListener: PerListener) => {
            if (perListener.listener != callback)
                return false;

            if (perListener.nativeListener) {
                const native = perEvent.nativeForward;
                native.emitter.removeListener(native.event, perListener.nativeListener);
            }

            return true;
        });
    }

    public removeAllListeners<T extends Event>(ctor: EventConstructor<T>): void {
        if (!this.fEventRegistry.has(ctor)) {
            throw TypeError(`Event ${ctor.name} is not registered on this emitter`);
        }

        const perEvent = this.fEventRegistry.get(ctor);

        // Iterate to remove all the callbacks, and also remove native
        // listeners. Never use `native.emitter.removeAllListeners()`
        // as there may be other listeners (not registered by this emitter)
        // registered on the native emitter.
        perEvent.listeners.removeIf((perListener: PerListener) => {
            if (perListener.nativeListener) {
                const native = perEvent.nativeForward;
                native.emitter.removeListener(native.event, perListener.nativeListener);
            }
            return true;
        });
    }

    public isRegisteredEvent<T extends Event>(ctor: EventConstructor<T>): boolean {
        return this.fEventRegistry.has(ctor);
    }

    public eventConstructorNames(): Array<string> {
        return [...this.fEventRegistry.keys()].map(ctor => ctor.name);
    }
}
