import {Buffer} from 'core';

export interface RenderClientError extends Error {
    opcode: number;
}

export interface ApplicationInfo {
    name: string;
    major: number;
    minor: number;
    patch: number;
}

export class RenderHost {
    public static Initialize(info: ApplicationInfo): void;
    public static Dispose(): void;

    public static Connect(name?: string): Promise<Display>;
    public static MakeBlender(surface: Surface): Promise<Blender>;
}

type SlotID = number;
export class RenderClientObject {
    public connect(signal: string, slot: Function): SlotID;
    public disconnect(id: SlotID): void;
    public inspectObject(): object;
}

export class Display extends RenderClientObject {
    public close(): Promise<void>;
    public createRasterSurface(w: number, h: number): Promise<Surface>;
    public createHWComposeSurface(w: number, h: number): Promise<Surface>;
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

    public close(): Promise<void>;
    public setTitle(title: string): Promise<void>;
    public resize(width: number, height: number): Promise<void>;

    public getBufferDescriptor(): Promise<string>;
    public requestNextFrame(): Promise<void>;
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

// =========================================================
// GskIR (Graphic Scene Kit Intermediate Representation)
// Toolchain
// =========================================================

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

export interface MoeExternalBreakpointHandlers {
    debugCallback: (breakpointId: number) => void;
    profilingCallback: (breakpointId: number) => void;
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
                              breakpointHandlers: MoeExternalBreakpointHandlers,
                              heapProfiling: boolean): MoeTranslationResult;
    public static Disassemble(buffers: Array<Buffer>): string;
}
