type SizeT = number;
type OffsetT = number;

/**
 * An argument list (Escapable Arguments) specified by `--escapable-args`
 * and `--escapable-args-delimiter`.
 * Stays empty if not specify explicitly (Never be null or undefined).
 */
export declare const args: string[];

/**
 * Prints `str` to standard output (without '\n').
 * @param str String content to print.
 */
export declare function print(str: string): void;

/**
 * The promise will be resolved after `timeout` ms.
 */
export declare function delay(timeout: number): Promise<void>;

export interface PropertyPair {
    readonly type: "data" | "object" | "array";
    readonly value?: number | boolean | string | bigint;
}
export declare function getProperty(specifier: string): PropertyPair;

export interface PropertyChild {
    readonly type: "data" | "object" | "array";
    /* Name of the node. This can be an index number
       with a "#" prefix if property node is an array. */
    readonly name: string;
    // index is available if property node is an array
    readonly index?: number;
}
export declare function enumeratePropertyNode(specifier: string): PropertyChild[];

/**
 * Determines whether the property that is specified by `specifier`
 * exists and is accessible.
 */
export declare function hasProperty(specifier: string): boolean;


/**
 * Exit immediately without waiting for the event loop.
 * This will not cause Cocoa to crash but print a debug-level log
 * which reports the call of `exit()`.
 */
export declare function exit(): void;

/**
 * A reusable and high-resolution (microsecond) timer.
 */
export declare class TimerProxy {
    /**
     * Waits for `timeout` ms, then call `callback` every `interval` ms.
     * Throws an exception if `timeout` or `interval` is negative.
     * When `interval` is zero, this acts the same as `setTimeout()`.
     */
    setInterval(timeout: number, interval: number, callback: () => void|boolean): void;

    /**
     * Waits for `timeout` ms, then call `callback` once.
     * Throws an exception if `timeout` is negative.
     */
    setTimeout(timeout: number, callback: () => void): void;

    /**
     * Interrupt a pending call and reset the timer.
     */
    stop(): void;
}

type BufferEncoding = 'ascii' | 'latin1' | 'utf8' | 'ucs2';
export declare class Buffer {
    readonly length: number;

    constructor(str: string, encoding: BufferEncoding);
    constructor(length: number);

    byteAt(index: number): number;
    copy(offset?: OffsetT, length?: SizeT): Buffer;
    toDataView(offset?: OffsetT, length?: SizeT): DataView;
}

type SeekWhenceT = number;
type FileModeT = number;

export declare class FileHandle {
    static readonly MODE_NONE: FileModeT;
    static readonly MODE_USR_W: FileModeT;
    static readonly MODE_USR_R: FileModeT;
    static readonly MODE_USR_X: FileModeT;
    static readonly MODE_OTH_W: FileModeT;
    static readonly MODE_OTH_R: FileModeT;
    static readonly MODE_OTH_X: FileModeT;
    static readonly MODE_GRP_R: FileModeT;
    static readonly MODE_GRP_W: FileModeT;
    static readonly MODE_GRP_X: FileModeT;
    static readonly MODE_DIR: FileModeT;
    static readonly MODE_LINK: FileModeT;
    static readonly MODE_REGULAR: FileModeT;
    static readonly MODE_CHAR: FileModeT;
    static readonly MODE_BLOCK: FileModeT;
    static readonly MODE_FIFO: FileModeT;
    static readonly MODE_SOCKET: FileModeT;
    static readonly SEEK_SET: SeekWhenceT;
    static readonly SEEK_CURRENT: SeekWhenceT;
    static readonly SEEK_END: SeekWhenceT;

    constructor(path: string, flags?: string, mode?: FileModeT);

    close(): void;
    read(dstBuffer: DataView, size: SizeT): SizeT;
    write(srcBuffer: DataView, size: SizeT): SizeT;
    seek(whence: SeekWhenceT, offset: SizeT): SizeT;
}

export declare const __name__: string;
export declare const __desc__: string;
export declare const __unique_id__: string;
