/**
 * [[NativeCode]]
 * Members in namespace Cocoa.core are implemented by `core`
 * language binding.
 */

declare namespace Cocoa.core  {
    /* Constants: Virtual File Descriptor (Randomized by FDLR) */
    const VFD_STDIN: number;
    const VFS_STDOUT: number;
    const VFD_STDERR: number;

    /* Constants: File creation modes */
    const MODE_NONE: number;
    const MODE_USR_W: number;
    const MODE_USR_R: number;
    const MODE_USR_X: number;
    const MODE_OTH_W: number;
    const MODE_OTH_R: number;
    const MODE_OTH_X: number;
    const MODE_GRP_R: number;
    const MODE_GRP_W: number;
    const MODE_GRP_X: number;
    const MODE_DIR: number;
    const MODE_LINK: number;
    const MODE_REGULAR: number;
    const MODE_CHAR: number;
    const MODE_BLOCK: number;
    const MODE_FIFO: number;
    const MODE_SOCKET: number;

    /**
     * A reusable and high-resolution (microsecond) timer.
     */
    class Timer {
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

    /**
     * Prints `str` to standard output (without '\n').
     * @param str String content to print.
     */
    function print(str: string): void;

    /**
     * The promise will be resolved after `timeout` ms.
     */
    function delay(timeout: number): Promise<void>;

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
     * (docs/[Users] Core.md)
     */
    function getProperty(specifier: string): any;

    /**
     * Determines whether the property that is specified by `specifier`
     * exists and is accessible.
     */
    function hasProperty(specifier: string): boolean;

    function dump(what: string): void;

    function open(path: string, flags?: string, mode?: number): number;
    function close(vfd: number);

    function exit(): void;
}
