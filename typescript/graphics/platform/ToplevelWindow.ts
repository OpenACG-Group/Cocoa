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
    PointerButton,
    AxisSourceType,
    Surface,
    Display,
    Monitor,
    KeyboardKey,
    KeyboardModifiers
} from 'present';

import { DrawContext } from './DrawContext';
import { EventEmitter, Event } from '../../core/event-dispatcher';

export { PointerButton, AxisSourceType, KeyboardKey, KeyboardModifiers };

class ToplevelWindowEvent extends Event {
    public readonly window: ToplevelWindow;
    protected constructor(window: ToplevelWindow) {
        super();
        this.window = window;
    }
}

export class CloseRequestEvent extends ToplevelWindowEvent {
    constructor(window: ToplevelWindow) {
        super(window);
    }
}

export enum PointerHoverState {
    kEnter,
    kLeave
}

export enum PointerButtonState {
    kPressed,
    kReleased
}

export class PointerHoverEvent extends ToplevelWindowEvent {
    constructor(window: ToplevelWindow,
                public readonly state: PointerHoverState) {
        super(window);
    }
}

export class PointerMotionEvent extends ToplevelWindowEvent {
    constructor(window: ToplevelWindow,
                public readonly x: number,
                public readonly y: number) {
        super(window);
    }
}

export class PointerButtonEvent extends ToplevelWindowEvent {
    constructor(window: ToplevelWindow,
                public readonly button: PointerButton,
                public readonly state: PointerButtonState) {
        super(window);
    }
}

export class PointerAxisEvent extends ToplevelWindowEvent {
    constructor(window: ToplevelWindow,
                public readonly source: AxisSourceType,
                public readonly dx: number,
                public readonly dy: number) {
        super(window);
    }
}


export class KeyboardFocusEvent extends ToplevelWindowEvent {
    constructor(window: ToplevelWindow,
                public readonly focused: boolean) {
        super(window);
    }
}

export class KeyboardKeyEvent extends ToplevelWindowEvent {
    constructor(window: ToplevelWindow,
                public readonly modifiers: number,
                public readonly key: KeyboardKey,
                public readonly pressed: boolean) {
        super(window);
    }
}

/**
 * A high-level wrapper class for `Surface` in `native://present`, with typesafe events
 * and a DrawContext.
 */
export class ToplevelWindow extends EventEmitter {
    private readonly fDisplay: Display;
    private readonly fSurface: Surface;
    private readonly fDrawContext: DrawContext;
    private fClosed: boolean;

    public static async Create(display: Display, width: number,
                               height: number, hwaccel: boolean = false): Promise<ToplevelWindow>
    {
        let surface = await display.createSurface(width, height, hwaccel);
        const drawContext = await DrawContext.Make(surface);
        return new ToplevelWindow(display, surface, drawContext);
    }

    private constructor(display: Display, surface: Surface, DC: DrawContext) {
        super();
        this.fDisplay = display;
        this.fSurface = surface;
        this.fDrawContext = DC;
        this.fClosed = false;

        this.forwardNative(surface, 'close', CloseRequestEvent, () => {
            return new CloseRequestEvent(this);
        });

        this.forwardNative(surface, 'pointer-hovering', PointerHoverEvent,
            (enter: boolean) => {
                return new PointerHoverEvent(this,
                    enter ? PointerHoverState.kEnter : PointerHoverState.kLeave);
            }
        );

        this.forwardNative(surface, 'pointer-motion', PointerMotionEvent,
            (x: number, y: number) => { return new PointerMotionEvent(this, x, y); }
        );

        this.forwardNative(surface, 'pointer-button', PointerButtonEvent,
            (button: PointerButton, pressed: boolean) => {
                return new PointerButtonEvent(this, button,
                    pressed ? PointerButtonState.kPressed : PointerButtonState.kReleased);
            }
        );

        this.forwardNative(surface, 'pointer-axis', PointerAxisEvent,
            (source: AxisSourceType, dx: number, dy: number) => {
                return new PointerAxisEvent(this, source, dx, dy);
            }
        );

        this.forwardNative(surface, 'keyboard-focus', KeyboardFocusEvent,
            (focused: boolean) => {
                return new KeyboardFocusEvent(this, focused);
            }
        );

        this.forwardNative(surface, 'keyboard-key', KeyboardKeyEvent,
            (key: KeyboardKey, modifiers: KeyboardModifiers, pressed: boolean) => {
                return new KeyboardKeyEvent(this, modifiers, key, pressed);
            }
        );
    }

    public async close(): Promise<void> {
        if (this.fClosed) {
            throw Error('Window has been closed');
        }

        this.fDrawContext.dispose();

        this.removeAllListeners(CloseRequestEvent);
        this.removeAllListeners(PointerHoverEvent);
        this.removeAllListeners(PointerMotionEvent);
        this.removeAllListeners(PointerButtonEvent);
        this.removeAllListeners(PointerAxisEvent);
        this.removeAllListeners(KeyboardFocusEvent);
        this.removeAllListeners(KeyboardKeyEvent);

        await this.fSurface.close();
        this.fClosed = true;
    }

    public get display(): Display {
        return this.fDisplay;
    }

    public get drawContext(): DrawContext {
        return this.fDrawContext;
    }

    public get width(): number {
        return this.fDrawContext.width;
    }

    public get height(): number {
        return this.fDrawContext.height;
    }

    public setTitle(title: string): Promise<void> {
        return this.fSurface.setTitle(title);
    }

    public requestNextFrame(): Promise<number> {
        return this.fSurface.requestNextFrame();
    }

    public resize(width: number, height: number): Promise<boolean> {
        return this.fSurface.resize(width, height);
    }

    public setMinSize(width: number, height: number): Promise<void> {
        return this.fSurface.setMinSize(width, height);
    }

    public setMaxSize(width: number, height: number): Promise<void> {
        return this.fSurface.setMaxSize(width, height);
    }

    public setMaximized(value: boolean): Promise<void> {
        return this.fSurface.setMaximized(value);
    }

    public setMinimized(value: boolean): Promise<void> {
        return this.fSurface.setMinimized(value);
    }

    public setFullscreen(value: boolean, monitor: Monitor): Promise<void> {
        return this.fSurface.setFullscreen(value, monitor);
    }
}
