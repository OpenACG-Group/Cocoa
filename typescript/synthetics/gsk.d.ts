type SlotID = number;

export class SignalEmitter {
    connect(slot: string, callback: Function): SlotID;
    disconnect(id: SlotID): void;
    clearConnections(): void;
}

export class Display extends SignalEmitter {
    readonly closed: boolean;
    readonly monitorsCount: number;
    readonly hasSurfaceComposited: boolean;
    readonly hasSurfaceAlphaChannel: boolean;
    readonly hasInputShapes: boolean;

    close(): void;
    sync(): void;
    flush(): void;
    getMonitor(index: number): Monitor;
}

type SubpixelLayout = number;
export class Monitor extends SignalEmitter {
    static readonly SUBPIXEL_NONE: SubpixelLayout;
    static readonly SUBPIXEL_UNKNOWN: SubpixelLayout;
    static readonly SUBPIXEL_VERTICAL_RGB: SubpixelLayout;
    static readonly SUBPIXEL_VERTICAL_BGR: SubpixelLayout;
    static readonly SUBPIXEL_HORIZONTAL_RGB: SubpixelLayout;
    static readonly SUBPIXEL_HORIZONTAL_BGR: SubpixelLayout;

    readonly geometryWidth: number;
    readonly geometryHeight: number;
    readonly geometryX: number;
    readonly geometryY: number;
    readonly widthMM: number;
    readonly heightMM: number;
    readonly manufacturer: string;
    readonly model: string;
    readonly connector: string;
    readonly scaleFactor: number;
    readonly refreshRate: number;
    readonly subpixelLayout: SubpixelLayout;
    readonly valid: boolean;
}

/**
 * Connect to display.
 *
 * @param displayName An optional name of the display which will be connected to.
 *
 * @note Multiple backends are supported (typically X11 and Wayland on Linux platform).
 *       Wayland backend will be used if Wayland compositor is available, or X11 backend
 *       will be used if Xorg display server is available. If both of them are unavailable
 *       this function throws an exception.
 *       Specify commandline options to force to use one of them.
 */
export function Connect(displayName?: string): Display;
