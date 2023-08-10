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

import * as gl from 'glamor';
import { DrawContext } from './draw-context';
import { EventEmitter, Event } from '../base/event-dispatcher';

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

export enum PointerButton {
    kLeft = gl.Constants.POINTER_BUTTON_LEFT,
    kRight = gl.Constants.POINTER_BUTTON_RIGHT,
    kMiddle = gl.Constants.POINTER_BUTTON_MIDDLE,
    kSide = gl.Constants.POINTER_BUTTON_SIDE,
    kExtra = gl.Constants.POINTER_BUTTON_EXTRA,
    kForward = gl.Constants.POINTER_BUTTON_FORWARD,
    kBack = gl.Constants.POINTER_BUTTON_BACK,
    kTask = gl.Constants.POINTER_BUTTON_TASK
}

export enum PointerButtonState {
    kPressed,
    kReleased
}

export enum PointerAxisSource {
    kWheel = gl.Constants.POINTER_AXIS_SOURCE_WHEEL,
    kWheelTilt = gl.Constants.POINTER_AXIS_SOURCE_WHEEL_TILT,
    kFinger = gl.Constants.POINTER_AXIS_SOURCE_FINGER,
    kContinuous = gl.Constants.POINTER_AXIS_SOURCE_CONTINUOUS,
    kUnknown = gl.Constants.POINTER_AXIS_SOURCE_UNKNOWN
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
                public readonly source: PointerAxisSource,
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

export enum KeyboardKey {
    kSPACE = gl.Constants.KEY_SPACE,
    kAPOSTROPHE = gl.Constants.KEY_APOSTROPHE,
    kCOMMA = gl.Constants.KEY_COMMA,
    kMINUS = gl.Constants.KEY_MINUS,
    kPERIOD = gl.Constants.KEY_PERIOD,
    kSLASH = gl.Constants.KEY_SLASH,
    k0 = gl.Constants.KEY_0,
    k1 = gl.Constants.KEY_1,
    k2 = gl.Constants.KEY_2,
    k3 = gl.Constants.KEY_3,
    k4 = gl.Constants.KEY_4,
    k5 = gl.Constants.KEY_5,
    k6 = gl.Constants.KEY_6,
    k7 = gl.Constants.KEY_7,
    k8 = gl.Constants.KEY_8,
    k9 = gl.Constants.KEY_9,
    kSEMICOLON = gl.Constants.KEY_SEMICOLON,
    kEQUAL = gl.Constants.KEY_EQUAL,
    kA = gl.Constants.KEY_A,
    kB = gl.Constants.KEY_B,
    kC = gl.Constants.KEY_C,
    kD = gl.Constants.KEY_D,
    kE = gl.Constants.KEY_E,
    kF = gl.Constants.KEY_F,
    kG = gl.Constants.KEY_G,
    kH = gl.Constants.KEY_H,
    kI = gl.Constants.KEY_I,
    kJ = gl.Constants.KEY_J,
    kK = gl.Constants.KEY_K,
    kL = gl.Constants.KEY_L,
    kM = gl.Constants.KEY_M,
    kN = gl.Constants.KEY_N,
    kO = gl.Constants.KEY_O,
    kP = gl.Constants.KEY_P,
    kQ = gl.Constants.KEY_Q,
    kR = gl.Constants.KEY_R,
    kS = gl.Constants.KEY_S,
    kT = gl.Constants.KEY_T,
    kU = gl.Constants.KEY_U,
    kV = gl.Constants.KEY_V,
    kW = gl.Constants.KEY_W,
    kX = gl.Constants.KEY_X,
    kY = gl.Constants.KEY_Y,
    kZ = gl.Constants.KEY_Z,
    kLEFT_BRACKET = gl.Constants.KEY_LEFT_BRACKET,
    kBACKSLASH = gl.Constants.KEY_BACKSLASH,
    kRIGHT_BRACKET = gl.Constants.KEY_RIGHT_BRACKET,
    kGRAVE_ACCENT = gl.Constants.KEY_GRAVE_ACCENT,
    kWORLD_1 = gl.Constants.KEY_WORLD_1,
    kWORLD_2 = gl.Constants.KEY_WORLD_2,
    kESCAPE = gl.Constants.KEY_ESCAPE,
    kENTER = gl.Constants.KEY_ENTER,
    kTAB = gl.Constants.KEY_TAB,
    kBACKSPACE = gl.Constants.KEY_BACKSPACE,
    kINSERT = gl.Constants.KEY_INSERT,
    kDELETE = gl.Constants.KEY_DELETE,
    kRIGHT = gl.Constants.KEY_RIGHT,
    kLEFT = gl.Constants.KEY_LEFT,
    kDOWN = gl.Constants.KEY_DOWN,
    kUP = gl.Constants.KEY_UP,
    kPAGE_UP = gl.Constants.KEY_PAGE_UP,
    kPAGE_DOWN = gl.Constants.KEY_PAGE_DOWN,
    kHOME = gl.Constants.KEY_HOME,
    kEND = gl.Constants.KEY_END,
    kCAPS_LOCK = gl.Constants.KEY_CAPS_LOCK,
    kSCROLL_LOCK = gl.Constants.KEY_SCROLL_LOCK,
    kNUM_LOCK = gl.Constants.KEY_NUM_LOCK,
    kPRINT_SCREEN = gl.Constants.KEY_PRINT_SCREEN,
    kPAUSE = gl.Constants.KEY_PAUSE,
    kF1 = gl.Constants.KEY_F1,
    kF2 = gl.Constants.KEY_F2,
    kF3 = gl.Constants.KEY_F3,
    kF4 = gl.Constants.KEY_F4,
    kF5 = gl.Constants.KEY_F5,
    kF6 = gl.Constants.KEY_F6,
    kF7 = gl.Constants.KEY_F7,
    kF8 = gl.Constants.KEY_F8,
    kF9 = gl.Constants.KEY_F9,
    kF10 = gl.Constants.KEY_F10,
    kF11 = gl.Constants.KEY_F11,
    kF12 = gl.Constants.KEY_F12,
    kF13 = gl.Constants.KEY_F13,
    kF14 = gl.Constants.KEY_F14,
    kF15 = gl.Constants.KEY_F15,
    kF16 = gl.Constants.KEY_F16,
    kF17 = gl.Constants.KEY_F17,
    kF18 = gl.Constants.KEY_F18,
    kF19 = gl.Constants.KEY_F19,
    kF20 = gl.Constants.KEY_F20,
    kF21 = gl.Constants.KEY_F21,
    kF22 = gl.Constants.KEY_F22,
    kF23 = gl.Constants.KEY_F23,
    kF24 = gl.Constants.KEY_F24,
    kF25 = gl.Constants.KEY_F25,
    kKP_0 = gl.Constants.KEY_KP_0,
    kKP_1 = gl.Constants.KEY_KP_1,
    kKP_2 = gl.Constants.KEY_KP_2,
    kKP_3 = gl.Constants.KEY_KP_3,
    kKP_4 = gl.Constants.KEY_KP_4,
    kKP_5 = gl.Constants.KEY_KP_5,
    kKP_6 = gl.Constants.KEY_KP_6,
    kKP_7 = gl.Constants.KEY_KP_7,
    kKP_8 = gl.Constants.KEY_KP_8,
    kKP_9 = gl.Constants.KEY_KP_9,
    kKP_DECIMAL = gl.Constants.KEY_KP_DECIMAL,
    kKP_DIVIDE = gl.Constants.KEY_KP_DIVIDE,
    kKP_MULTIPLY = gl.Constants.KEY_KP_MULTIPLY,
    kKP_SUBTRACT = gl.Constants.KEY_KP_SUBTRACT,
    kKP_ADD = gl.Constants.KEY_KP_ADD,
    kKP_ENTER = gl.Constants.KEY_KP_ENTER,
    kKP_EQUAL = gl.Constants.KEY_KP_EQUAL,
    kLEFT_SHIFT = gl.Constants.KEY_LEFT_SHIFT,
    kLEFT_CONTROL = gl.Constants.KEY_LEFT_CONTROL,
    kLEFT_ALT = gl.Constants.KEY_LEFT_ALT,
    kLEFT_SUPER = gl.Constants.KEY_LEFT_SUPER,
    kRIGHT_SHIFT = gl.Constants.KEY_RIGHT_SHIFT,
    kRIGHT_CONTROL = gl.Constants.KEY_RIGHT_CONTROL,
    kRIGHT_ALT = gl.Constants.KEY_RIGHT_ALT,
    kRIGHT_SUPER = gl.Constants.KEY_RIGHT_SUPER,
    kMENU = gl.Constants.KEY_MENU
}

export enum KeyboardModifiers {
    kControl = gl.Constants.MODIFIER_CONTROL,
    kAlt = gl.Constants.MODIFIER_ALT,
    kShift = gl.Constants.MODIFIER_SHIFT,
    kSuper = gl.Constants.MODIFIER_SUPER,
    kCapsLock = gl.Constants.MODIFIER_CAPS_LOCK,
    kNumLock = gl.Constants.MODIFIER_NUM_LOCK
}

export class KeyboardKeyEvent extends ToplevelWindowEvent {
    constructor(window: ToplevelWindow,
                public readonly modifiers: number,
                public readonly key: KeyboardKey,
                public readonly pressed: boolean) {
        super(window);
    }
}

export class ToplevelWindow extends EventEmitter {
    private readonly fDisplay: gl.Display;
    private readonly fSurface: gl.Surface;
    private readonly fDrawContext: DrawContext;
    private fClosed: boolean;

    public static async Create(display: gl.Display, width: number,
                               height: number, hwCompose: boolean = false): Promise<ToplevelWindow>
    {
        const hasHWCompose = gl.queryCapabilities(
            gl.Constants.CAPABILITY_HWCOMPOSE_ENABLED);
        if (hwCompose && !hasHWCompose) {
            throw Error('ToplevelWindow requires HWCompose feature, but it is disabled');
        }

        let surface: gl.Surface = null;
        if (hwCompose) {
            surface = await display.createHWComposeSurface(width, height);
        } else {
            surface = await display.createRasterSurface(width, height);
        }

        const drawContext = await DrawContext.Make(surface);
        return new ToplevelWindow(display, surface, drawContext);
    }

    private constructor(display: gl.Display, surface: gl.Surface, DC: DrawContext) {
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
            (button: gl.PointerButton, pressed: boolean) => {
                return new PointerButtonEvent(this, button as PointerButton,
                    pressed ? PointerButtonState.kPressed : PointerButtonState.kReleased);
            }
        );

        this.forwardNative(surface, 'pointer-axis', PointerAxisEvent,
            (source: gl.PointerAxisSource, dx: number, dy: number) => {
                return new PointerAxisEvent(this, source as PointerAxisSource, dx, dy);
            }
        );

        this.forwardNative(surface, 'keyboard-focus', KeyboardFocusEvent,
            (focused: boolean) => {
                return new KeyboardFocusEvent(this, focused);
            }
        );

        this.forwardNative(surface, 'keyboard-key', KeyboardKeyEvent,
            (key: gl.KeyboardKey, modifiers: gl.KeyboardModifiers, pressed: boolean) => {
                return new KeyboardKeyEvent(this, modifiers, key as KeyboardKey, pressed);
            }
        );
    }

    public async close(): Promise<void> {
        if (this.fClosed) {
            throw Error('Window has been closed');
        }

        await this.fDrawContext.dispose();

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

    public get display(): gl.Display {
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

    public requestNextFrame(): Promise<void> {
        return this.fSurface.requestNextFrame();
    }

    public resize(width: number, height: number): Promise<void> {
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

    public setFullscreen(value: boolean, monitor: gl.Monitor): Promise<void> {
        return this.fSurface.setFullscreen(value, monitor);
    }
}
