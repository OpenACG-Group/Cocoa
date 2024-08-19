import * as _renderer from 'renderer';
import * as _multimedia from 'multimedia';
import * as _event from 'event';
type i8 = number;
type u8 = number;
type i16 = number;
type u16 = number;
type i32 = number;
type u32 = number;
type i64 = number;
type u64 = number;
type f32 = number;
type f64 = number;
export declare enum MonitorSubpixel {
    Unknown,
    None,
    HorizontalRGB,
    HorizontalBGR,
    VerticalRGB,
    VerticalBGR,
}
export declare enum MonitorTransform {
    Normal,
    Rotate90,
    Rotate180,
    Rotate270,
    Flipped,
    Flipped90,
    Flipped180,
    Flipped270,
}
export declare enum MonitorMode {
    Current,
    Preferred,
}
export declare enum PointerButton {
    Left,
    Right,
    Middle,
    Side,
    Extra,
    Forward,
    Back,
    Task,
}
export declare enum AxisSourceType {
    Wheel,
    WheelTilt,
    Finger,
    Continuous,
    Unknown,
}
export declare enum KeyboardModifiers {
    Control,
    Alt,
    Shift,
    Super,
    CapsLock,
    NumLock,
    Meta,
}
export declare enum KeyboardKey {
    Placeholder,
    Key_SPACE,
    Key_APOSTROPHE,
    Key_COMMA,
    Key_MINUS,
    Key_PERIOD,
    Key_SLASH,
    Key_0,
    Key_1,
    Key_2,
    Key_3,
    Key_4,
    Key_5,
    Key_6,
    Key_7,
    Key_8,
    Key_9,
    Key_SEMICOLON,
    Key_EQUAL,
    Key_A,
    Key_B,
    Key_C,
    Key_D,
    Key_E,
    Key_F,
    Key_G,
    Key_H,
    Key_I,
    Key_J,
    Key_K,
    Key_L,
    Key_M,
    Key_N,
    Key_O,
    Key_P,
    Key_Q,
    Key_R,
    Key_S,
    Key_T,
    Key_U,
    Key_V,
    Key_W,
    Key_X,
    Key_Y,
    Key_Z,
    Key_LEFT_BRACKET,
    Key_BACKSLASH,
    Key_RIGHT_BRACKET,
    Key_GRAVE_ACCENT,
    Key_WORLD_1,
    Key_WORLD_2,
    Key_ESCAPE,
    Key_ENTER,
    Key_TAB,
    Key_BACKSPACE,
    Key_INSERT,
    Key_DELETE,
    Key_RIGHT,
    Key_LEFT,
    Key_DOWN,
    Key_UP,
    Key_PAGE_UP,
    Key_PAGE_DOWN,
    Key_HOME,
    Key_END,
    Key_CAPS_LOCK,
    Key_SCROLL_LOCK,
    Key_NUM_LOCK,
    Key_PRINT_SCREEN,
    Key_PAUSE,
    Key_F1,
    Key_F2,
    Key_F3,
    Key_F4,
    Key_F5,
    Key_F6,
    Key_F7,
    Key_F8,
    Key_F9,
    Key_F10,
    Key_F11,
    Key_F12,
    Key_F13,
    Key_F14,
    Key_F15,
    Key_F16,
    Key_F17,
    Key_F18,
    Key_F19,
    Key_F20,
    Key_F21,
    Key_F22,
    Key_F23,
    Key_F24,
    Key_F25,
    Key_KP_0,
    Key_KP_1,
    Key_KP_2,
    Key_KP_3,
    Key_KP_4,
    Key_KP_5,
    Key_KP_6,
    Key_KP_7,
    Key_KP_8,
    Key_KP_9,
    Key_KP_DECIMAL,
    Key_KP_DIVIDE,
    Key_KP_MULTIPLY,
    Key_KP_SUBTRACT,
    Key_KP_ADD,
    Key_KP_ENTER,
    Key_KP_EQUAL,
    Key_LEFT_SHIFT,
    Key_LEFT_CONTROL,
    Key_LEFT_ALT,
    Key_LEFT_SUPER,
    Key_RIGHT_SHIFT,
    Key_RIGHT_CONTROL,
    Key_RIGHT_ALT,
    Key_RIGHT_SUPER,
    Key_MENU,
}
export declare enum UpdateResult {
    Success,
    FrameDropped,
    Error,
}
export declare enum VideoFrameViewResampler {
    /** Nearest single sample. Fastest but low quality. */
    Nearest,
    /** Bilinear interpolation. Slower but higher quality, best choice for most cases. */
    Bilinear,
    /** Bicubic interpolation. Slowest but high quality. */
    Bicubic,
}
export interface SurfaceCreationOptions {
    enableGpuPipeline?: boolean;
    enableGpuVideoDecodeCompatible?: boolean;
}
export interface MonitorPropertySet {
    logicalX: i32;
    logicalY: i32;
    physicalWidth: i32;
    physicalHeight: i32;
    subpixel: MonitorSubpixel;
    manufactureName: string;
    modelName: string;
    transform: MonitorTransform;
    modeFlags: u32;
    modeWidth: i32;
    modeHeight: i32;
    refreshRate: i32;
    scaleFactor: i32;
    connectorName: string;
    description: string;
}
export class ContentAggregator extends _event.EventEmitterBase {
    private constructor();
    requestImageInfo(): Promise<_renderer.ImageInfo>;
    purgeRasterCacheResources(): Promise<void>;
    update(scene: Scene): Promise<UpdateResult>;
}
export class CursorTheme {
    private constructor();
    dispose(): Promise<void>;
    loadCursor(name: string): Promise<Cursor>;
}
export class Cursor {
    private constructor();
    dispose(): Promise<void>;
    getHotspotVector(): Promise<[i32, i32]>;
}
export class Display extends _event.EventEmitterBase {
    private constructor();
    readonly defaultCursorTheme: CursorTheme;
    close(): Promise<void>;
    requestMonitorList(): Promise<Monitor[]>;
    loadCursorTheme(name: string, size: i32): Promise<CursorTheme>;
    createCursor(pixmap: _renderer.Pixmap, hotspotX: i32, hotspotY: i32): Promise<Cursor>;
    createSurface(width: i32, height: i32, options: SurfaceCreationOptions): Promise<Surface>;
}
export class Monitor extends _event.EventEmitterBase {
    private constructor();
    requestPropertySet(): Promise<void>;
}
export class PresentThread {
    private constructor();
    static Start(): PresentThread;
    dispose(): void;
    collect(): void;
    traceResourcesJSON(): Promise<string>;
    createDisplay(): Promise<Display>;
}
export class Scene {
    private constructor();
    toString(): string;
}
export class SceneBuilder {
    constructor(viewportCull: _renderer.Rect);
    build(): Scene;
    pop(): SceneBuilder;
    pushOffset(x: f32, y: f32): SceneBuilder;
    pushTransform(matrix: _renderer.Mat3x3): SceneBuilder;
    addPicture(picture: _renderer.Picture, clipBounds: boolean): SceneBuilder;
    pushOpacity(alpha: f32): SceneBuilder;
    pushImageFilter(filter: _renderer.ImageFilter): SceneBuilder;
    pushBackdropFilter(filter: _renderer.ImageFilter, blendMode: _renderer.BlendMode, clipChildBounds: boolean): SceneBuilder;
    pushRectClip(shape: _renderer.Rect, antialias: boolean): SceneBuilder;
    pushRRectClip(shape: _renderer.RRect, antialias: boolean): SceneBuilder;
    pushPathClip(shape: _renderer.Path, op: _renderer.ClipOp, antialias: boolean): SceneBuilder;
    addVideoFrameView(frame: _multimedia.Frame, offset: [f32, f32], width: i32, height: i32, sampling: VideoFrameViewResampler): SceneBuilder;
}
export class Surface extends _event.EventEmitterBase {
    private constructor();
    readonly dimensions: [i32, i32];
    readonly display: Display;
    readonly contentAggregator: ContentAggregator;
    close(): Promise<void>;
    setTitle(title: string): Promise<void>;
    resize(width: i32, height: i32): Promise<boolean>;
    requestBufferStateInfo(): Promise<string>;
    requestNextFrame(): Promise<u32>;
    setMaxSize(width: i32, height: i32): Promise<void>;
    setMinSize(width: i32, height: i32): Promise<void>;
    setMaximized(value: boolean): Promise<void>;
    setMinimized(value: boolean): Promise<void>;
    setFullscreen(value: boolean, monitor: Monitor): Promise<void>;
    setAttachedCursor(cursor: Cursor): Promise<void>;
    getVideoDecodeCompatibleDevice(): (null | _multimedia.HWDeviceContext);
}
