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

import {Buffer} from 'core';

/**
 * Error class thrown when a asynchronous operation fails.
 * Promise may be rejected with its instance.
 * @property opcode     A unique code of the asynchronous operation.
 */
export interface RenderClientError extends Error {
    opcode: number;
}

/**
 * Basic information of the application that should be provided for
 * the `RenderHost.Initialize` method.
 * Provided information like the name of the application will not be parsed.
 * The only use of them is to identify user's application.
 */
export interface ApplicationInfo {
    // Name of the application
    name: string;

    // Major version number
    major: number;
    
    // Minor version number
    minor: number;

    // Patch version number
    patch: number;
}

/**
 * RenderHost, aka local thread, is an interface to communicate with
 * rendering thread in Glamor's multithreading architecture.
 * RenderHost can initialize and deinitialize the global Glamor context,
 * and create a Display instance.
 */
export class RenderHost {
    /**
     * Initialize the global Glamor context.
     * As long as the context is still alive, the rendering thread keeps running
     * and the event loop will not exit.
     * 
     * @param info Basic information of the application.
     */
    public static Initialize(info: ApplicationInfo): void;

    /**
     * Deinitialize the global Glamor context.
     * Rendering thread will be stopped, pending requests will be rejected,
     * and pending tasks will be canceled.
     * All the objects and resources that you get or create from Glamor will
     * be invalid when the `Dispose` function returns. It is undefined behavior
     * if the caller still use them.
     */
    public static Dispose(): void;

    /**
     * Connect to the platform's display server and return an abstracted interface
     * that can interact with the display server.
     *
     * @param name Optional display name.
     */
    public static Connect(name?: string): Promise<Display>;

    /**
     * Send a "sync" request to the rendering thread then block the JavaScript thread
     * until the rendering thread has responded the request.
     * The returning of this method means the message queue of rendering thread is empty
     * and all the previous requests have been processed and responded.
     * If any signals are emitted during the blocking period, they will be received
     * after the JavaScript thread resumes and enters the event loop again.
     * 
     * @param timeoutInMs   Time in milliseconds that it will wait for.
     *                      -1 or other negative values indicate to wait for infinite time.
     */
    public static WaitForSyncBarrier(timeoutInMs: number): void;

    /**
     * [Only used for testing purpose]
     */
    public static SleepRendererFor(timeoutInMs: number): Promise<void>;
}

// A unique number to identify a signal slot.
type SlotID = number;

/**
 * `RenderClientObject` is the base class of other classes that support
 * asynchronous method call and signal mechanism.
 */
export class RenderClientObject {
    /**
     * Connect a certain callback function (slot) to the specified signal.
     * The callback function will be fired in the event loop when the signal
     * is emitted (by rendering thread or current thread).
     * Note that when there are multiple slots connect to the same signal,
     * the order where the callback functions are called is unreliable,
     * so the caller should assume that they are called in random order.
     * 
     * @param signal Name of the signal to connect to.
     * @param slot A callback function.
     * @return Slot ID, a unique number within the scope of this object which represents
     *         the created connection (slot). It can be used with `disconnect` to disconnect
     *         the slot.
     */
    public connect(signal: string, slot: Function): SlotID;

    /**
     * Disconnect the specified slot with its signal. The corresponding
     * callback function will not be called anymore.
     * 
     * @param id A unique ID returned by `connect` method.
     */
    public disconnect(id: SlotID): void;

    /* Experimental interface. Undocumented. */
    public inspectObject(): object;
}

/**
 * An abstracted interface for system's display server (like Wayland, X11, etc.).
 * 
 * @signal [closed] Emitted when the display is closed.
 *         prototype: () -> void
 * 
 * @signal [monitor-added] Emitted when a new monitor was plugged into the system.
 *                         Note that the monitors that have existed in the system when the
 *                         display object is created will not cause this signal to be emitted.
 *         prototype: (monitor: Monitor) -> void
 * 
 * @signal [monitor-removed] Emitted when an existing monitor was removed.
 *         prototype: (monitor: Monitor) -> void
 */
export class Display extends RenderClientObject {
    /**
     * Close the display. This method should not be called for multiple times.
     * Signal `closed` will be emitted when the display is actually closed.
     * Promise will be resolved when the close request is processed.
     */
    public close(): Promise<void>;

    /**
     * Create a toplevel window (aka surface). All the rendering operations
     * performed on the created window will use an internal CPU rasterizer.
     * 
     * @param w Width of the new created window.
     * @param h Height of the new created window.
     */
    public createRasterSurface(w: number, h: number): Promise<Surface>;

    /**
     * Create a toplevel window (aka surface). All the rendering operations
     * performed on the created window will use the hardware-accelerated
     * GPU rasterizer, which is called HWCompose.
     * 
     * All the HWCompose surfaces share the same physical GPU and GPU context.
     * If there are multiple GPUs in the system configuration, Cocoa will choose a
     * suitable one to use, which is opaque to the caller.
     * If it is the first HWCompose surface that the caller created, it will lead to
     * the initialization of GPU context, which may be very slow. In the worst case,
     * it will take several seconds to complete.
     * 
     * @param w Width of the new created window.
     * @param h Height of the new created window.
     */
    public createHWComposeSurface(w: number, h: number): Promise<Surface>;
    public requestMonitorList(): Promise<Array<Monitor>>;
}

type MonitorSubpixel = number;
type MonitorTransform = number;
type MonitorMode = number;
export interface MonitorPropertySet {
    logicalX: number;
    logicalY: number;
    subpixel: MonitorSubpixel;
    manufactureName: string;
    modelName: string;
    transform: MonitorTransform;
    modeFlags: MonitorMode;
    modeWidth: number;
    modeHeight: number;
    refreshRate: number;
    scaleFactor: number;
    connectorName: string;
    description: string;
}

export class Monitor extends RenderClientObject {
    static readonly SUBPIXEL_UNKNOWN: MonitorSubpixel;
    static readonly SUBPIXEL_NONE: MonitorSubpixel;
    static readonly SUBPIXEL_HORIZONTAL_RGB: MonitorSubpixel;
    static readonly SUBPIXEL_HORIZONTAL_BGR: MonitorSubpixel;
    static readonly SUBPIXEL_VERTICAL_RGB: MonitorSubpixel;
    static readonly SUBPIXEL_VERTICAL_BGR: MonitorSubpixel;
    static readonly TRANSFORM_NORMAL: MonitorTransform;
    static readonly TRANSFORM_ROTATE_90: MonitorTransform;
    static readonly TRANSFORM_ROTATE_180: MonitorTransform;
    static readonly TRANSFORM_ROTATE_270: MonitorTransform;
    static readonly TRANSFORM_FLIPPED: MonitorTransform;
    static readonly TRANSFORM_FLIPPED_90: MonitorTransform;
    static readonly TRANSFORM_FLIPPED_180: MonitorTransform;
    static readonly TRANSFORM_FLIPPED_270: MonitorTransform;
    static readonly MODE_CURRENT: MonitorMode;
    static readonly MODE_PREFERRED: MonitorMode;

    public requestPropertySet(): Promise<void>;
}

export class Surface extends RenderClientObject {
    static readonly TOPLEVEL_MAXIMZED: number;
    static readonly TOPLEVEL_FULLSCREEN: number;
    static readonly TOPLEVEL_RESIZING: number;
    static readonly TOPLEVEL_ACTIVATED: number;
    static readonly TOPLEVEL_TILED_LEFT: number;
    static readonly TOPLEVEL_TILED_RIGHT: number;
    static readonly TOPLEVEL_TILED_TOP: number;
    static readonly TOPLEVEL_TILED_BOTTOM: number;

    public readonly width: number;
    public readonly height: number;

    public createBlender(): Promise<Blender>;
    public close(): Promise<void>;
    public getBufferDescriptor(): Promise<string>;
    public requestNextFrame(): Promise<void>;
    public setTitle(title: string): Promise<void>;
    public resize(width: number, height: number): Promise<void>;
    public setMinSize(width: number, height: number): Promise<void>;
    public setMaxSize(width: number, height: number): Promise<void>;
    public setMaximized(value: boolean): Promise<void>;
    public setMinimized(value: boolean): Promise<void>;
    public setFullscreen(value: boolean, monitor: Monitor): Promise<void>;
}

export class Blender extends RenderClientObject {
    public dispose(): Promise<void>;
    public update(scene: Scene): Promise<void>;
}

export class Scene {
    public dispose(): void;
    public toImage(width: number, height: number): Promise<CkImage>;
}

export class SceneBuilder {
    public constructor(viewportWidth: number, viewportHeight: number);
    public build(): Scene;
    public pop(): SceneBuilder;
    public addPicture(picture: CkPicture, dx: number, dy: number): SceneBuilder;
    public pushOffset(dx: number, dy: number): SceneBuilder;
}

// ===============================
// Basic Rendering Objects
// ===============================

interface Constants {
    readonly COLOR_TYPE_ALPHA8: number;
    readonly COLOR_TYPE_RGB565: number;
    readonly COLOR_TYPE_ARGB4444: number;
    readonly COLOR_TYPE_RGBA8888: number;
    readonly COLOR_TYPE_RGB888x: number;
    readonly COLOR_TYPE_BGRA8888: number;
    readonly COLOR_TYPE_BGRA1010102: number;
    readonly COLOR_TYPE_RGBA1010102: number;
    readonly COLOR_TYPE_RGB101010x: number;
    readonly COLOR_TYPE_BGR101010x: number;
    readonly COLOR_TYPE_GRAY8: number;
    readonly COLOR_TYPE_RGBA_F16_NORM: number;
    readonly COLOR_TYPE_RGBA_F16: number;
    readonly COLOR_TYPE_RGBA_F32: number;
    readonly COLOR_TYPE_R8G8_UNORM: number;
    readonly COLOR_TYPE_A16_FLOAT: number;
    readonly COLOR_TYPE_R16G16_FLOAT: number;
    readonly COLOR_TYPE_A16_UNORM: number;
    readonly COLOR_TYPE_R16G16_UNORM: number;
    readonly COLOR_TYPE_R16G16B16A16_UNORM: number;

    readonly ALPHA_TYPE_PREMULTIPLIED: number;
    readonly ALPHA_TYPE_UNPREMULTIPLIED: number;
    readonly ALPHA_TYPE_OPAQUE: number;

    readonly FORMAT_PNG: number;
    readonly FORMAT_JPEG: number;
    readonly FORMAT_WEBP: number;
}

export declare let Constants: Constants;

export interface CkRect {
    left: number;
    top: number;
    right: number;
    bottom: number;
}

export class CkBitmap {
}

export class CkImage {
    public static MakeFromEncodedData(buffer: Buffer): Promise<CkImage>;
    public static MakeFromEncodedFile(path: string): Promise<CkImage>;
    public encodeToData(format: number, quality: number): Buffer;
}

export class CkPicture {
    public static MakeFromData(buffer: Buffer): CkPicture;
    public static MakeFromFile(path: string): CkPicture;

    public serialize(): Buffer;
    public approximateOpCount(nested: boolean): number;
    public approximateByteUsed(): number;
    public uniqueId(): number;
}


export interface MoeHeapProfiling {
    heapSingleCellSize: number;
    heapTotalSize: number;
    heapAllocationsCount: number;
    heapExtractionsCount: number;
    heapLeakedCellsCount: number;
}

export interface MoeTranslationResult {
    artifact?: CkPicture;
    heapProfiling?: MoeHeapProfiling;
}

export class MoeHeapObjectBinder {
    constructor();

    public bindBitmap(key: number, bitmap: CkBitmap): void;
    public bindImage(key: number, image: CkImage): void;
    public bindPicture(key: number, picture: CkPicture): void;
    public bindString(key: number, string: string): void;
}

export class MoeTranslationToolchain {
    public static Interpreter(buffers: Array<Buffer>,
                              binder: MoeHeapObjectBinder,
                              breakpointHandler: (id: number) => void,
                              heapProfiling: boolean): MoeTranslationResult;
    public static Disassemble(buffers: Array<Buffer>): string;
    public static Compress(buffer: Array<Buffer>): Buffer;
}
