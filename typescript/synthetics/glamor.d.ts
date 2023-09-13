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

import {VideoBuffer} from 'synthetic://utau';
import {EventEmitterBase} from 'private/base';

export interface CkLTRBRect {
    left: number;
    top: number;
    bottom: number;
    right: number;
}

export interface CkXYWHRect {
    x: number;
    y: number;
    width: number;
    height: number;
}

export type CkArrayXYWHRect = [number, number, number, number];
export type CkRect = CkLTRBRect | CkXYWHRect | CkArrayXYWHRect | Float32Array;

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

type TypedArray = Uint8Array | Int8Array | Uint8ClampedArray | Uint16Array | Int16Array
    | Uint32Array | Int32Array | Float64Array | Float32Array | BigInt64Array
    | BigUint64Array;

// [x, y]
export type CkPoint = [number, number];

// [x, y, z]
export type CkPoint3 = [number, number, number];

// [R, G, B, A] where R,G,B,A∈[0,1]
export type CkColor4f = [number, number, number, number];

/**
 * Overwrite the default application name and version.
 * The application info will not be parsed, and only be used for identify
 * the application itself.
 */
export function setApplicationInfo(name: string, major: number,
                                   minor: number, patch: number): void;

/**
 * Dump memory statistics of Skia cache. Returned value is a string
 * in JSON format. `JSON.parse()` could be used for further analysis.
 */
export function traceSkiaMemoryJSON(): string;

/**
 * Query a certain capability of current Glamor context.
 * This function can be called before Glamor is initialized.
 * See `Constants.CAPABILITY_*` constants for available capabilities.
 */
export function queryCapabilities(cap: Capability): boolean | number | string;

export class PresentThread {
    private constructor();

    static Start(): Promise<PresentThread>;

    dispose(): void;

    createDisplay(): Promise<Display>;

    collect(): void;

    startMessageProfiling(): void;

    finishMessageProfilingJSON(): Promise<string>;
}

/**
 * An abstracted interface for system's display server (like Wayland, X11, etc.).
 *
 * @event [closed] Emitted when the display is closed.
 *        Prototype: () -> void
 *
 * @event [monitor-added] Emitted when a new monitor was plugged into the system.
 *                        Note that the monitors that have existed in the system when the
 *                        display object is created will not cause this signal to be emitted.
 *        Prototype: (monitor: Monitor) -> void
 *
 * @event [monitor-removed] Emitted when an existing monitor was removed.
 *                          The corresponding `Monitor` object will also be notified by `detached` signal.
 *        Prototype: (monitor: Monitor) -> void
 */
export class Display extends EventEmitterBase {
    /**
     * Close the display. This method should not be called for multiple times.
     * Signal `closed` will be emitted when the display is actually closed.
     * Promise will be resolved when the close request is processed.
     */
    close(): Promise<void>;

    /**
     * Create a toplevel window (aka surface). All the rendering operations
     * performed on the created window will use an internal CPU rasterizer.
     *
     * @param w Width of the new created window.
     * @param h Height of the new created window.
     */
    createRasterSurface(w: number, h: number): Promise<Surface>;

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
    createHWComposeSurface(w: number, h: number): Promise<Surface>;

    /**
     * Get a list of monitors connected to the system.
     */
    requestMonitorList(): Promise<Array<Monitor>>;

    readonly defaultCursorTheme: CursorTheme;

    loadCursorTheme(name: string, size: number): Promise<CursorTheme>;

    createCursor(bitmap: CkBitmap, hotspotX: number, hotspotY: number): Promise<Cursor>;
}

export class CursorTheme {
    dispose(): Promise<void>;

    loadCursorFromName(name: string): Promise<Cursor>;
}

export class Cursor {
    dispose(): Promise<void>;

    getHotspotVector(): Promise<{ x: number, y: number }>;
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
 * @event [properties-changed] Notify that the properties of the Monitor has changed.
 *                             New properties are provided as an argument of the signal.
 *                             It also can be triggered by call `requestPropertySet` manually.
 *        Prototype: (properties: MonitorPropertySet) -> void
 *
 * @event [detached] Monitor has been detached from the `Display` object,
 *                   which means the corresponding physical monitor has been removed from
 *                   the system or disabled.
 *        Prototype: () -> void
 */
export class Monitor extends EventEmitterBase {
    /**
     * Get a set of monitor's properties.
     * Promise returned by the function will be resolved when the rendering thread
     * accepts the request. Monitor properties will be sent with signal `properties-changed`
     * later.
     */
    requestPropertySet(): Promise<void>;
}

/**
 * An abstraction of a toplevel visual window where the contents can be rendered.
 * As `Surface` object itself only provides methods to operate the window,
 * a `Blender` object, which provides methods for rendering and frame scheduling,
 * should be associated with the `Surface` object.
 *
 * @event [closed] Emitted when the window has been closed.
 *                 Prototype: () -> void
 *
 * @event [resize] Emitted when the window has been resized.
 *                 Prototype: (width: number, height: number) -> void
 *
 * @event [configure] Emitted when the Window Manager notified us that the window
 *                    should be reconfigured (resize, move, etc.)
 *                    Prototype: (width: number, height: number, state: ToplevelStates) -> void
 *
 * @event [frame] Emitted when it is a good time to start submitting a new frame.
 *                This signal is related to the VSync mechanism where the system will
 *                notify us when the monitor will refresh for next frame.
 *                There are two ways to schedule this signal: call `Surface.requestNextFrame`
 *                explicitly or call `Blender.update`, which also calls `requestNextFrame` implicitly.
 *                Prototype: () -> void
 *
 * @event [pointer-hovering] Emitted when the pointer device enters or leaves the window area.
 *                           Prototype: (enter: boolean) -> void
 *
 * @event [pointer-motion] Emitted when a pointer moves on the window.
 *                         Prototype: (double x, double y) -> void
 *
 * @event [pointer-button] Emitted when a button of pointer device is pressed or released.
 *                         Prototype: (button: PointerButton, pressed: boolean) -> void
 *
 * @event [pointer-axis] <Undocumented>
 *                       Prototype: (sourceType: PointerAxisSource, dx: number, dy: number) -> void
 *
 *
 * @event [pointer-highres-scroll] <Undocumented>
 *                                 Prototype: (sourceType: PointerAxisSource, dx: number, dy: number) -> void
 *
 * @event [keyboard-focus] <Undocumented>
 *                         Prototype: (focused: boolean) -> void
 *
 * @event [keyboard-key] <Undocumented>
 *                       Prototype: (key: KeyboardKey, modifiers: KeyboardModifiers, pressed: boolean) -> void
 */
export class Surface extends EventEmitterBase {
    /* Window width in pixels. */
    readonly width: number;
    /* Window height in pixels. */
    readonly height: number;

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
    createBlender(): Promise<Blender>;

    /**
     * Close the window immediately.
     * `close` signal does not indicate that a window is closed, which just means user
     * has clicked the "close" button on the window decoration.
     * Only by calling `close` method can we actually close a window.
     *
     * `closed` single will be emitted when the window is actually closed.
     */
    close(): Promise<void>;

    /**
     * Get a string representation about available framebuffers.
     * There is no any guarantee that the returned string has a specific format.
     * The format is highly implementation-defined.
     */
    getBufferDescriptor(): Promise<string>;

    /**
     * Schedule next frame to keep VSync.
     * The resolution of Promise only means the rendering thread has accepted
     * the request. A `frame` signal will be emitted when it is a good time to display
     * next frame.
     */
    requestNextFrame(): Promise<void>;

    /**
     * Set a title for the window.
     */
    setTitle(title: string): Promise<void>;

    /**
     * Resize the window.
     * The resolution of Promise only means the rendering thread has accepted
     * the request. A `resize` signal will be emitted when the resizing is finished.
     */
    resize(width: number, height: number): Promise<void>;

    /**
     * Set a maximum geometry size of the window in pixels.
     */
    setMinSize(width: number, height: number): Promise<void>;

    /**
     * Set a minimum geometry size of the window in pixels.
     */
    setMaxSize(width: number, height: number): Promise<void>;

    /**
     * Maximize or unmaximize the window if the system Window Manager supports
     * the operation.
     */
    setMaximized(value: boolean): Promise<void>;

    /**
     * Minimize or unminimize the window if the system Window Manager supports
     * the operation.
     */
    setMinimized(value: boolean): Promise<void>;

    /**
     * Make the window enter or leave fullscreen state.
     *
     * @param value     true to enter the fullscreen state; otherwise, leave the fullscreen state.
     * @param monitor   A monitor where the fullscreen window should be displayed.
     */
    setFullscreen(value: boolean, monitor: Monitor): Promise<void>;
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
    generateCurrentReport(): GProfilerReport;

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
    purgeRecentHistorySamples(freeMemory: boolean): void;
}

type TextureId = number;

/**
 * Blender, a content aggregator in the Glamor rendering framework, mainly manages
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
 * @event [picture-captured] A captured Picture of current frame has been delivered
 *                           from rendering thread. The serial number corresponds with the
 *                           number returned by `captureNextFrameAsPicture`.
 *                           Prototype: (pict: CriticalPicture, serial: number) -> void
 *
 * @event [<dynamic: texture deletion>] A texture has been deleted. The name of the signal
 *                                      is specified by user.
 *                                      Prototype: () -> void
 */
export class Blender extends EventEmitterBase {
    readonly profiler: GProfiler | null;

    dispose(): Promise<void>;

    update(scene: Scene): Promise<void>;

    purgeRasterCacheResources(): Promise<void>;

    /**
     * Record the rasterization process of current frame (current layer tree)
     * as a Picture.
     * The caller will be notified by `picture-captured` signal when
     * the Picture is generated.
     * All the required drawing resources are interned
     * in the captured Picture (including textures, images, typefaces, etc.).
     *
     * @return A serial number of capture increasing with frame counter.
     *         If the function is called for multiple times before calling `update`,
     *         it will return the same serial number.
     */
    captureNextFrameAsPicture(): Promise<number>;

    /**
     * If HWCompose backend is used, import a semaphore object that was exported
     * from other `GpuDirectContext`, and return a number presenting the ID of
     * the imported semaphore.
     * If raster backend is used, this operation will fail.
     */
    importGpuSemaphoreFd(fd: GpuExportedFd): Promise<bigint>;

    /**
     * If HWCompose backend is used, delete an imported semaphore object.
     * If raster backend is used, this operation will fail.
     */
    deleteImportedGpuSemaphore(id: bigint): Promise<void>;
}

export class Scene {
    /**
     * A `Scene` instance MUST be disposed as quickly as possible
     * if it is not used anymore.
     */
    dispose(): void;

    // ~Experimental API~
    toImage(width: number, height: number): Promise<CkImage>;

    /**
     * Get an S-Expression representation of the layer tree.
     * Related tools provided by Cocoa Project can be used to perform
     * further analysis for the returned string.
     */
    toString(): string;
}

export class SceneBuilder {
    constructor(viewportWidth: number, viewportHeight: number);

    build(): Scene;

    pop(): SceneBuilder;

    addPicture(picture: CkPicture,
               autoFastClipping: boolean,
               dx: number, dy: number): SceneBuilder;

    addTexture(textureId: TextureId,
               offsetX: number,
               offsetY: number,
               width: number,
               height: number,
               sampling: number): SceneBuilder;

    addVideoBuffer(vbo: VideoBuffer,
                   offsetX: number,
                   offsetY: number,
                   width: number,
                   height: number,
                   sampling: number): SceneBuilder;

    pushOffset(offsetX: number, offsetY: number): SceneBuilder;

    pushOpacity(alpha: number): SceneBuilder;

    pushRotate(rad: number, pivotX: number, pivotY: number): SceneBuilder;

    pushImageFilter(filter: CkImageFilter): SceneBuilder;

    pushRectClip(shape: CkRect, AA: boolean): SceneBuilder;

    pushRRectClip(shape: CkRRect, AA: boolean): SceneBuilder;

    pushPathClip(shape: CkPath, op: ClipOp, AA: boolean): SceneBuilder;

    pushBackdropFilter(filter: CkImageFilter,
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
export type PathMeasureMatrixFlags = number;
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
export type RuntimeEffectUniformType = number;
export type RuntimeEffectUniformFlag = number;
export type RuntimeEffectChildType = number;
export type VerticesVertexMode = number;
export type CkSurfaceContentChangeMode = number;
export type ToplevelStates = number;
export type ImageBitDepth = number;
export type TextureCompressionType = number;
export type GpuSemaphoreSubmitted = number;

interface Constants {
    readonly CAPABILITY_HWCOMPOSE_ENABLED: Capability;
    readonly CAPABILITY_PROFILER_ENABLED: Capability;
    readonly CAPABILITY_PROFILER_MAX_SAMPLES: Capability;
    readonly CAPABILITY_MESSAGE_QUEUE_PROFILING_ENABLED: Capability;

    readonly GPU_SEMAPHORE_SUBMITTED_YES: GpuSemaphoreSubmitted;
    readonly GPU_SEMAPHORE_SUBMITTED_NO: GpuSemaphoreSubmitted;

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

    readonly IMAGE_BIT_DEPTH_U8: ImageBitDepth;
    readonly IMAGE_BIT_DEPTH_F16: ImageBitDepth;

    readonly TEXTURE_COMPRESSION_TYPE_ETC2_RGB8_UNORM: TextureCompressionType;
    readonly TEXTURE_COMPRESSION_TYPE_BC1_RGB8_UNORM: TextureCompressionType;
    readonly TEXTURE_COMPRESSION_TYPE_BC1_RGBA8_UNORM: TextureCompressionType;

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

    readonly PATH_MEASURE_MATRIX_FLAGS_GET_POSITION: PathMeasureMatrixFlags;
    readonly PATH_MEASURE_MATRIX_FLAGS_GET_TANGENT: PathMeasureMatrixFlags;

    readonly APPLY_PERSPECTIVE_CLIP_YES: ApplyPerspectiveClip;
    readonly APPLY_PERSPECTIVE_CLIP_NO: ApplyPerspectiveClip;

    readonly MATRIX_SCALE_TO_FIT_FILL: MatrixScaleToFit;
    readonly MATRIX_SCALE_TO_FIT_START: MatrixScaleToFit;
    readonly MATRIX_SCALE_TO_FIT_CENTER: MatrixScaleToFit;
    readonly MATRIX_SCALE_TO_FIT_END: MatrixScaleToFit;

    readonly CKSURFACE_CONTENT_CHANGE_MODE_DISCARD: CkSurfaceContentChangeMode;
    readonly CKSURFACE_CONTENT_CHANGE_MODE_RETAIN: CkSurfaceContentChangeMode;

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

    readonly RUNTIME_EFFECT_UNIFORM_TYPE_FLOAT: RuntimeEffectUniformType;
    readonly RUNTIME_EFFECT_UNIFORM_TYPE_FLOAT2: RuntimeEffectUniformType;
    readonly RUNTIME_EFFECT_UNIFORM_TYPE_FLOAT3: RuntimeEffectUniformType;
    readonly RUNTIME_EFFECT_UNIFORM_TYPE_FLOAT4: RuntimeEffectUniformType;
    readonly RUNTIME_EFFECT_UNIFORM_TYPE_FLOAT2X2: RuntimeEffectUniformType;
    readonly RUNTIME_EFFECT_UNIFORM_TYPE_FLOAT3X3: RuntimeEffectUniformType;
    readonly RUNTIME_EFFECT_UNIFORM_TYPE_FLOAT4X4: RuntimeEffectUniformType;
    readonly RUNTIME_EFFECT_UNIFORM_TYPE_INT: RuntimeEffectUniformType;
    readonly RUNTIME_EFFECT_UNIFORM_TYPE_INT2: RuntimeEffectUniformType;
    readonly RUNTIME_EFFECT_UNIFORM_TYPE_INT3: RuntimeEffectUniformType;
    readonly RUNTIME_EFFECT_UNIFORM_TYPE_INT4: RuntimeEffectUniformType;

    readonly RUNTIME_EFFECT_UNIFORM_FLAG_ARRAY: RuntimeEffectUniformFlag;
    readonly RUNTIME_EFFECT_UNIFORM_FLAG_COLOR: RuntimeEffectUniformFlag;
    readonly RUNTIME_EFFECT_UNIFORM_FLAG_VERTEX: RuntimeEffectUniformFlag;
    readonly RUNTIME_EFFECT_UNIFORM_FLAG_FRAGMENT: RuntimeEffectUniformFlag;
    readonly RUNTIME_EFFECT_UNIFORM_FLAG_HALF_PRECISION: RuntimeEffectUniformFlag;

    readonly RUNTIME_EFFECT_CHILD_TYPE_SHADER: RuntimeEffectChildType;
    readonly RUNTIME_EFFECT_CHILD_TYPE_COLOR_FILTER: RuntimeEffectChildType;
    readonly RUNTIME_EFFECT_CHILD_TYPE_BLENDER: RuntimeEffectChildType;

    readonly VERTICES_VERTEX_MODE_TRIANGLES: VerticesVertexMode;
    readonly VERTICES_VERTEX_MODE_TRIANGLE_FAN: VerticesVertexMode;
    readonly VERTICES_VERTEX_MODE_TRIANGLE_STRIP: VerticesVertexMode;

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

    readonly MONITOR_SUBPIXEL_UNKNOWN: MonitorSubpixel;
    readonly MONITOR_SUBPIXEL_NONE: MonitorSubpixel;
    readonly MONITOR_SUBPIXEL_HORIZONTAL_RGB: MonitorSubpixel;
    readonly MONITOR_SUBPIXEL_HORIZONTAL_BGR: MonitorSubpixel;
    readonly MONITOR_SUBPIXEL_VERTICAL_RGB: MonitorSubpixel;
    readonly MONITOR_SUBPIXEL_VERTICAL_BGR: MonitorSubpixel;
    readonly MONITOR_TRANSFORM_NORMAL: MonitorTransform;
    readonly MONITOR_TRANSFORM_ROTATE_90: MonitorTransform;
    readonly MONITOR_TRANSFORM_ROTATE_180: MonitorTransform;
    readonly MONITOR_TRANSFORM_ROTATE_270: MonitorTransform;
    readonly MONITOR_TRANSFORM_FLIPPED: MonitorTransform;
    readonly MONITOR_TRANSFORM_FLIPPED_90: MonitorTransform;
    readonly MONITOR_TRANSFORM_FLIPPED_180: MonitorTransform;
    readonly MONITOR_TRANSFORM_FLIPPED_270: MonitorTransform;
    readonly MONITOR_MODE_CURRENT: MonitorMode;
    readonly MONITOR_MODE_PREFERRED: MonitorMode;

    readonly SURFACE_TOPLEVEL_MAXIMIZED: ToplevelStates;
    readonly SURFACE_TOPLEVEL_FULLSCREEN: ToplevelStates;
    readonly SURFACE_TOPLEVEL_RESIZING: ToplevelStates;
    readonly SURFACE_TOPLEVEL_ACTIVATED: ToplevelStates;
    readonly SURFACE_TOPLEVEL_TILED_LEFT: ToplevelStates;
    readonly SURFACE_TOPLEVEL_TILED_RIGHT: ToplevelStates;
    readonly SURFACE_TOPLEVEL_TILED_TOP: ToplevelStates;
    readonly SURFACE_TOPLEVEL_TILED_BOTTOM: ToplevelStates;
}

export declare let Constants: Constants;

export interface GpuFlushInfo {
    signalSemaphores?: Array<GpuBinarySemaphore>;
    onFinished?: () => void;
    onSubmitted?: (success: boolean) => void;
}

export class GpuDirectContext {
    static Make(): GpuDirectContext;

    dispose(): void;

    isDisposed(): boolean;

    makeSurface(imageInfo: CkImageInfo, budgeted: boolean, aaSamplesPerPixel: number): CkSurface;

    makeBinarySemaphore(): GpuBinarySemaphore;

    exportSemaphoreFd(semaphore: GpuBinarySemaphore): GpuExportedFd;

    importSemaphoreFd(fd: GpuExportedFd): GpuBinarySemaphore;

    flush(info: GpuFlushInfo): GpuSemaphoreSubmitted;

    submit(waitForSubmit: boolean): boolean;
}

export class GpuExportedFd {
    private constructor();

    close(): void;

    isImportedOrClosed(): boolean;
}

export class GpuBinarySemaphore {
    dispose(): void;

    detach(): void;

    isDetachedOrDisposed(): boolean;
}

export class CkImageInfo {
    static MakeSRGB(w: number, h: number, colorType: ColorType, alphaType: AlphaType): CkImageInfo;

    static MakeN32(w: number, h: number, alphaType: AlphaType): CkImageInfo;

    static MakeS32(w: number, h: number, alphaType: AlphaType): CkImageInfo;

    static MakeN32Premul(w: number, h: number): CkImageInfo;

    static MakeA8(w: number, h: number): CkImageInfo;

    static MakeUnknown(w: number, h: number): CkImageInfo;

    readonly alphaType: AlphaType;
    readonly colorType: ColorType;
    readonly width: number;
    readonly height: number;
    readonly isEmpty: boolean;
    readonly isOpaque: boolean;
    readonly bytesPerPixel: number;
    readonly shiftPerPixel: number;
    readonly minRowBytes: number;

    makeWH(w: number, h: number): CkImageInfo;

    makeAlphaType(alphaType: AlphaType): CkImageInfo;

    makeColorType(colorType: ColorType): CkImageInfo;

    computeOffset(x: number, y: number, rowBytes: number): number;

    equalsTo(other: CkImageInfo): boolean;

    computeByteSize(rowBytes: number): number;

    computeMinByteSize(): number;

    validRowBytes(rowBytes: number): boolean;
}

export class CkSurface {
    private constructor();

    static MakeRaster(imageInfo: CkImageInfo): CkSurface;
    static MakeNull(width: number, height: number): CkSurface;
    static WrapPixels(imageInfo: CkImageInfo, rowBytes: number, pixels: TypedArray): CkSurface;

    dispose(): void;

    isDisposed(): boolean;

    readonly width: number;
    readonly height: number;
    readonly generationID: number;
    readonly imageInfo: CkImageInfo;

    getCanvas(): CkCanvas;

    getGpuDirectContext(): GpuDirectContext | null;

    makeSurface(width: number, height: number): CkSurface;

    makeImageSnapshot(bounds: CkRect | null): CkImage;

    draw(canvas: CkCanvas, x: number, y: number,
         sampling: SamplingOption, paint: CkPaint | null): void;

    peekPixels<T>(scopeCallback: (pixmap: CkPixmap) => T): T;

    readPixels(dstInfo: CkImageInfo, dstPixels: Uint8Array,
               dstRowBytes: number, srcX: number, srcY: number): void;

    readPixelsToPixmap(pixmap: CkPixmap, srcX: number, srcY: number): void;

    writePixels(pixmap: CkPixmap, dstX: number, dstY: number): void;

    notifyContentWillChange(mode: CkSurfaceContentChangeMode): void;

    waitOnGpu(waitSemaphores: Array<GpuBinarySemaphore>,
              takeSemaphoresOwnership: boolean): boolean;

    flush(info: GpuFlushInfo): GpuSemaphoreSubmitted;
}

export class CkMatrix {
    private constructor();

    static Identity(): CkMatrix;

    static Scale(sx: number, sy: number): CkMatrix;

    static Translate(dx: number, dy: number): CkMatrix;

    static RotateRad(rad: number, pt: CkPoint): CkMatrix;

    static Skew(kx: number, ky: number): CkMatrix;

    static RectToRect(src: CkRect, dst: CkRect, mode: MatrixScaleToFit): CkMatrix;

    static All(scaleX: number, skewX: number, transX: number,
               skewY: number, scaleY: number, transY: number,
               pers0: number, pers1: number, pers2: number): CkMatrix;

    static Concat(a: CkMatrix, b: CkMatrix): CkMatrix;

    readonly scaleX: number;
    readonly scaleY: number;
    readonly skewX: number;
    readonly skewY: number;
    readonly translateX: number;
    readonly translateY: number;
    readonly perspectiveX: number;
    readonly perspectiveY: number;

    clone(): CkMatrix;

    rectStaysRect(): boolean;

    hasPerspective(): boolean;

    isSimilarity(): boolean;

    preservesRightAngles(): boolean;

    preTranslate(dx: number, dy: number): void;

    preScale(sx: number, sy: number): void;

    preRotate(rad: number, px: number, py: number): void;

    preSkew(kx: number, ky: number, px: number, py: number): void;

    preConcat(other: CkMatrix): CkMatrix;

    postTranslate(dx: number, dy: number): void;

    postScale(sx: number, sy: number): void;

    postRotate(rad: number, px: number, py: number): void;

    postSkew(kx: number, ky: number, px: number, py: number): void;

    postConcat(other: CkMatrix): CkMatrix;

    invert(): null | CkMatrix;

    normalizePerspective(): CkMatrix;

    mapPoints(points: Array<CkPoint>): Array<CkPoint>;

    mapPoint(point: CkPoint): CkPoint;

    mapHomogeneousPoints(points: Array<CkPoint3>): Array<CkPoint3>;

    mapRect(src: CkRect, pc: ApplyPerspectiveClip): CkArrayXYWHRect;

    mapRadius(): number;

    isFinite(): boolean;
}

export class CkPixmap {
    constructor(imageInfo: CkImageInfo, rowBytes: number, buffer: TypedArray);
    constructor();

    resetEmpty(): void;

    reset(imageInfo: CkImageInfo, rowBytes: number, buffer: TypedArray): void;

    extractSubset(area: CkRect): CkPixmap | null;

    readonly info: CkImageInfo;
    readonly rowBytes: number;
    readonly width: number;
    readonly height: number;
    readonly colorType: ColorType;
    readonly alphaType: AlphaType;
    readonly isOpaque: boolean;
    readonly bounds: CkRect;
    readonly rowBytesAsPixels: number;
    readonly shiftPerPixel: number;

    computeByteSize(): number;

    computeIsOpaque(): boolean;

    getColor4f(x: number, y: number): CkColor4f;

    getAlphaf(x: number, y: number): number;

    readPixels(dstInfo: CkImageInfo, dstBuffer: TypedArray, dstRowBytes: number, srcX: number, srcY: number): void;

    copy(dst: CkPixmap, srcX: number, srcY: number): void;

    scale(dst: CkPixmap, sampling: SamplingOption): void;

    erase(color: CkColor4f, subset: CkRect | null): void;
}

export class CkPaint {
    constructor();

    reset(): void;

    setAntiAlias(AA: boolean): void;

    setDither(dither: boolean): void;

    setStyle(style: PaintStyle): void;

    setColor(color: number): void;

    setColor4f(color: CkColor4f): void;

    setAlpha(alpha: number): void;

    setAlphaf(alpha: number): void;

    setStrokeWidth(width: number): void;

    setStrokeMiter(miter: number): void;

    setStrokeCap(cap: PaintCap): void;

    setStrokeJoin(join: PaintJoin): void;

    setShader(shader: CkShader): void;

    setColorFilter(filter: CkColorFilter): void;

    setBlendMode(mode: BlendMode): void;

    setBlender(blender: CkBlender): void;

    setPathEffect(effect: CkPathEffect): void;

    setImageFilter(filter: CkImageFilter): void;
}

export class CkPath {
    static IsLineDegenerate(p1: CkPoint, p2: CkPoint, exact: boolean): boolean;

    static IsQuadDegenerate(p1: CkPoint, p2: CkPoint, p3: CkPoint, exact: boolean): boolean;

    static IsCubicDegenerate(p1: CkPoint, p2: CkPoint, p3: CkPoint, p4: CkPoint, exact: boolean): boolean;

    constructor();

    clone(): CkPath;

    isInterpolatable(compare: CkPath): boolean;

    interpolate(ending: CkPath, weight: number): CkPath;

    setFillType(ft: PathFillType): void;

    toggleInverseFillType(): void;

    isConvex(): boolean;

    reset(): void;

    rewind(): void;

    isEmpty(): boolean;

    isLastContourClosed(): boolean;

    isFinite(): boolean;

    isVolatile(): boolean;

    setIsVolatile(volatile: boolean): void;

    countPoints(): number;

    getPoint(): CkPoint;

    getBounds(): CkRect;

    computeTightBounds(): CkRect;

    conservativelyContainsRect(rect: CkRect): boolean;

    moveTo(x: number, y: number): void;

    rMoveTo(dx: number, y: number): void;

    lineTo(x: number, y: number): void;

    rLineTo(x: number, y: number): void;

    quadTo(x1: number, y1: number, x2: number, y2: number): void;

    rQuadTo(dx1: number, dy1: number, dx2: number, dy2: number): void;

    conicTo(x1: number, y1: number, x2: number, y2: number): void;

    rConicTo(dx1: number, dy1: number, dx2: number, dy2: number): void;

    cubicTo(x1: number, y1: number, x2: number, y2: number, x3: number, y3: number): void;

    rCubicTo(dx1: number, dy1: number, dx2: number, dy2: number, dx3: number, dy3: number): void;

    ooaArcTo(oval: CkRect, startAngleDeg: number, sweepAngleDeg: number, forceMoveTo: boolean): void;

    pprArcTo(p1: CkPoint, p2: CkPoint, radius: number): void;

    pspArcTo(r: CkPoint, xAxisRotate: number, arc: PathArcSize, sweep: PathDirection, xy: CkPoint): void;

    rPspArcTo(rx: number, ry: number, xAxisRotate: number, arc: PathArcSize, sweep: PathDirection,
              dx: number, dy: number): void;

    close(): void;

    addRect(rect: CkRect, dir: PathDirection, start: number): void;

    addOval(oval: CkRect, dir: PathDirection, start: number): void;

    addCircle(x: number, y: number, r: number, dir: PathDirection): void;

    addArc(oval: CkRect, startAngleDeg: number, sweepAngleDeg: number): void;

    addRRect(rrect: CkRRect, dir: PathDirection, start: number): void;

    addPoly(pts: Array<CkPoint>, close: boolean): void;

    addPath(src: CkPath, dx: number, dy: number, mode: PathAddPathMode): void;

    addPathMatrix(src: CkPath, matrix: CkMatrix, mode: PathAddPathMode): void;

    reverseAddPath(src: CkPath): void;

    offset(dx: number, dy: number): void;

    transform(matrix: CkMatrix, pc: ApplyPerspectiveClip): void;

    toString(hex: boolean): string;
}

export class CkFontStyle {
    constructor(weight: number, width: number, slant: FontStyleSlant);

    static MakeNormal(): CkFontStyle;

    static MakeBold(): CkFontStyle;

    static MakeItalic(): CkFontStyle;

    static MakeBoldItalic(): CkFontStyle;

    readonly weight: number;
    readonly width: number;
    readonly slant: FontStyleSlant;
}

export class CkTypeface {
    static MakeDefault(): CkTypeface;

    static MakeFromName(name: string, style: CkFontStyle): CkTypeface;

    static MakeFromFile(file: string, index: number): CkTypeface;

    static MakeFromData(buffer: TypedArray, index: number): CkTypeface;

    readonly fontStyle: CkFontStyle;
    readonly bold: boolean;
    readonly fixedPitch: boolean;
    readonly uniqueID: number;
    readonly unitsPerEm: number;
    readonly familyName: string;
    readonly postScriptName: string | null;
    readonly bounds: CkRect;

    getKerningPairAdjustments(glyphs: Uint16Array): Array<number> | null;

    unicharsToGlyphs(unichars: Uint32Array): Uint16Array;

    textToGlyphs(text: Uint8Array, encoding: TextEncoding): Uint16Array | null;

    unicharToGlyph(unichar: number): number;

    countGlyphs(): number;

    countTables(): number;

    getTableTags(): Uint32Array;

    getTableSize(tag: number): number;

    copyTableData(tag: number): Uint8Array;
}

export class CkFont {
    private constructor();

    static Make(typeface: CkTypeface): CkFont;

    static MakeFromSize(typeface: CkTypeface, size: number): CkFont;

    static MakeTransformed(typeface: CkTypeface, size: number, scaleX: number, skewX: number): CkFont;

    forceAutoHinting: boolean;
    embeddedBitmaps: boolean;
    subpixel: boolean;
    linearMetrics: boolean;
    embolden: boolean;
    baselineSnap: boolean;
    edging: FontEdging;
    hinting: FontHinting;
    size: number;
    scaleX: number;
    skewX: number;
    readonly spacing: number;

    countText(text: Uint8Array, encoding: TextEncoding): number;

    measureText(text: Uint8Array, encoding: TextEncoding, paint: null | CkPaint): number;

    measureTextBounds(text: Uint8Array, encoding: TextEncoding, paint: null | CkPaint): CkArrayXYWHRect;

    getBounds(glyphs: Uint16Array, paint: null | CkPaint): Array<CkArrayXYWHRect>;

    getPos(glyphs: Uint16Array, origin: CkPoint): Array<CkPoint>;

    getIntercepts(glyphs: Uint16Array, pos: Array<CkPoint>, top: number,
                  bottom: number, paint: null | CkPaint): Float32Array;

    getPath(glyph: number): null | CkPath;
}

export class CkFontStyleSet {
    private constructor();

    count(): number;

    getStyle(index: number): CkFontStyle;

    getStyleName(index: number): CkFontStyle;

    createTypeface(index: number): CkTypeface | null;

    matchStyle(pattern: CkFontStyle): CkTypeface | null;
}

export class CkFontMgr {
    private constructor();

    countFamilies(): number;

    getFamilyName(index: number): string;

    createStyleSet(index: number): CkFontStyleSet;

    matchFamilyStyle(familyName: string | null, style: CkFontStyle): CkTypeface | null;
}

export const defaultFontMgr: CkFontMgr;

export interface CkRSXform {
    ssin: number;
    scos: number;
    tx: number;
    ty: number;
}

export class CkTextBlob {
    private constructor();

    static MakeFromText(text: Uint8Array,
                        font: CkFont,
                        encoding: TextEncoding): CkTextBlob;

    static MakeFromPosTextH(text: Uint8Array,
                            xpos: Float32Array,
                            constY: number,
                            font: CkFont,
                            encoding: TextEncoding): CkTextBlob;

    static MakeFromPosText(text: Uint8Array,
                           pos: Array<CkPoint>,
                           font: CkFont,
                           encoding: TextEncoding): CkTextBlob;

    static MakeFromRSXformText(text: Uint8Array,
                               forms: Array<CkRSXform>,
                               font: CkFont,
                               encoding: TextEncoding): CkTextBlob;

    readonly bounds: CkRect;
    readonly uniqueID: number;

    getIntercepts(upperBound: number, lowerBound: number, paint: null | CkPaint): Float32Array;
}

export interface CanvasSaveLayerRec {
    bounds: null | CkRect;
    paint: null | CkPaint;
    backdrop: null | CkImageFilter;
    flags: CanvasSaveLayerFlag;
}

export class CkCanvas {
    protected constructor();

    save(): number;

    saveLayer(bounds: null | CkRect, paint: null | CkPaint): number;

    saveLayerAlpha(bounds: null | CkRect, alpha: number): number;

    saveLayerRec(rec: CanvasSaveLayerRec): number;

    restore(): void;

    restoreToCount(saveCount: number): void;

    translate(dx: number, dy: number): void;

    scale(sx: number, sy: number): void;

    rotate(rad: number, px: number, py: number): void;

    skew(sx: number, sy: number): void;

    concat(matrix: CkMatrix): void;

    setMatrix(matrix: CkMatrix): void;

    resetMatrix(): void;

    clipRect(rect: CkRect, op: ClipOp, AA: boolean): void;

    clipRRect(rrect: CkRRect, op: ClipOp, AA: boolean): void;

    clipPath(path: CkPath, op: ClipOp, AA: boolean): void;

    clipShader(shader: CkShader, op: ClipOp): void;

    quickRejectRect(rect: CkRect): boolean;

    quickRejectPath(path: CkPath): boolean;

    getLocalClipBounds(): CkRect

    getDeviceClipBounds(): CkRect

    drawColor(color: CkColor4f, mode: BlendMode): void;

    clear(color: CkColor4f): void;

    drawPaint(paint: CkPaint): void;

    drawPoints(mode: CanvasPointMode, points: Array<CkPoint>, paint: CkPaint): void;

    drawPoint(x: number, y: number, paint: CkPaint): void;

    drawLine(p1: CkPoint, p2: CkPoint, paint: CkPaint): void;

    drawRect(rect: CkRect, paint: CkPaint): void;

    drawOval(oval: CkRect, paint: CkPaint): void;

    drawRRect(rrect: CkRRect, paint: CkPaint): void;

    drawDRRect(outer: CkRRect, inner: CkRRect, paint: CkPaint): void;

    drawCircle(cx: number, cy: number, r: number, paint: CkPaint): void;

    drawArc(oval: CkRect, startAngle: number, sweepAngle: number, useCenter: boolean, paint: CkPaint): void;

    drawRoundRect(rect: CkRect, rx: number, ry: number, paint: CkPaint): void;

    drawPath(path: CkPath, paint: CkPaint): void;

    drawImage(image: CkImage, left: number, top: number, sampling: SamplingOption, paint: null | CkPaint): void;

    drawImageRect(image: CkImage, src: CkRect, dst: CkRect, sampling: SamplingOption, paint: null | CkPaint,
                  constraint: CanvasSrcRectConstraint): void;

    drawString(str: string, x: number, y: number, font: CkFont, paint: CkPaint): void;

    drawGlyphs(glyphs: Uint16Array, positions: Array<CkPoint>, origin: CkPoint, font: CkFont, paint: CkPaint): void;

    drawTextBlob(blob: CkTextBlob, x: number, y: number, paint: CkPaint): void;

    drawPicture(picture: CkPicture, matrix: null | CkMatrix, paint: null | CkPaint): void;

    drawVertices(vertices: CkVertices, mode: BlendMode, paint: CkPaint): void;

    drawPatch(cubics: Array<CkPoint>,
              colors: Array<CkColor4f> | null,
              texCoords: Array<CkPoint> | null,
              mode: BlendMode, paint: CkPaint): void;
}

export class CkPathMeasure {
    private constructor();

    static Make(path: CkPath, forceClosed: boolean, resScale: number): CkPathMeasure;

    getLength(): number;

    isClosed(): boolean;

    nextContour(): boolean;

    getPositionTangent(distance: number): { position: CkPoint, tangent: CkPoint } | null;

    getMatrix(distance: number, flags: PathMeasureMatrixFlags): CkMatrix | null;

    getSegment(startD: number, stopD: number, startWithMoveTo: boolean): CkPath | null;
}

export class CkVertices {
    private constructor();

    static MakeCopy(mode: VerticesVertexMode,
                    positions: Float32Array,
                    texCoords: Float32Array | null,
                    colors: Uint32Array | null,
                    indices: Uint16Array | null): CkVertices;

    static MakeTransform(mode: VerticesVertexMode,
                         positions: Float32Array,
                         positionsMatrix: CkMatrix,
                         texCoords: Float32Array | null,
                         texCoordsMatrix: CkMatrix | null,
                         colors: Uint32Array | null,
                         indices: Uint16Array | null): Promise<CkVertices>;

    readonly uniqueID: number;
    readonly bounds: CkArrayXYWHRect;
}

export class CkImageFilter {
    static MakeFromDSL(dsl: string, kwargs: object): CkImageFilter;

    static Deserialize(buffer: TypedArray): CkImageFilter;

    serialize(): ArrayBuffer;
}

export class CkColorFilter {
    static MakeFromDSL(dsl: string, kwargs: object): CkColorFilter;

    static Deserialize(buffer: TypedArray): CkColorFilter;

    serialize(): ArrayBuffer;
}

export class CkShader {
    static MakeFromDSL(dsl: string, kwargs: object): CkShader;

    makeWithLocalMatrix(matrix: CkMatrix): CkShader;

    makeWithColorFilter(filter: CkColorFilter): CkShader;
}

export class CkBlender {
    static Mode(mode: BlendMode): CkBlender;

    static Arithmetic(k1: number, k2: number, k3: number, k4: number, enforcePM: boolean): CkBlender;
}

export interface RTEffectUniform {
    name: string;
    offset: number;
    type: RuntimeEffectUniformType;
    count: number;
    flags: RuntimeEffectUniformFlag;
    sizeInBytes: number;
}

export interface RTEffectChild {
    name: string;
    type: RuntimeEffectChildType;
    index: number;
}

export interface RTEffectChildSpecifier {
    shader?: CkShader;
    blender?: CkBlender;
    colorFilter?: CkColorFilter;
}

export class CkRuntimeEffect {
    private constructor();

    static MakeForColorFilter(source: string, forceUnoptimized: boolean,
                              callback: (error: string) => void): CkRuntimeEffect | null;

    static MakeForShader(source: string, forceUnoptimized: boolean,
                         callback: (error: string) => void): CkRuntimeEffect | null;

    static MakeForBlender(source: string, forceUnoptimized: boolean,
                          callback: (error: string) => void): CkRuntimeEffect | null;

    uniforms(): Array<RTEffectUniform>;

    children(): Array<RTEffectChild>;

    findUniform(name: string): RTEffectUniform;

    findChild(name: string): RTEffectChild;

    makeShader(uniforms: Array<number>, children: Array<RTEffectChildSpecifier>,
               localMatrix: CkMatrix | null): CkShader | null;

    makeBlender(uniforms: Array<number>, children: Array<RTEffectChildSpecifier>): CkBlender | null;

    makeColorFilter(uniforms: Array<number>, children: Array<RTEffectChildSpecifier>): CkColorFilter | null;
}

export class CkPathEffect {
    private constructor();

    static MakeFromDSL(dsl: string, kwargs: object): CkPathEffect;
}

export class CkBitmap {
    private constructor();

    static MakeFromBuffer(buffer: Uint8Array,
                          width: number,
                          height: number,
                          stride: number,
                          colorType: ColorType,
                          alphaType: AlphaType): CkBitmap;

    static MakeFromEncodedFile(path: string): CkBitmap;

    readonly width: number;
    readonly height: number;
    readonly alphaType: AlphaType;
    readonly colorType: ColorType;
    readonly bytesPerPixel: number;
    readonly rowBytesAsPixels: number;
    readonly shiftPerPixel: number;
    readonly rowBytes: number;
    readonly immutable: boolean;

    setImmutable(): void;

    computeByteSize(): number;

    asImage(): CkImage;

    makeShader(tmx: TileMode, tmy: TileMode, sampling: SamplingOption,
               localMatrix: CkMatrix | null): CkShader;

    asTypedArray(): Uint8Array;
}

export interface FilteredImage {
    image: CkImage;
    offset: CkPoint;
    subset: CkArrayXYWHRect;
}

export class CkImage {
    static MakeFromEncodedData(buffer: Uint8Array): Promise<CkImage>;

    static MakeFromEncodedFile(path: string): Promise<CkImage>;

    static MakeFromVideoBuffer(vbo: VideoBuffer): CkImage;

    static MakeDeferredFromPicture(picture: CkPicture,
                                   width: number,
                                    height: number,
                                    matrix: CkMatrix | null,
                                    paint: CkPaint | null,
                                    bitDepth: ImageBitDepth,
                                    colorSpace: ColorSpace): CkImage;

    static MakeFromMemory(buffer: TypedArray,
                         info: CkImageInfo,
                         rowBytes: number,
                         sharedPixelMemory: boolean): CkImage;

    static MakeFromCompressedTextureData(data: TypedArray,
                                         width: number,
                                         height: number,
                                         type: TextureCompressionType): CkImage;
    readonly width: number;
    readonly height: number;
    readonly alphaType: number;
    readonly colorType: number;

    uniqueId(): number;

    dispose(): void;

    isDisposed(): boolean;

    hasMipmaps(): boolean;

    withDefaultMipmaps(): CkImage;

    isTextureBacked(): boolean;

    approximateTextureSize(): number;

    isValid(context: GpuDirectContext | null): boolean;

    makeNonTextureImage(context: GpuDirectContext | null): CkImage;

    makeRasterImage(context: GpuDirectContext | null): CkImage;

    makeWithFilter(context: GpuDirectContext | null,
                   filter: CkImageFilter,
                   subset: CkRect,
                   clipBounds: CkRect): FilteredImage;

    peekPixels<T>(scopeCallback: (pixmap: CkPixmap) => T): T;

    readPixels(dstInfo: CkImageInfo,
               dstBuffer: TypedArray,
               dstRowBytes: number,
               srcX: number,
               srcY: number): void;

    scalePixels(dstInfo: CkImageInfo,
                dstBuffer: TypedArray,
                dstRowBytes: number,
                sampling: SamplingOption): void;

    makeSubset(context: GpuDirectContext | null, subset: CkRect): CkImage;

    makeShader(tmx: TileMode,
               tmy: TileMode,
               sampling: SamplingOption,
               local_matrix: CkMatrix | null): CkShader | null;

    makeRawShader(tmx: TileMode,
                  tmy: TileMode,
                  sampling: SamplingOption,
                  local_matrix: CkMatrix | null): CkShader | null;
}

export class CkPicture {
    static MakeFromData(buffer: TypedArray): CkPicture;

    static MakeFromFile(path: string): CkPicture;

    serialize(): ArrayBuffer;

    approximateOpCount(nested: boolean): number;

    approximateByteUsed(): number;

    uniqueId(): number;
}

export class CkPictureRecorder {
    constructor();

    beginRecording(bounds: CkRect): CkCanvas;

    getRecordingCanvas(): CkCanvas | null;

    finishRecordingAsPicture(): CkPicture | null;

    finishRecordingAsPictureWithCull(cull: CkRect): CkPicture | null;
}

export class CriticalPicture {
    private constructor();

    sanitize(): Promise<CkPicture>;

    serialize(): Promise<ArrayBuffer>;

    discardOwnership(): void;

    setCollectionCallback(F: () => void): void;
}

export class VertexBatch {
    private constructor();
}

export class VertexBatchBuilder {
    constructor();

    pushPositionMatrix(matrix: CkMatrix): VertexBatchBuilder;

    pushTexCoordMatrix(matrix: CkMatrix): VertexBatchBuilder;

    popPositionMatrix(): VertexBatchBuilder;

    popTexCoordMatrix(): VertexBatchBuilder;

    addVertexGroup(positions: Float32Array, texCoords: Float32Array | null): VertexBatchBuilder;

    build(): VertexBatch;
}

export interface VertexTransformResult {
    positions: Float32Array;
    texCoords: Float32Array | null;
}

export class ConcurrentVertexProcessor {
    constructor(vertexCountHint: number, uvCountHint: number);

    transform(batch: VertexBatch): Promise<Array<VertexTransformResult>>;
}
