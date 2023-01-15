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

import * as GL from 'synthetic://glamor';

export enum PointerHoveringState {
    kEnter,
    kLeave
}

export enum PointerButton {
    kLeft,
    kRight,
    kMiddle,
    kSide,
    kExtra,
    kForward,
    kBack,
    kTask
}

export enum PointerAxisSource {
    kWheel,
    kWheelTilt,
    kFinger,
    kContinuous,
    kUnknown
}

export interface WindowEventHandler {
    onCloseRequest?(): void;

    onResizeRequest?(width: number, height: number): void;

    onInResizingChanged?(resizing: boolean): void;
    onFullScreenChanged?(fullscreen: boolean): void;
    onActivatedChanged?(activated: boolean): void;
    onMaximizedChanged?(maximized: boolean): void;

    onFrame?(): void;

    onPointerHovering?(state: PointerHoveringState): void;
    onPointerMotion?(x: number, y: number): void;
    onPointerButton?(button: PointerButton, pressed: boolean): void;
    onPointerAxis?(sourceType: PointerAxisSource, dx: number, dy: number): void;

    onKeyboardFocus?(focused: boolean): void;
    onKeyboardKey?(key: GL.KeyboardKey, modifiers: GL.KeyboardModifiers, pressed: boolean): void;
}

const signalNameHandlerMap = [
    {
        signal: 'close',
        handlerFunc: 'onCloseRequest',
        receiver(handler: WindowEventHandler) {
            handler.onCloseRequest();
        }
    },
    {
        signal: 'frame',
        handlerFunc: 'onFrame',
        receiver(handler: WindowEventHandler) {
            handler.onFrame();
        }
    },
    {
        signal: 'pointer-hovering',
        handlerFunc: 'onPointerHovering',
        receiver(handler: WindowEventHandler, enter: boolean) {
            handler.onPointerHovering(enter ? PointerHoveringState.kEnter : PointerHoveringState.kLeave);
        }
    },
    {
        signal: 'pointer-motion',
        handlerFunc: 'onPointerMotion',
        receiver(handler: WindowEventHandler, x: number, y: number) {
            handler.onPointerMotion(x, y);
        }
    },
    {
        signal: 'pointer-button',
        handlerFunc: 'onPointerButton',
        receiver(handler: WindowEventHandler, button: GL.PointerButton, pressed: boolean) {
            let btn: PointerButton;
            switch (button) {
                case GL.Constants.POINTER_BUTTON_LEFT:
                    btn = PointerButton.kLeft;
                    break;
                case GL.Constants.POINTER_BUTTON_RIGHT:
                    btn = PointerButton.kRight;
                    break;
                case GL.Constants.POINTER_BUTTON_MIDDLE:
                    btn = PointerButton.kMiddle;
                    break;
                case GL.Constants.POINTER_BUTTON_EXTRA:
                    btn = PointerButton.kExtra;
                    break;
                case GL.Constants.POINTER_BUTTON_SIDE:
                    btn = PointerButton.kSide;
                    break;
                case GL.Constants.POINTER_BUTTON_BACK:
                    btn = PointerButton.kBack;
                    break;
                case GL.Constants.POINTER_BUTTON_FORWARD:
                    btn = PointerButton.kForward;
                    break;
                case GL.Constants.POINTER_BUTTON_TASK:
                    btn = PointerButton.kTask;
                    break;
                default:
                    return;
            }
            handler.onPointerButton(btn, pressed);
        }
    },
    {
        signal: 'pointer-axis',
        handlerFunc: 'onPointerAxis',
        receiver(handler: WindowEventHandler, sourceType: GL.PointerAxisSource, dx: number, dy: number) {
            let src: PointerAxisSource;
            switch (sourceType) {
                case GL.Constants.POINTER_AXIS_SOURCE_WHEEL:
                    src = PointerAxisSource.kWheel;
                    break;
                case GL.Constants.POINTER_AXIS_SOURCE_WHEEL_TILT:
                    src = PointerAxisSource.kWheelTilt;
                    break;
                case GL.Constants.POINTER_AXIS_SOURCE_FINGER:
                    src = PointerAxisSource.kFinger;
                    break;
                case GL.Constants.POINTER_AXIS_SOURCE_CONTINUOUS:
                    src = PointerAxisSource.kContinuous;
                    break;
                case GL.Constants.POINTER_AXIS_SOURCE_UNKNOWN:
                    src = PointerAxisSource.kUnknown;
                    break;
                default:
                    return;
            }
            handler.onPointerAxis(src, dx, dy);
        }
    },
    {
        signal: 'keyboard-key',
        handlerFunc: 'onKeyboardKey',
        receiver(handler: WindowEventHandler, key: GL.KeyboardKey, mod: GL.KeyboardModifiers, pressed: boolean) {
            handler.onKeyboardKey(key, mod, pressed);
        }
    },
    {
        signal: 'keyboard-focus',
        handlerFunc: 'onKeyboardFocus',
        receiver(handler: WindowEventHandler, focused: boolean) {
            handler.onKeyboardFocus(focused);
        }
    }
];

export interface WindowStatus {
    activated: boolean;
    fullscreen: boolean;
    maximized: boolean;
    resizing: boolean;
}

export class Window {
    static async Create(display: GL.Display, width: number, height: number, hwCompose: boolean): Promise<Window> {
        if (width <= 0 || height <= 0) {
            throw RangeError(`Invalid window size: ${width}x${height}`);
        }

        let surface: GL.Surface = null;
        if (hwCompose) {
            surface = await display.createHWComposeSurface(width, height);
        } else {
            surface = await display.createRasterSurface(width, height);
        }

        const blender = await surface.createBlender();

        return new Window(display, surface, blender);
    }

    readonly status: WindowStatus;
    #eventHandler: WindowEventHandler;
    #surfaceConnections: GL.SlotID[];
    #surfaceConfigureConnection: GL.SlotID;

    private constructor(public readonly display: GL.Display,
                        public readonly surface: GL.Surface,
                        public readonly blender: GL.Blender)
    {
        this.#eventHandler = null;
        this.#surfaceConnections = [];
        this.status = {
            activated: false,
            fullscreen: false,
            maximized: false,
            resizing: false
        };
        this.#surfaceConfigureConnection =
            surface.connect('configure', (width: number, height: number, states: GL.TopLevelStates) => {
                this._onSurfaceConfigure(width, height, states);
            });
    }

    public async close(): Promise<void> {
        this.surface.disconnect(this.#surfaceConfigureConnection);
        return this.blender.dispose().then(() => {
            return this.surface.close();
        });
    }

    private _clearSurfaceConnections(): void {
        for (let slot of this.#surfaceConnections) {
            this.surface.disconnect(slot);
        }
        this.#surfaceConnections = [];
    }

    private _surfaceConnect(signal: string, callback: Function): void {
        const slot = this.surface.connect(signal, callback);
        this.#surfaceConnections.push(slot);
    }

    private _onSurfaceConfigure(width: number, height: number, state: GL.TopLevelStates): void {
        const newStatus: WindowStatus = {
            activated: (state & GL.Surface.TOPLEVEL_ACTIVATED) == GL.Surface.TOPLEVEL_ACTIVATED,
            fullscreen: (state & GL.Surface.TOPLEVEL_FULLSCREEN) == GL.Surface.TOPLEVEL_FULLSCREEN,
            maximized: (state & GL.Surface.TOPLEVEL_MAXIMIZED) == GL.Surface.TOPLEVEL_MAXIMIZED,
            resizing: (state & GL.Surface.TOPLEVEL_RESIZING) == GL.Surface.TOPLEVEL_RESIZING
        };
        if (newStatus.activated != this.status.activated) {
            this.status.activated = newStatus.activated;
            if (this.#eventHandler?.onActivatedChanged) {
                this.#eventHandler.onActivatedChanged(newStatus.activated);
            }
        }
        if (newStatus.fullscreen != this.status.fullscreen) {
            this.status.fullscreen = newStatus.fullscreen;
            if (this.#eventHandler?.onFullScreenChanged) {
                this.#eventHandler.onFullScreenChanged(newStatus.fullscreen);
            }
        }
        if (newStatus.maximized != this.status.maximized) {
            this.status.maximized = newStatus.maximized;
            if (this.#eventHandler?.onMaximizedChanged) {
                this.#eventHandler.onMaximizedChanged(newStatus.maximized);
            }
        }
        if (newStatus.resizing != this.status.resizing) {
            this.status.resizing = newStatus.resizing;
            if (this.#eventHandler?.onInResizingChanged) {
                this.#eventHandler.onInResizingChanged(newStatus.resizing);
            }
        }

        if ((width != this.surface.width || height != this.surface.height) &&
             this.#eventHandler.onResizeRequest)
        {
            this.#eventHandler.onResizeRequest(width, height);
        }
    }

    public set eventHandler(handler: WindowEventHandler) {
        if (!handler) {
            this._clearSurfaceConnections();
            return;
        }

        this.#eventHandler = handler;

        for (let entry of signalNameHandlerMap) {
            if (typeof handler[entry.handlerFunc] != 'function') {
                continue;
            }
            const boundReceiver = entry.receiver.bind(this, handler);
            this._surfaceConnect(entry.signal, boundReceiver);
        }
    }

    public get eventHandler(): WindowEventHandler {
        return this.#eventHandler;
    }
}
