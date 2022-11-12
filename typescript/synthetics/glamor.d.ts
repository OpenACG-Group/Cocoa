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
 * Error class thrown when an asynchronous operation fails.
 * Promise may be rejected with its instance.
 * @property opcode     A unique code of the asynchronous operation.
 */
export interface RenderClientError extends Error {
    opcode: number;
}

interface CkLTRBRect {
    left: number;
    top: number;
    bottom: number;
    right: number;
}
interface CkXYWHRect {
    x: number;
    y: number;
    width: number;
    height: number;
}
type CkRect = CkLTRBRect | CkXYWHRect | Array<number> | Float32Array;

export interface CkRRect {
   rect: CkRect;

    /**
     * Decide whether apply the same radii in X and Y directions.
     * If true, all the corners are arcs of a circle, otherwise, they are arcs of
     * the specified ellipse.
     */
   uniformRadii: boolean;

   /**
    * Symbols:
    *   TL: top-left, BL: bottom-left, TR: top-right, BR: bottom-right
    *
    * XY uniform radii:
    *   1 value:  [TL|BL|TR|BR]
    *   2 values: [TL|BR, TR|BL]
    *   3 values: [TL, TR|BL, BR]
    *   4 values: [TL, TR, BR, BL]
    *
    * XY discrete radii:
    *   1 value pair:  [TL<xy>|BL<xy>|TR<xy>|BR<xy>]
    *   2 value pairs: [TL<xy>|BR<xy>, TR<xy>|BL<xy>]
    *   3 value pairs: [TL<xy>, TR<xy>|BL<xy>, BR<xy>]
    *   4 value pairs: [TL<xy>, TR<xy>, BR<xy>, BL<xy>]
    */
   borderRadii: Float32Array | Array<number>;
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

// Experimental API (see `RenderHost.SetTypefaceTransferCallback`)
export interface TypefaceInfo {
    family: string;
    weight: number;
    width: number;
    slant: string;
}

type TypefaceTransferCallbackT = (info: TypefaceInfo) => Uint8Array;

/**
 * Query a certain capability of current Glamor context.
 * This function can be called before Glamor is initialized.
 * See `Constants.CAPABILITY_*` constants for available capabilities.
 */
export function queryCapabilities(cap: number): boolean | number | string;

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

    // Internal testing API
    public static SleepRendererFor(timeoutInMs: number): Promise<void>;

    /**
     * Get tracing information of graphics resources in JSON string format.
     * The JSON string returned can be written into a file, or be parsed
     * by `JSON.parse` for analysis purpose.
     */
    public static TraceGraphicsResources(): Promise<string>;

    /**
     * Set a callback function which is used for typeface transferring.
     * The callback will be fired when a typeface object is required during the
     * deserialization of Picture objects (if `CkPicture.USAGE_TRANSFER` is specified),
     * and the callee is supposed to return a certain serialized `SkTypeface` object.
     * Once a serialized typeface object is received, it will be cached internally
     * and Glamor will not request for the same typeface object anymore in the future.
     *
     * @note This is an internal API which should be used with CanvasKit.
     *       See `bindWithInitializedRenderHost` function in CanvasKit module.
     */
    public static SetTypefaceTransferCallback(callback: TypefaceTransferCallbackT): void;

    /**
     * Free all the critical graphics resources (e.g. `CriticalPicture` objects).
     * Critical resources is a special type of graphics resources whose ownerships
     * belong to JavaScript thread instead of rendering thread (GPU thread).
     * They usually refer to sensitive GPU resources (like textures in video memory),
     * which only can be accessed and destroyed in GPU thread, but for some reasons they
     * need to be used in JavaScript thread directly. Consequently, JavaScript thread
     * only retains weak references to actual resources, and user will be notified
     * when they are destroyed (see `CriticalPicture.setCollectionCallback` for example).
     *
     * In most cases, those critical resources will be released automatically,
     * depending on JavaScript GC, but user is also allowed to collect them explicitly and
     * manually by calling this function.
     *
     * @note This is NOT an asynchronous API and the rendering thread will be blocked
     *       when it attempts to manipulate the critical resources list.
     */
    public static CollectCriticalSharedResources(): void;
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
 *         Prototype: () -> void
 * 
 * @signal [monitor-added] Emitted when a new monitor was plugged into the system.
 *                         Note that the monitors that have existed in the system when the
 *                         display object is created will not cause this signal to be emitted.
 *         Prototype: (monitor: Monitor) -> void
 * 
 * @signal [monitor-removed] Emitted when an existing monitor was removed.
 *                           The corresponding `Monitor` object will also be notified by `detached` signal.
 *         Prototype: (monitor: Monitor) -> void
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

    /**
     * Get a list of monitors connected to the system.
     */
    public requestMonitorList(): Promise<Array<Monitor>>;

    readonly defaultCursorTheme: CursorTheme;

    public loadCursorTheme(name: string, size: number): Promise<CursorTheme>;
    public createCursor(bitmap: CkBitmap, hotspotX: number, hotspotY: number): Promise<Cursor>;
}

export class CursorTheme extends RenderClientObject {
    public dispose(): Promise<void>;
    public loadCursorFromName(name: string): Promise<Cursor>;
}

export class Cursor extends RenderClientObject {
    public dispose(): Promise<void>;
    public getHotspotVector(): Promise<{x: number, y: number}>;
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

/**
 * Monitor connected to the system.
 * 
 * @signal [properties-changed] Notify that the properties of the Monitor has changed.
 *                              New properties are provided as an argument of the signal.
 *                              It also can be triggered by call `requestPropertySet` manually.
 *         Prototype: (properties: MonitorPropertySet) -> void
 * 
 * @signal [detached] Monitor has been detached from the `Display` object,
 *                    which means the corresponding physical monitor has been removed from
 *                    the system or disabled.
 *         Prototype: () -> void
 */
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

    /**
     * Get a set of monitor's properties.
     * Promise returned by the function will be resolved when the rendering thread
     * accepts the request. Monitor properties will be sent with signal `properties-changed`
     * later.
     */
    public requestPropertySet(): Promise<void>;
}

/**
 * An abstraction of a toplevel visual window where the contents can be rendered.
 * As `Surface` object itself only provides methods to operate the window,
 * a `Blender` object, which provides methods for rendering and frame scheduling,
 * should be associated with the `Surface` object.
 * 
 * @signal [closed] Emitted when the window has been closed.
 *                  Prototype: () -> void
 * 
 * @signal [resize] Emitted when the window has been resized.
 *                  Prototype: (width: number, height: number) -> void
 * 
 * @signal [configure] Emitted when the Window Manager notified us that the window
 *                     should be reconfigured (resize, move, etc.)
 *                  Prototype: (width: number, height: number, state: ToplevelStates) -> void
 * 
 * @signal [frame] Emitted when it is a good time to start submitting a new frame.
 *                 This signal is related to the VSync mechanism where the system will
 *                 notify us when the monitor will refresh for next frame.
 *                 There are two ways to schedule this signal: call `Surface.requestNextFrame`
 *                 explicitly or call `Blender.update`, which also calls `requestNextFrame` implicitly.
 *                 Prototype: () -> void
 * 
 * @signal [pointer-hovering] Emitted when the pointer device enters or leaves the window area.
 *                            Prototype: (enter: boolean) -> void
 * 
 * @signal [pointer-motion] Emitted when a pointer moves on the window.
 *                          Prototype: (double dx, double dy) -> void
 * 
 * @signal [pointer-button] Emitted when a button of pointer device is pressed or released.
 *                          Prototype: (button: PointerButton, pressed: boolean) -> void
 */
export class Surface extends RenderClientObject {
    // ToplevelStates
    static readonly TOPLEVEL_MAXIMIZED: number;
    static readonly TOPLEVEL_FULLSCREEN: number;
    static readonly TOPLEVEL_RESIZING: number;
    static readonly TOPLEVEL_ACTIVATED: number;
    static readonly TOPLEVEL_TILED_LEFT: number;
    static readonly TOPLEVEL_TILED_RIGHT: number;
    static readonly TOPLEVEL_TILED_TOP: number;
    static readonly TOPLEVEL_TILED_BOTTOM: number;

    // PointerButton
    static readonly POINTER_BUTTON_LEFT: number;
    static readonly POINTER_BUTTON_RIGHT: number;
    static readonly POINTER_BUTTON_MIDDLE: number;
    static readonly POINTER_BUTTON_SIDE: number;
    static readonly POINTER_BUTTON_EXTRA: number;
    static readonly POINTER_BUTTON_FORWARD: number;
    static readonly POINTER_BUTTON_BACK: number;
    static readonly POINTER_BUTTON_TASK: number;

    /* Window width in pixels. */
    public readonly width: number;
    /* Window height in pixels. */
    public readonly height: number;

    /**
     * Create a `Blender` object and make it associated with the surface.
     * If the surface has an associated `Blender` object, `Blender.dispose`
     * must be called before disposing the surface.
     * 
     * Note that the surface only can associate a single `Blender` object,
     * and the `Blender` object also only can be associated with a unique `Surface`.
     * If the surface already has a `Blender`, this operation will fail and the
     * promise will be rejected.
     */
    public createBlender(): Promise<Blender>;

    /**
     * Close the window immediately.
     * `close` signal does not indicate that a window is closed, which just means user
     * has clicked the "close" button on the window decoration.
     * Only by calling `close` method can we actually close a window.
     * 
     * `closed` single will be emitted when the window is actually closed.
     */
    public close(): Promise<void>;

    /**
     * Get a string representation about available framebuffers.
     * There is no any guarantee that the returned string has a specific format.
     * The format is highly implementation-defined.
     */
    public getBufferDescriptor(): Promise<string>;

    /**
     * Schedule next frame to keep VSync.
     * The resolution of Promise only means the rendering thread has accepted
     * the request. A `frame` signal will be emitted when it is a good time to display
     * next frame.
     */
    public requestNextFrame(): Promise<void>;

    /**
     * Set a title for the window.
     */
    public setTitle(title: string): Promise<void>;

    /**
     * Resize the window.
     * The resolution of Promise only means the rendering thread has accepted
     * the request. A `resize` signal will be emitted when the resizing is finished.
     */
    public resize(width: number, height: number): Promise<void>;

    /**
     * Set a maximum geometry size of the window in pixels.
     */
    public setMinSize(width: number, height: number): Promise<void>;

    /**
     * Set a minimum geometry size of the window in pixels.
     */
    public setMaxSize(width: number, height: number): Promise<void>;

    /**
     * Maximize or unmaximize the window if the system Window Manager supports
     * the operation.
     */
    public setMaximized(value: boolean): Promise<void>;

    /**
     * Minimize or unminimize the window if the system Window Manager supports
     * the operation.
     */
    public setMinimized(value: boolean): Promise<void>;

    /**
     * Make the window enter or leave fullscreen state.
     *
     * @param value     true to enter the fullscreen state; otherwise, leave the fullscreen state.
     * @param monitor   A monitor where the fullscreen window should be displayed.
     */
    public setFullscreen(value: boolean, monitor: Monitor): Promise<void>;
}

/**
 * The result of profiling which contains timing measurements for
 * several recent frames (we call them "samples").
 * A maximum number of samples can be specified by commandline option
 * `--gl-profiler-ringbuffer-threshold`, otherwise, a default value
 * will be used. Typically, about 30-60 samples can be carried in a single
 * profiling report.
 * This profiling report is generated by `GProfiler` object.
 */
export interface GProfilerReport {
    // Profiling timebase in microseconds, a reference time point
    // for all the timing measurements in this report.
    timebaseUsSinceEpoch: number;

    // Profiling entries. Each entry is a single frame.
    entries: Array<{

        // Frame serial number counted by Blender.
        frame: number;

        // Timing measurements. See related documentations to know
        // more information about each milestone.
        milestones: Array<{
            requested: number;
            presented: number;
            prerollBegin: number;
            prerollEnd: number;
            paintBegin: number;
            paintEnd: number;
            begin: number;
            end: number;
        }>;
    }>;
}

/**
 * Graphics profiler.
 * An instance of `GProfiler` always associates with a unique `Blender`
 * instance (if it is enabled by commandline option `--gl-enable-profiler`),
 * collecting the profiling information and manage them.
 */
export class GProfiler {
    /**
     * Generate a profiling report (export profiling results).
     * A report contains profiling information of several
     * the most recent frames. See `GProfilerReport` for more details.
     *
     * @note It is NOT an asynchronous API and will lock the rendering thread
     *       if the rendering thread attempts to insert a new sample.
     */
    public generateCurrentReport(): GProfilerReport;

    /**
     * Purge all the history samples stored in the ring buffer
     * to relieve memory pressure (if `freeMemory` is True).
     *
     * @param freeMemory Whether or not release the allocated memory after
     *                   purging the samples. If not, memory of purged samples
     *                   can be reused for new samples in the future to avoid
     *                   frequent memory allocations.
     *
     * @note It is NOT an asynchronous API and will block the rendering thread
     *       if the rendering thread attempts to insert a new sample.
     */
    public purgeRecentHistorySamples(freeMemory: boolean): void;
}

type TextureId = number;

/**
 * Blender, a content aggregator in Glamor rendering framework, mainly manages
 * the textures and performs the rasterization work of layer trees.
 * It controls the presentation process of contents on a window, providing a higher
 * abstraction of `Surface`. While `Surface` itself provides an interface by which
 * user can manipulate the behaviors and appearances of windows, `Blender` provides
 * an interface by which user can render frames on a window.
 *
 * A `Blender` instance always associates with a unique `Surface` instance
 * implicitly. See `Surface.createBlender` for more details about
 * creating a Blender.
 *
 * @signal [picture-captured] A captured Picture of current frame has been delivered
 *                            from rendering thread. The serial number corresponds with the
 *                            number returned by `captureNextFrameAsPicture`.
 *                            Prototype: (pict: CriticalPicture, serial: number) -> void
 *
 * @signal [<dynamic: texture deletion>] A texture has been deleted. Name of the signal
 *                                       is specified by user.
 *                                       Prototype: () -> void
 */
export class Blender extends RenderClientObject {
    public readonly profiler: GProfiler | null;

    public dispose(): Promise<void>;
    public update(scene: Scene): Promise<void>;

    public createTextureFromImage(image: CkImage,
                                  annotation: string): Promise<TextureId>;

    public createTextureFromEncodedData(data: Buffer,
                                        alphaType: number,
                                        annotation: string): Promise<TextureId>;

    public createTextureFromPixmap(pixels: Buffer,
                                   width: number,
                                   height: number,
                                   colorType: number,
                                   alphaType: number,
                                   annotation: string): Promise<TextureId>;

    public deleteTexture(textureId: TextureId): Promise<void>;

    public newTextureDeletionSubscriptionSignal(textureId: TextureId,
                                                sigName: string): Promise<void>;

    public purgeRasterCacheResources(): Promise<void>;

    /**
     * Record the rasterization process of current frame (current layer tree)
     * as a Picture. Caller will be notified by `picture-captured` signal when
     * the Picture is generated. All the required drawing resources are interned
     * in the captured Picture (including textures, images, typefaces, etc.).
     *
     * @return A serial number of capture increasing with frame counter.
     *         If the function is called for multiple times before calling `update`,
     *         it will return the same serial number.
     */
    public captureNextFrameAsPicture(): Promise<number>;
}

export class Scene {
    /**
     * A `Scene` instance MUST be disposed as quickly as possible
     * if it will not be used anymore.
     */
    public dispose(): void;

    // ~Experimental API~
    public toImage(width: number, height: number): Promise<CkImage>;

    /**
     * Get an S-Expression representation of the layer tree.
     * Related tools provided by Cocoa Project can be used to perform
     * further analysis for the returned string.
     */
    public toString(): string;
}

export class SceneBuilder {
    public constructor(viewportWidth: number, viewportHeight: number);

    public build(): Scene;

    public pop(): SceneBuilder;

    public addPicture(picture: CkPicture,
                      autoFastClipping: boolean,
                      dx: number, dy: number): SceneBuilder;

    public addTexture(textureId: TextureId,
                      offsetX: number,
                      offsetY: number,
                      width: number,
                      height: number,
                      sampling: number): SceneBuilder;

    public pushOffset(offsetX: number, offsetY: number): SceneBuilder;

    public pushImageFilter(filter: CkImageFilter): SceneBuilder;

    public pushRectClip(shape: CkRect, AA: boolean): SceneBuilder;

    public pushRRectClip(shape: CkRRect, AA: boolean): SceneBuilder;

    public pushBackdropFilter(filter: CkImageFilter,
                              blendMode: number,
                              autoChildClipping: boolean): SceneBuilder;
}

// ===============================
// Basic Rendering Objects
// ===============================

interface Constants {
    readonly CAPABILITY_HWCOMPOSE_ENABLED: number;
    readonly CAPABILITY_PROFILER_ENABLED: number;
    readonly CAPABILITY_PROFILER_MAX_SAMPLES: number;
    readonly CAPABILITY_MESSAGE_QUEUE_PROFILING_ENABLED: number;

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

    readonly SAMPLING_FILTER_NEAREST: number;
    readonly SAMPLING_FILTER_LINEAR: number;
    readonly SAMPLING_CUBIC_MITCHELL: number;
    readonly SAMPLING_CUBIC_CATMULL_ROM: number;

    readonly TILE_MODE_CLAMP: number;
    readonly TILE_MODE_REPEAT: number;
    readonly TILE_MODE_MIRROR: number;
    readonly TILE_MODE_DECAL: number;

    readonly BLEND_MODE_CLEAR: number;
    readonly BLEND_MODE_SRC: number;
    readonly BLEND_MODE_DST: number;
    readonly BLEND_MODE_SRC_OVER: number;
    readonly BLEND_MODE_DST_OVER: number;
    readonly BLEND_MODE_SRC_IN: number;
    readonly BLEND_MODE_DST_IN: number;
    readonly BLEND_MODE_SRC_OUT: number;
    readonly BLEND_MODE_DST_OUT: number;
    readonly BLEND_MODE_SRC_ATOP: number;
    readonly BLEND_MODE_DST_ATOP: number;
    readonly BLEND_MODE_XOR: number;
    readonly BLEND_MODE_PLUS: number;
    readonly BLEND_MODE_MODULATE: number;
    readonly BLEND_MODE_SCREEN: number;
    readonly BLEND_MODE_OVERLAY: number;
    readonly BLEND_MODE_DARKEN: number;
    readonly BLEND_MODE_LIGHTEN: number;
    readonly BLEND_MODE_COLOR_DODGE: number;
    readonly BLEND_MODE_COLOR_BURN: number;
    readonly BLEND_MODE_HARD_LIGHT: number;
    readonly BLEND_MODE_SOFT_LIGHT: number;
    readonly BLEND_MODE_DIFFERENCE: number;
    readonly BLEND_MODE_EXCLUSION: number;
    readonly BLEND_MODE_HUE: number;
    readonly BLEND_MODE_SATURATION: number;
    readonly BLEND_MODE_COLOR: number;
    readonly BLEND_MODE_LUMINOSITY: number;
}

export declare let Constants: Constants;

export class CkImageFilter {
    public static MakeFromDSL(dsl: string, kwargs: object): CkImageFilter;
    public static Deserialize(buffer: Buffer): CkImageFilter;

    public serialize(): Buffer;
}

export class CkColorFilter {
    public static MakeFromDSL(dsl: string, kwargs: object): CkColorFilter;
    public static Deserialize(buffer: Buffer): CkColorFilter;

    public serialize(): Buffer;
}

export class CkBitmap {
    public static MakeFromBuffer(buffer: Buffer,
                                 width: number,
                                 height: number,
                                 colorType: number,
                                 alphaType: number): CkBitmap;
    
    public static MakeFromEncodedFile(path: string): CkBitmap;

    readonly width: number;
    readonly height: number;
    readonly alphaType: number;
    readonly colorType: number;
    readonly bytesPerPixel: number;
    readonly rowBytesAsPixels: number;
    readonly shiftPerPixel: number;
    readonly rowBytes: number;
    
    public computeByteSize(): number;
    public toImage(): CkImage;
    public getPixelBuffer(): Buffer;
}

export class CkImage {
    public static MakeFromEncodedData(buffer: Buffer): Promise<CkImage>;
    public static MakeFromEncodedFile(path: string): Promise<CkImage>;

    readonly width: number;
    readonly height: number;
    readonly alphaType: number;
    readonly colorType: number;

    public uniqueId(): number;
    public encodeToData(format: number, quality: number): Buffer;
}

export class CkPicture {
    static readonly USAGE_GENERIC: number;
    static readonly USAGE_TRANSFER: number;

    public static MakeFromData(buffer: Buffer, usage: number): CkPicture;
    public static MakeFromFile(path: string, usage: number): CkPicture;

    public serialize(): Buffer;
    public approximateOpCount(nested: boolean): number;
    public approximateByteUsed(): number;
    public uniqueId(): number;
}
