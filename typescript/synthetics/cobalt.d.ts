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
    static Initialize(info: ApplicationInfo): void;
    static Dispose(): void;
    static Connect(name?: string): Promise<Display>;
}

type SlotID = number;
export class RenderClientObject {
    connect(signal: string, slot: Function): SlotID;
    disconnect(id: SlotID): void;
}

export class Display extends RenderClientObject {
    close(): Promise<void>;
    createRasterSurface(w: number, h: number): Promise<Surface>;
    createHWComposeSurface(w: number, h: number): Promise<Surface>;
}

export class Surface extends RenderClientObject {
    readonly width: number;
    readonly height: number;

    close(): Promise<void>;
    setTitle(title: string): Promise<void>;
    resize(width: number, height: number): Promise<void>;

    getBufferDescriptor(): Promise<string>;
}

export class RecordedPicture {
}

export interface VGIRCompileResult {
    hasError: boolean;
    error: string;
    artifact: RecordedPicture;
}

export class VGIRCompiler {
    static Compile(buffers: Array<Buffer>): VGIRCompileResult;
}
