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

import {Buffer} from 'synthetic://core';
import {VideoBuffer} from 'synthetic://utau';

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

// [x, y]
export type CkPoint = Array<number>;

// [x, y, z]
export type CkPoint3 = Array<number>;

// [R, G, B, A] where R,G,B,A∈[0,1]
export type CkColor4f = Array<number>;

export interface CkImageInfo {
    alphaType: AlphaType;
    colorType: ColorType;
    colorSpace: ColorSpace;
    width: number;
    height: number;
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
export function queryCapabilities(cap: Capability): boolean | number | string;

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
     * If it is the first HWCompose surface that the caller creates, it will lead to
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
 *                          Prototype: (double x, double y) -> void
 * 
 * @signal [pointer-button] Emitted when a button of pointer device is pressed or released.
 *                          Prototype: (button: PointerButton, pressed: boolean) -> void
 *
 * @signal [pointer-axis] <Undocumented>
 *                        Prototype: (sourceType: PointerAxisSource, dx: number, dy: number) -> void
 *
 *
 * @signal [pointer-highres-scroll] <Undocumented>
 *                                  Prototype: (sourceType: PointerAxisSource, dx: number, dy: number) -> void
 *
 * @signal [keyboard-focus] <Undocumented>
 *                          Prototype: (focused: boolean) -> void
 *
 * @signal [keyboard-key] <Undocumented>
 *                        Prototype: (key: KeyboardKey, modifiers: KeyboardModifiers, pressed: boolean) -> void
 */

export type TopLevelStates = number;
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
        milestones: {
            requested: number;
            presented: number;
            prerollBegin: number;
            prerollEnd: number;
            paintBegin: number;
            paintEnd: number;
            begin: number;
            end: number;
        };
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
                                        alphaType: AlphaType,
                                        annotation: string): Promise<TextureId>;

    public createTextureFromPixmap(pixels: Buffer,
                                   width: number,
                                   height: number,
                                   colorType: ColorType,
                                   alphaType: AlphaType,
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

    public addVideoBuffer(vbo: VideoBuffer,
                          offsetX: number,
                          offsetY: number,
                          width: number,
                          height: number,
                          sampling: number): SceneBuilder;

    public pushOffset(offsetX: number, offsetY: number): SceneBuilder;

    public pushOpacity(alpha: number): SceneBuilder;

    public pushRotate(rad: number, pivotX: number, pivotY: number): SceneBuilder;

    public pushImageFilter(filter: CkImageFilter): SceneBuilder;

    public pushRectClip(shape: CkRect, AA: boolean): SceneBuilder;

    public pushRRectClip(shape: CkRRect, AA: boolean): SceneBuilder;

    public pushBackdropFilter(filter: CkImageFilter,
                              blendMode: BlendMode,
                              autoChildClipping: boolean): SceneBuilder;
}

export type Capability = number;
export type PointerButton = number;
export type PointerAxisSource = number;
export type KeyboardModifiers = number;
export type KeyboardKey = number;
export type ColorType = number;
export type AlphaType = number;
export type CodecFormat = number;
export type SamplingOption = number;
export type TileMode = number;
export type BlendMode = number;
export type ColorSpace = number;
export type PaintStyle = number;
export type PaintCap = number;
export type PaintJoin = number;
export type PathFillType = number;
export type PathDirection = number;
export type PathArcSize = number;
export type PathAddPathMode = number;
export type ApplyPerspectiveClip = number;
export type CanvasSaveLayerFlag = number;
export type CanvasPointMode = number;
export type CanvasSrcRectConstraint = number;
export type ClipOp = number;
export type FontStyleWeight = number;
export type FontStyleWidth = number;
export type FontStyleSlant = number;
export type MatrixScaleToFit = number;
export type FontEdging = number;
export type FontHinting = number;
export type TextEncoding = number;
export type PathEffectPath1DStyle = number;
export type PathEffectTrim = number;

interface Constants {
    readonly CAPABILITY_HWCOMPOSE_ENABLED: Capability;
    readonly CAPABILITY_PROFILER_ENABLED: Capability;
    readonly CAPABILITY_PROFILER_MAX_SAMPLES: Capability;
    readonly CAPABILITY_MESSAGE_QUEUE_PROFILING_ENABLED: Capability;

    readonly COLOR_TYPE_ALPHA8: ColorType;
    readonly COLOR_TYPE_RGB565: ColorType;
    readonly COLOR_TYPE_ARGB4444: ColorType;
    readonly COLOR_TYPE_RGBA8888: ColorType;
    readonly COLOR_TYPE_RGB888x: ColorType;
    readonly COLOR_TYPE_BGRA8888: ColorType;
    readonly COLOR_TYPE_BGRA1010102: ColorType;
    readonly COLOR_TYPE_RGBA1010102: ColorType;
    readonly COLOR_TYPE_RGB101010x: ColorType;
    readonly COLOR_TYPE_BGR101010x: ColorType;
    readonly COLOR_TYPE_GRAY8: ColorType;
    readonly COLOR_TYPE_RGBA_F16_NORM: ColorType;
    readonly COLOR_TYPE_RGBA_F16: ColorType;
    readonly COLOR_TYPE_RGBA_F32: ColorType;
    readonly COLOR_TYPE_R8G8_UNORM: ColorType;
    readonly COLOR_TYPE_A16_FLOAT: ColorType;
    readonly COLOR_TYPE_R16G16_FLOAT: ColorType;
    readonly COLOR_TYPE_A16_UNORM: ColorType;
    readonly COLOR_TYPE_R16G16_UNORM: ColorType;
    readonly COLOR_TYPE_R16G16B16A16_UNORM: ColorType;

    readonly ALPHA_TYPE_PREMULTIPLIED: AlphaType;
    readonly ALPHA_TYPE_UNPREMULTIPLIED: AlphaType;
    readonly ALPHA_TYPE_OPAQUE: AlphaType;

    readonly COLOR_SPACE_SRGB: ColorSpace;

    readonly PAINT_STYLE_FILL: PaintStyle;
    readonly PAINT_STYLE_STROKE: PaintStyle;
    readonly PAINT_STYLE_STROKE_FILL: PaintStyle;
    readonly PAINT_CAP_BUTT: PaintCap;
    readonly PAINT_CAP_ROUND: PaintCap;
    readonly PAINT_CAP_SQUARE: PaintCap;
    readonly PAINT_JOIN_MITER: PaintJoin;
    readonly PAINT_JOIN_ROUND: PaintJoin;
    readonly PAINT_JOIN_BEVEL: PaintJoin;
    readonly PATH_FILL_TYPE_WINDING: PathFillType;
    readonly PATH_FILL_TYPE_EVEN_ODD: PathFillType;
    readonly PATH_FILL_TYPE_INVERSE_WINDING: PathFillType;
    readonly PATH_FILL_TYPE_INVERSE_EVEN_ODD: PathFillType;
    readonly PATH_DIRECTION_CW: PathDirection;
    readonly PATH_DIRECTION_CCW: PathDirection;
    readonly PATH_ARC_SIZE_SMALL: PathArcSize;
    readonly PATH_ARC_SIZE_LARGE: PathArcSize;
    readonly PATH_ADD_PATH_MODE_APPEND: PathAddPathMode;
    readonly PATH_ADD_PATH_MODE_EXTEND: PathAddPathMode;

    readonly APPLY_PERSPECTIVE_CLIP_YES: ApplyPerspectiveClip;
    readonly APPLY_PERSPECTIVE_CLIP_NO: ApplyPerspectiveClip;

    readonly MATRIX_SCALE_TO_FIT_FILL: MatrixScaleToFit;
    readonly MATRIX_SCALE_TO_FIT_START: MatrixScaleToFit;
    readonly MATRIX_SCALE_TO_FIT_CENTER: MatrixScaleToFit;
    readonly MATRIX_SCALE_TO_FIT_END: MatrixScaleToFit;

    readonly CANVAS_SAVE_LAYER_PRESERVE_LCD_TEXT: CanvasSaveLayerFlag;
    readonly CANVAS_SAVE_LAYER_INIT_WITH_PREVIOUS: CanvasSaveLayerFlag;
    readonly CANVAS_SAVE_LAYER_F16_COLOR_TYPE: CanvasSaveLayerFlag;
    readonly CANVAS_POINT_MODE_POINTS: CanvasPointMode;
    readonly CANVAS_POINT_MODE_LINES: CanvasPointMode;
    readonly CANVAS_POINT_MODE_POLYGON: CanvasPointMode;
    readonly CANVAS_SRC_RECT_CONSTRAINT_STRICT: CanvasSrcRectConstraint;
    readonly CANVAS_SRC_RECT_CONSTRAINT_FAST: CanvasSrcRectConstraint;

    readonly CLIP_OP_DIFFERENCE: ClipOp;
    readonly CLIP_OP_INTERSECT: ClipOp;

    readonly FONT_STYLE_WEIGHT_INVISIBLE: FontStyleWeight;
    readonly FONT_STYLE_WEIGHT_THIN: FontStyleWeight;
    readonly FONT_STYLE_WEIGHT_EXTRA_LIGHT: FontStyleWeight;
    readonly FONT_STYLE_WEIGHT_LIGHT: FontStyleWeight;
    readonly FONT_STYLE_WEIGHT_NORMAL: FontStyleWeight;
    readonly FONT_STYLE_WEIGHT_MEDIUM: FontStyleWeight;
    readonly FONT_STYLE_WEIGHT_SEMI_BOLD: FontStyleWeight;
    readonly FONT_STYLE_WEIGHT_BOLD: FontStyleWeight;
    readonly FONT_STYLE_WEIGHT_EXTRA_BOLD: FontStyleWeight;
    readonly FONT_STYLE_WEIGHT_BLACK: FontStyleWeight;
    readonly FONT_STYLE_WEIGHT_EXTRA_BLACK: FontStyleWeight;
    readonly FONT_STYLE_WIDTH_ULTRA_CONDENSED: FontStyleWidth;
    readonly FONT_STYLE_WIDTH_EXTRA_CONDENSED: FontStyleWidth;
    readonly FONT_STYLE_WIDTH_CONDENSED: FontStyleWidth;
    readonly FONT_STYLE_WIDTH_SEMI_CONDENSED: FontStyleWidth;
    readonly FONT_STYLE_WIDTH_NORMAL: FontStyleWidth;
    readonly FONT_STYLE_WIDTH_SEMI_EXPANDED: FontStyleWidth;
    readonly FONT_STYLE_WIDTH_EXPANDED: FontStyleWidth;
    readonly FONT_STYLE_WIDTH_EXTRA_EXPANDED: FontStyleWidth;
    readonly FONT_STYLE_WIDTH_ULTRA_EXPANDED: FontStyleWidth;
    readonly FONT_STYLE_SLANT_UPRIGHT: FontStyleSlant;
    readonly FONT_STYLE_SLANT_ITALIC: FontStyleSlant;
    readonly FONT_STYLE_SLANT_OBLIQUE: FontStyleSlant;

    readonly FONT_EDGING_ALIAS: FontEdging;
    readonly FONT_EDGING_ANTIALIAS: FontEdging;
    readonly FONT_EDGING_SUBPIXEL_ANTIALIAS: FontEdging;

    readonly FONT_HINTING_NONE: FontHinting;
    readonly FONT_HINTING_SLIGHT: FontHinting;
    readonly FONT_HINTING_NORMAL: FontHinting;
    readonly FONT_HINTING_FULL: FontHinting;

    readonly TEXT_ENCODING_UTF8: TextEncoding;
    readonly TEXT_ENCODING_UTF16: TextEncoding;
    readonly TEXT_ENCODING_UTF32: TextEncoding;

    readonly PATH_EFFECT_PATH1D_STYLE_TRANSLATE: PathEffectPath1DStyle;
    readonly PATH_EFFECT_PATH1D_STYLE_ROTATE: PathEffectPath1DStyle;
    readonly PATH_EFFECT_PATH1D_STYLE_MORPH: PathEffectPath1DStyle;
    readonly PATH_EFFECT_TRIM_NORMAL: PathEffectTrim;
    readonly PATH_EFFECT_TRIM_INVERTED: PathEffectTrim;

    readonly FORMAT_PNG: CodecFormat;
    readonly FORMAT_JPEG: CodecFormat;
    readonly FORMAT_WEBP: CodecFormat;

    readonly SAMPLING_FILTER_NEAREST: SamplingOption;
    readonly SAMPLING_FILTER_LINEAR: SamplingOption;
    readonly SAMPLING_CUBIC_MITCHELL: SamplingOption;
    readonly SAMPLING_CUBIC_CATMULL_ROM: SamplingOption;

    readonly TILE_MODE_CLAMP: TileMode;
    readonly TILE_MODE_REPEAT: TileMode;
    readonly TILE_MODE_MIRROR: TileMode;
    readonly TILE_MODE_DECAL: TileMode;

    readonly BLEND_MODE_CLEAR: BlendMode;
    readonly BLEND_MODE_SRC: BlendMode;
    readonly BLEND_MODE_DST: BlendMode;
    readonly BLEND_MODE_SRC_OVER: BlendMode;
    readonly BLEND_MODE_DST_OVER: BlendMode;
    readonly BLEND_MODE_SRC_IN: BlendMode;
    readonly BLEND_MODE_DST_IN: BlendMode;
    readonly BLEND_MODE_SRC_OUT: BlendMode;
    readonly BLEND_MODE_DST_OUT: BlendMode;
    readonly BLEND_MODE_SRC_ATOP: BlendMode;
    readonly BLEND_MODE_DST_ATOP: BlendMode;
    readonly BLEND_MODE_XOR: BlendMode;
    readonly BLEND_MODE_PLUS: BlendMode;
    readonly BLEND_MODE_MODULATE: BlendMode;
    readonly BLEND_MODE_SCREEN: BlendMode;
    readonly BLEND_MODE_OVERLAY: BlendMode;
    readonly BLEND_MODE_DARKEN: BlendMode;
    readonly BLEND_MODE_LIGHTEN: BlendMode;
    readonly BLEND_MODE_COLOR_DODGE: BlendMode;
    readonly BLEND_MODE_COLOR_BURN: BlendMode;
    readonly BLEND_MODE_HARD_LIGHT: BlendMode;
    readonly BLEND_MODE_SOFT_LIGHT: BlendMode;
    readonly BLEND_MODE_DIFFERENCE: BlendMode;
    readonly BLEND_MODE_EXCLUSION: BlendMode;
    readonly BLEND_MODE_HUE: BlendMode;
    readonly BLEND_MODE_SATURATION: BlendMode;
    readonly BLEND_MODE_COLOR: BlendMode;
    readonly BLEND_MODE_LUMINOSITY: BlendMode;

    /* Pointer buttons (mouse and other pointing devices) */
    readonly POINTER_BUTTON_LEFT: PointerButton;
    readonly POINTER_BUTTON_RIGHT: PointerButton;
    readonly POINTER_BUTTON_MIDDLE: PointerButton;
    readonly POINTER_BUTTON_SIDE: PointerButton;
    readonly POINTER_BUTTON_EXTRA: PointerButton;
    readonly POINTER_BUTTON_FORWARD: PointerButton;
    readonly POINTER_BUTTON_BACK: PointerButton;
    readonly POINTER_BUTTON_TASK: PointerButton;

    /* Pointer axis (scrolling) */
    readonly POINTER_AXIS_SOURCE_WHEEL: PointerAxisSource;
    readonly POINTER_AXIS_SOURCE_WHEEL_TILT: PointerAxisSource;
    readonly POINTER_AXIS_SOURCE_FINGER: PointerAxisSource;
    readonly POINTER_AXIS_SOURCE_CONTINUOUS: PointerAxisSource;
    readonly POINTER_AXIS_SOURCE_UNKNOWN: PointerAxisSource;

    /* Keyboard modifiers (bitfields) */
    readonly MODIFIER_CONTROL: KeyboardModifiers;
    readonly MODIFIER_ALT: KeyboardModifiers;
    readonly MODIFIER_SHIFT: KeyboardModifiers;
    readonly MODIFIER_SUPER: KeyboardModifiers;
    readonly MODIFIER_CAPS_LOCK: KeyboardModifiers;
    readonly MODIFIER_NUM_LOCK: KeyboardModifiers;

    /* Keyboard keys */
    readonly KEY_SPACE: KeyboardKey;
    readonly KEY_APOSTROPHE: KeyboardKey;
    readonly KEY_COMMA: KeyboardKey;
    readonly KEY_MINUS: KeyboardKey;
    readonly KEY_PERIOD: KeyboardKey;
    readonly KEY_SLASH: KeyboardKey;
    readonly KEY_0: KeyboardKey;
    readonly KEY_1: KeyboardKey;
    readonly KEY_2: KeyboardKey;
    readonly KEY_3: KeyboardKey;
    readonly KEY_4: KeyboardKey;
    readonly KEY_5: KeyboardKey;
    readonly KEY_6: KeyboardKey;
    readonly KEY_7: KeyboardKey;
    readonly KEY_8: KeyboardKey;
    readonly KEY_9: KeyboardKey;
    readonly KEY_SEMICOLON: KeyboardKey;
    readonly KEY_EQUAL: KeyboardKey;
    readonly KEY_A: KeyboardKey;
    readonly KEY_B: KeyboardKey;
    readonly KEY_C: KeyboardKey;
    readonly KEY_D: KeyboardKey;
    readonly KEY_E: KeyboardKey;
    readonly KEY_F: KeyboardKey;
    readonly KEY_G: KeyboardKey;
    readonly KEY_H: KeyboardKey;
    readonly KEY_I: KeyboardKey;
    readonly KEY_J: KeyboardKey;
    readonly KEY_K: KeyboardKey;
    readonly KEY_L: KeyboardKey;
    readonly KEY_M: KeyboardKey;
    readonly KEY_N: KeyboardKey;
    readonly KEY_O: KeyboardKey;
    readonly KEY_P: KeyboardKey;
    readonly KEY_Q: KeyboardKey;
    readonly KEY_R: KeyboardKey;
    readonly KEY_S: KeyboardKey;
    readonly KEY_T: KeyboardKey;
    readonly KEY_U: KeyboardKey;
    readonly KEY_V: KeyboardKey;
    readonly KEY_W: KeyboardKey;
    readonly KEY_X: KeyboardKey;
    readonly KEY_Y: KeyboardKey;
    readonly KEY_Z: KeyboardKey;
    readonly KEY_LEFT_BRACKET: KeyboardKey;
    readonly KEY_BACKSLASH: KeyboardKey;
    readonly KEY_RIGHT_BRACKET: KeyboardKey;
    readonly KEY_GRAVE_ACCENT: KeyboardKey;
    readonly KEY_WORLD_1: KeyboardKey;
    readonly KEY_WORLD_2: KeyboardKey;
    readonly KEY_ESCAPE: KeyboardKey;
    readonly KEY_ENTER: KeyboardKey;
    readonly KEY_TAB: KeyboardKey;
    readonly KEY_BACKSPACE: KeyboardKey;
    readonly KEY_INSERT: KeyboardKey;
    readonly KEY_DELETE: KeyboardKey;
    readonly KEY_RIGHT: KeyboardKey;
    readonly KEY_LEFT: KeyboardKey;
    readonly KEY_DOWN: KeyboardKey;
    readonly KEY_UP: KeyboardKey;
    readonly KEY_PAGE_UP: KeyboardKey;
    readonly KEY_PAGE_DOWN: KeyboardKey;
    readonly KEY_HOME: KeyboardKey;
    readonly KEY_END: KeyboardKey;
    readonly KEY_CAPS_LOCK: KeyboardKey;
    readonly KEY_SCROLL_LOCK: KeyboardKey;
    readonly KEY_NUM_LOCK: KeyboardKey;
    readonly KEY_PRINT_SCREEN: KeyboardKey;
    readonly KEY_PAUSE: KeyboardKey;
    readonly KEY_F1: KeyboardKey;
    readonly KEY_F2: KeyboardKey;
    readonly KEY_F3: KeyboardKey;
    readonly KEY_F4: KeyboardKey;
    readonly KEY_F5: KeyboardKey;
    readonly KEY_F6: KeyboardKey;
    readonly KEY_F7: KeyboardKey;
    readonly KEY_F8: KeyboardKey;
    readonly KEY_F9: KeyboardKey;
    readonly KEY_F10: KeyboardKey;
    readonly KEY_F11: KeyboardKey;
    readonly KEY_F12: KeyboardKey;
    readonly KEY_F13: KeyboardKey;
    readonly KEY_F14: KeyboardKey;
    readonly KEY_F15: KeyboardKey;
    readonly KEY_F16: KeyboardKey;
    readonly KEY_F17: KeyboardKey;
    readonly KEY_F18: KeyboardKey;
    readonly KEY_F19: KeyboardKey;
    readonly KEY_F20: KeyboardKey;
    readonly KEY_F21: KeyboardKey;
    readonly KEY_F22: KeyboardKey;
    readonly KEY_F23: KeyboardKey;
    readonly KEY_F24: KeyboardKey;
    readonly KEY_F25: KeyboardKey;
    readonly KEY_KP_0: KeyboardKey;
    readonly KEY_KP_1: KeyboardKey;
    readonly KEY_KP_2: KeyboardKey;
    readonly KEY_KP_3: KeyboardKey;
    readonly KEY_KP_4: KeyboardKey;
    readonly KEY_KP_5: KeyboardKey;
    readonly KEY_KP_6: KeyboardKey;
    readonly KEY_KP_7: KeyboardKey;
    readonly KEY_KP_8: KeyboardKey;
    readonly KEY_KP_9: KeyboardKey;
    readonly KEY_KP_DECIMAL: KeyboardKey;
    readonly KEY_KP_DIVIDE: KeyboardKey;
    readonly KEY_KP_MULTIPLY: KeyboardKey;
    readonly KEY_KP_SUBTRACT: KeyboardKey;
    readonly KEY_KP_ADD: KeyboardKey;
    readonly KEY_KP_ENTER: KeyboardKey;
    readonly KEY_KP_EQUAL: KeyboardKey;
    readonly KEY_LEFT_SHIFT: KeyboardKey;
    readonly KEY_LEFT_CONTROL: KeyboardKey;
    readonly KEY_LEFT_ALT: KeyboardKey;
    readonly KEY_LEFT_SUPER: KeyboardKey;
    readonly KEY_RIGHT_SHIFT: KeyboardKey;
    readonly KEY_RIGHT_CONTROL: KeyboardKey;
    readonly KEY_RIGHT_ALT: KeyboardKey;
    readonly KEY_RIGHT_SUPER: KeyboardKey;
    readonly KEY_MENU: KeyboardKey;
}

export declare let Constants: Constants;

export class CkSurface {
    private constructor();
    public static MakeRaster(imageInfo: CkImageInfo): CkSurface;
    public static MakeNull(width: number, height: number): CkSurface;

    readonly width: number;
    readonly height: number;
    readonly generationID: number;
    readonly imageInfo: CkImageInfo;

    public getCanvas(): CkCanvas;
    public makeSurface(width: number, height: number): CkSurface;
    public makeImageSnapshot(bounds: CkRect | null): CkImage;
    public draw(canvas: CkCanvas, x: number, y: number, sampling: SamplingOption, paint: CkPaint | null): void;
    public readPixels(dstInfo: CkImageInfo, dstPixels: Buffer, dstRowBytes: number, srcX: number, srcY: number): void;
    public peekPixels(): Buffer;
}

export class CkMatrix {
    private constructor();
    public static Identity(): CkMatrix;
    public static Scale(sx: number, sy: number): CkMatrix;
    public static Translate(dx: number, dy: number): CkMatrix;
    public static RotateRad(rad: number, pt: CkPoint): CkMatrix;
    public static Skew(kx: number, ky: number): CkMatrix;
    public static RectToRect(src: CkRect, dst: CkRect, mode: MatrixScaleToFit): CkMatrix;
    public static All(scaleX: number, skewX: number, transX: number,
                      skewY: number, scaleY: number, transY: number,
                      pers0: number, pers1: number, pers2: number): CkMatrix;
    public static Concat(a: CkMatrix, b: CkMatrix): CkMatrix;

    public readonly scaleX: number;
    public readonly scaleY: number;
    public readonly skewX: number;
    public readonly skewY: number;
    public readonly translateX: number;
    public readonly translateY: number;
    public readonly perspectiveX: number;
    public readonly perspectiveY: number;

    public clone(): CkMatrix;
    public rectStaysRect(): boolean;
    public hasPerspective(): boolean;
    public isSimilarity(): boolean;
    public preservesRightAngles(): boolean;
    public preTranslate(dx: number, dy: number): void;
    public preScale(sx: number, sy: number): void;
    public preRotate(rad: number, px: number, py: number): void;
    public preSkew(kx: number, ky: number, px: number, py: number): void;
    public preConcat(other: CkMatrix): CkMatrix;
    public postTranslate(dx: number, dy: number): void;
    public postScale(sx: number, sy: number): void;
    public postRotate(rad: number, px: number, py: number): void;
    public postSkew(kx: number, ky: number, px: number, py: number): void;
    public postConcat(other: CkMatrix): CkMatrix;
    public invert(): null | CkMatrix;
    public normalizePerspective(): CkMatrix;
    public mapPoints(points: Array<CkPoint>): Array<CkPoint>;
    public mapPoint(point: CkPoint): CkPoint;
    public mapHomogeneousPoints(points: Array<CkPoint3>): Array<CkPoint3>;
    public mapRect(src: CkRect, pc: ApplyPerspectiveClip): CkRect;
    public mapRadius(): number;
    public isFinite(): boolean;
}

export class CkPaint {
    public constructor();
    public reset(): void;
    public setAntiAlias(AA: boolean): void;
    public setDither(dither: boolean): void;
    public setStyle(style: PaintStyle): void;
    public setColor(color: number): void;
    public setColor4f(color: CkColor4f): void;
    public setAlpha(alpha: number): void;
    public setAlphaf(alpha: number): void;
    public setStrokeWidth(width: number): void;
    public setStrokeMiter(miter: number): void;
    public setStrokeCap(cap: PaintCap): void;
    public setStrokeJoin(join: PaintJoin): void;
    public setShader(shader: CkShader): void;
    public setColorFilter(filter: CkColorFilter): void;
    public setBlendMode(mode: BlendMode): void;
    public setBlender(blender: CkBlender): void;
    public setPathEffect(effect: CkPathEffect): void;
    public setImageFilter(filter: CkImageFilter): void;
}

export class CkPath {
    public static IsLineDegenerate(p1: CkPoint, p2: CkPoint, exact: boolean): boolean;
    public static IsQuadDegenerate(p1: CkPoint, p2: CkPoint, p3: CkPoint, exact: boolean): boolean;
    public static IsCubicDegenerate(p1: CkPoint, p2: CkPoint, p3: CkPoint, p4: CkPoint, exact: boolean): boolean;
    public constructor();
    public clone(): CkPath;
    public isInterpolatable(compare: CkPath): boolean;
    public interpolate(ending: CkPath, weight: number): CkPath;
    public setFillType(ft: PathFillType): void;
    public toggleInverseFillType(): void;
    public isConvex(): boolean;
    public reset(): void;
    public rewind(): void;
    public isEmpty(): boolean;
    public isLastContourClosed(): boolean;
    public isFinite(): boolean;
    public isVolatile(): boolean;
    public setIsVolatile(volatile: boolean): void;
    public countPoints(): number;
    public getPoint(): CkPoint;
    public getBounds(): CkRect;
    public computeTightBounds(): CkRect;
    public conservativelyContainsRect(rect: CkRect): boolean;
    public moveTo(x: number, y: number): void;
    public rMoveTo(dx: number, y: number): void;
    public lineTo(x: number, y: number): void;
    public rLineTo(x: number, y: number): void;
    public quadTo(x1: number, y1: number, x2: number, y2: number): void;
    public rQuadTo(dx1: number, dy1: number, dx2: number, dy2: number): void;
    public conicTo(x1: number, y1: number, x2: number, y2: number): void;
    public rConicTo(dx1: number, dy1: number, dx2: number, dy2: number): void;
    public cubicTo(x1: number, y1: number, x2: number, y2: number, x3: number, y3: number): void;
    public rCubicTo(dx1: number, dy1: number, dx2: number, dy2: number, dx3: number, dy3: number): void;
    public ooaArcTo(oval: CkRect, startAngleDeg: number, sweepAngleDeg: number, forceMoveTo: boolean): void;
    public pprArcTo(p1: CkPoint, p2: CkPoint, radius: number): void;
    public pspArcTo(r: CkPoint, xAxisRotate: number, arc: PathArcSize, sweep: PathDirection, xy: CkPoint): void;
    public rPspArcTo(rx: number, ry: number, xAxisRotate: number, arc: PathArcSize, sweep: PathDirection,
                     dx: number, dy: number): void;
    public close(): void;
    public addRect(rect: CkRect, dir: PathDirection, start: number): void;
    public addOval(oval: CkRect, dir: PathDirection, start: number): void;
    public addCircle(x: number, y: number, r: number, dir: PathDirection): void;
    public addArc(oval: CkRect, startAngleDeg: number, sweepAngleDeg: number): void;
    public addRRect(rrect: CkRRect, dir: PathDirection, start: number): void;
    public addPoly(pts: Array<CkPoint>, close: boolean): void;
    public addPath(src: CkPath, dx: number, dy: number, mode: PathAddPathMode): void;
    public addPathMatrix(src: CkPath, matrix: CkMatrix, mode: PathAddPathMode): void;
    public reverseAddPath(src: CkPath): void;
    public offset(dx: number, dy: number): void;
    public transform(matrix: CkMatrix, pc: ApplyPerspectiveClip): void;
    public toString(hex: boolean): string;
}

export class CkFontStyle {
    public constructor(weight: number, width: number, slant: FontStyleSlant);
    public static MakeNormal(): CkFontStyle;
    public static MakeBold(): CkFontStyle;
    public static MakeItalic(): CkFontStyle;
    public static MakeBoldItalic(): CkFontStyle;
    readonly weight: number;
    readonly width: number;
    readonly slant: FontStyleSlant;
}

export class CkTypeface {
    public static MakeDefault(): CkTypeface;
    public static MakeFromName(name: string, style: CkFontStyle): CkTypeface;
    public static MakeFromFile(file: string, index: number): CkTypeface;
    public static MakeFromData(buffer: Buffer, index: number): CkTypeface;
    
    readonly fontStyle: CkFontStyle;
    readonly bold: boolean;
    readonly fixedPitch: boolean;
    readonly uniqueID: number;
    readonly unitsPerEm: number;
    readonly familyName: string;
    readonly postScriptName: string | null;
    readonly bounds: CkRect;

    public getKerningPairAdjuestments(glyphs: Uint16Array): Array<number> | null;
    public unicharsToGlyphs(unichars: Uint32Array): Uint16Array;
    public textToGlyphs(text: Buffer, encoding: TextEncoding): Uint16Array | null;
    public unicharToGlyph(unichar: number): number;
    public countGlyphs(): number;
    public countTables(): number;
    public getTableTags(): Uint32Array;
    public getTableSize(tag: number): number;
    public copyTableData(tag: number): Buffer;
}

export class CkFont {
    private constructor();
    public static Make(typeface: CkTypeface): CkFont;
    public static MakeFromSize(typeface: CkTypeface, size: number): CkFont;
    public static MakeTransformed(typeface: CkTypeface, size: number, scaleX: number, skewX: number): CkFont;

    public forceAutoHinting: boolean;
    public embeddedBitmaps: boolean;
    public subpixel: boolean;
    public linearMetrics: boolean;
    public embolden: boolean;
    public baselineSnap: boolean;
    public edging: FontEdging;
    public hinting: FontHinting;
    public size: number;
    public scaleX: number;
    public skewX: number;
    public readonly spacing: number;

    public countText(text: Buffer, encoding: TextEncoding): number;
    public measureText(text: Buffer, encoding: TextEncoding, paint: null | CkPaint): number;
    public measureTextBounds(text: Buffer, encoding: TextEncoding, paint: null | CkPaint): CkRect;
    public getBounds(glyphs: Uint16Array, paint: null | CkPaint): Array<CkRect>;
    public getPos(glyphs: Uint16Array, origin: CkPoint): Array<CkPoint>;
    public getIntercepts(glyphs: Uint16Array, pos: Array<CkPoint>, top: number,
                         bottom: number, paint: null | CkPaint): Float32Array;
    public getPath(glyph: number): null | CkPath;
}

export class CkTextBlob {
    private constructor();
    public static MakeFromText(text: Buffer, font: CkFont, encoding: TextEncoding): CkTextBlob;
    public static MakeFromPosTextH(text: Buffer, xpos: Float32Array, constY: number,
                                   font: CkFont, encoding: TextEncoding): CkTextBlob;
    public static MakeFromPosText(text: Buffer, pos: Array<CkPoint>, font: CkFont,
                                  encoding: TextEncoding): CkTextBlob;

    readonly bounds: CkRect;
    readonly uniqueID: number;

    public getIntercepts(upperBound: number, lowerBound: number, paint: null | CkPaint): Float32Array;
}

export interface CanvasSaveLayerRec {
    bounds: null | CkRect;
    paint: null | CkPaint;
    backdrop: null | CkImageFilter;
    flags: CanvasSaveLayerFlag;
}

export class CkCanvas {
    private constructor();
    public save(): number;
    public saveLayer(bounds: null | CkRect, paint: null | CkPaint): number;
    public saveLayerAlpha(bounds: null | CkRect, alpha: number): number;
    public saveLayerRec(rec: CanvasSaveLayerRec): number;
    public restore(): void;
    public restoreToCount(saveCount: number): void;
    public translate(dx: number, dy: number): void;
    public scale(sx: number, sy: number): void;
    public rotate(rad: number, px: number, py: number): void;
    public skew(sx: number, sy: number)
    public concat(matrix: CkMatrix): void;
    public setMatrix(matrix: CkMatrix): void;
    public resetMatrix(): void;
    public clipRect(rect: CkRect, op: ClipOp, AA: boolean): void;
    public clipRRect(rrect: CkRRect, op: ClipOp, AA: boolean): void;
    public clipPath(path: CkPath, op: ClipOp, AA: boolean): void;
    public clipShader(shader: CkShader, op: ClipOp): void;
    public quickRejectRect(rect: CkRect): boolean;
    public quickRejectPath(path: CkPath): boolean;
    public getLocalClipBounds(): CkRect
    public getDeviceClipBounds(): CkRect
    public drawColor(color: CkColor4f, mode: BlendMode): void;
    public clear(color: CkColor4f): void;
    public drawPaint(paint: CkPaint): void;
    public drawPoints(mode: CanvasPointMode, points: Array<CkPoint>, paint: CkPaint): void;
    public drawPoint(x: number, y: number, paint: CkPaint): void;
    public drawLine(p1: CkPoint, p2: CkPoint, paint: CkPaint): void;
    public drawRect(rect: CkRect, paint: CkPaint): void;
    public drawOval(oval: CkRect, paint: CkPaint): void;
    public drawRRect(rrect: CkRRect, paint: CkPaint): void;
    public drawDRRect(outer: CkRRect, inner: CkRRect, paint: CkPaint): void;
    public drawCircle(cx: number, cy: number, r: number, paint: CkPaint): void;
    public drawArc(oval: CkRect, startAngle: number, sweepAngle: number, useCenter: boolean, paint: CkPaint): void;
    public drawRoundRect(rect: CkRect, rx: number, ry: number, paint: CkPaint): void;
    public drawPath(path: CkPath, paint: CkPaint): void;
    public drawImage(image: CkImage, left: number, top: number, sampling: SamplingOption, paint: null | CkPaint): void;
    public drawImageRect(image: CkImage, src: CkRect, dst: CkRect, sampling: SamplingOption, paint: null | CkPaint,
                         constraint: CanvasSrcRectConstraint): void;
    public drawString(str: string, x: number, y: number, font: CkFont, paint: CkPaint): void;
    public drawGlyphs(glyphs: Uint16Array, positions: Array<CkPoint>, origin: CkPoint, font: CkFont, paint: CkPaint): void;
    public drawTextBlob(blob: CkTextBlob, x: number, y: number, paint: CkPaint): void;
    public drawPicture(picture: CkPicture, matrix: null | CkMatrix, paint: null | CkPaint): void;
    public drawVertices(vertices: CkVertices, mode: BlendMode, paint: CkPaint): void;
    public drawPatch(cubics: Array<CkPoint>, colors: Array<CkColor4f>, texCoords: Array<CkPoint>,
                     mode: BlendMode, paint: CkPaint): void;
}

export class CkVertices {
    // TODO(sora): to be implemented
}

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

export class CkShader {
    public static MakeFromDSL(dsl: string, kwargs: object): CkShader;
    
    public makeWithLocalMatrix(matrix: CkMatrix): CkShader;
    public makeWithColorFilter(filter: CkColorFilter): CkShader;
}

export class CkBlender {
    public static Mode(mode: BlendMode): CkBlender;
    public static Arithmetic(k1: number, k2: number, k3: number, k4: number, enforcePM: boolean): CkBlender;
}

export class CkPathEffect {
    public static MakeFromDSL(dsl: string, kwargs: object): CkPathEffect;
}

export class CkBitmap {
    public static MakeFromBuffer(buffer: Buffer,
                                 width: number,
                                 height: number,
                                 colorType: ColorType,
                                 alphaType: AlphaType): CkBitmap;
    
    public static MakeFromEncodedFile(path: string): CkBitmap;

    readonly width: number;
    readonly height: number;
    readonly alphaType: AlphaType;
    readonly colorType: ColorType;
    readonly bytesPerPixel: number;
    readonly rowBytesAsPixels: number;
    readonly shiftPerPixel: number;
    readonly rowBytes: number;
    
    public computeByteSize(): number;
    public toImage(): CkImage;
    public getPixelBuffer(): Buffer;
}

export interface ImageExportedPixelsBuffer {
    buffer: Buffer;
    width: number;
    height: number;
    colorType: ColorType;
    alphaType: AlphaType;
    rowBytes: number;
}

export class CkImage {
    public static MakeFromEncodedData(buffer: Buffer): Promise<CkImage>;
    public static MakeFromEncodedFile(path: string): Promise<CkImage>;
    public static MakeFromVideoBuffer(vbo: VideoBuffer): CkImage;

    readonly width: number;
    readonly height: number;
    readonly alphaType: AlphaType;
    readonly colorType: ColorType;

    public uniqueId(): number;
    public encodeToData(format: number, quality: number): Buffer;
    public makeSharedPixelsBuffer(): ImageExportedPixelsBuffer;
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

export class CkPictureRecorder {
    public constructor();
    public beginRecording(bounds: CkRect): CkCanvas;
    public getRecordingCanvas(): CkCanvas | null;
    public finishRecordingAsPicture(): CkPicture | null;
    public finishRecordingAsPictureWithCull(cull: CkRect): CkPicture | null;
}
