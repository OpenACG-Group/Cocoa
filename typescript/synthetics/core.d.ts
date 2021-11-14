/**
 * Standard Virtual File Descriptor.
 * Following three file descriptors always change every run, which is not
 * predictable.
 */
export declare const VFD_STDIN: number;
export declare const VFD_STDOUT: number;
export declare const VFD_STDERR: number;

/* File creation modes */
export declare const MODE_NONE: number;
export declare const MODE_USR_W: number;
export declare const MODE_USR_R: number;
export declare const MODE_USR_X: number;
export declare const MODE_OTH_W: number;
export declare const MODE_OTH_R: number;
export declare const MODE_OTH_X: number;
export declare const MODE_GRP_R: number;
export declare const MODE_GRP_W: number;
export declare const MODE_GRP_X: number;
export declare const MODE_DIR: number;
export declare const MODE_LINK: number;
export declare const MODE_REGULAR: number;
export declare const MODE_CHAR: number;
export declare const MODE_BLOCK: number;
export declare const MODE_FIFO: number;
export declare const MODE_SOCKET: number;

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

/**
 * Gets the value of a system property.
 * Returns a string, number or boolean (they are called primitive types).
 * Throws an exception if `specifier` refers to an invalid or inaccessible
 * property.
 *
 * Properties are always access-restricted. A property can be public,
 * protected or private. Public properties are accessible to anyone
 * (command line and JavaScript), but protected properties only can be
 * accessed by JavaScript. Private properties are inaccessible to
 * JavaScript. They are only exposed to Cocoa internally.
 *
 * About syntax of property specifier, see Cocoa documents.
 */
export interface PropertyPrimitive {
    readonly type: "data" | "object" | "array";
    readonly value?: number | boolean | string | bigint;
}
export declare function getProperty(specifier: string): PropertyPrimitive;

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

type DumpableLiteral = "descriptors-info" | "event-loop" | "properties";
export declare function dump(what: DumpableLiteral): void;

/**
 * Open a file specified by `path` with `flags`. If the file doesn't
 * exist and "create" flag is specified, creates the file with
 * `mode`.
 * @param path  File path
 * @param flags A combination string of "r" (readonly), "w" (write-only),
 *              "+" (create), "a" (append), "t" (truncate)
 * @param mode  Specify file type and permission (MODE_* | MODE_* | ...)
 * @return An integer of randomized virtual file descriptor (VFD), which is unique
 *         for each opened files. Negative value if an error occurs.
 */
export declare function open(path: string, flags?: string, mode?: number): number;

/**
 * Close the virtual file descriptor specified by `vfd`.
 * @param vfd Virtual file descriptor
 * @throws `Error` if `vfd` isn't a valid file descriptor or the file descriptor
 *         is not closable.
 */
export declare function close(vfd: number): void;

/**
 * Exit immediately without waiting for the event loop.
 * This will not cause Cocoa to crash but print a debug-level log
 * which reports the call of `exit()`.
 */
export declare function exit(): void;

/**
 * A reusable and high-resolution (microsecond) timer.
 */
export declare class Timer {
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

export declare const __name__: string;
export declare const __desc__: string;
export declare const __unique_id__: string;
