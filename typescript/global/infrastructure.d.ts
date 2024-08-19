interface RuntimeInfo {
    version: string;
    implementation: string;
    platform: string;
}

interface GlobalScope {
    global: GlobalScope;

    __runtime__: RuntimeInfo;
    introspect: Introspect;
}

interface WorkerGlobalScope {
    self: WorkerGlobalScope;
    __runtime__: RuntimeInfo;
}

// TODO(sora): make this available for workers
// declare const self: WorkerGlobalScope;
declare const global: GlobalScope;
declare const __runtime__: RuntimeInfo;

/* Global Functions */

/**
 * Creates a timer with a callback function attached on it.
 * After the timer is created, `fn` will be called AT LEAST `timeoutInMs` milliseconds later.
 * After that, if `repeat` is false, the timer will be cleared and `fn` will not be called anymore;
 * if `repeat` is true, `fn` will be called AT LEAST every `timeoutInMs` milliseconds, until the
 * timer is cleared by `clearTimerCallback()`.
 *
 * The timer itself is based on the main event loop, and cannot provide an accurate clock.
 * Each time the callstack of user JavaScript becomes empty, or an asynchronous task is completed,
 * we enter the event loop and wait for new events，for example, the expiration of timers.
 * If our event loop is busy handling other tasks when the timer expires, those tasks will not
 * be interrupted, and the expiration of the timer will NOT be handled until those tasks are
 * finished. That means the callback may be called later than the expiration of the timer.
 *
 * @returns An numeric identifier, which can be used to clear the timer by `clearTimerCallback()`.
 *          For each call, returns a different number.
 */
declare function setTimerCallback(timeoutInMs: number, repeat: boolean, fn: () => void): number;

/**
 * Clears the timer specified by `id`, or throws an error if `id` refers to an invalid timer.
 */
declare function clearTimerCallback(id: number): void;

/**
 * Gets the time elapsed in milliseconds since the JavaScript context was created.
 * For the main thread, JavaScript context is created when the program starts; for a worker thread,
 * JavaScript context is created when that thread starts.
 * The trustable precision is 1e-3 (1.0 nanoseconds).
 */
declare function getMillisecondTimeCounter(): number;

/* Introspect API */

type UnhandledRejectionHandler<T> = (promise: Promise<T>, value: any) => void;

type MultipleResolveAction = 'resolve' | 'reject';
type MultipleResolveHandler<T> = (promise: Promise<T>, action: MultipleResolveAction) => void;

interface StackTraceFrame {
    readonly line: number;          /* -1 if not available */
    readonly column: number;        /* -1 if not available */
    readonly scriptName: string;    /* undefined if not available */
    readonly functionName: string;  /* undefined if not available */
    readonly isEval: boolean;
    readonly isConstructor: boolean;
    readonly isWasm: boolean;
    readonly isUserJavaScript: boolean;
}

interface TracingConfig {
    bufferSizeKb: number;
    enables: Array<string>;
    largeTrace: boolean;
    writeToFile: string;
}

interface Introspect {
    /**
     * Register a callback function for uncaught exception.
     *
     * @param handler A function that will be called when an uncaught exception
     *                is thrown.
     */
    setUncaughtExceptionHandler(handler: (except: any) => void): void;

    /**
     * Register a callback function that will be called when exiting.
     *
     * @param handler A function that will be called when exiting.
     */
    setBeforeExitHandler(handler: () => void): void;

    /**
     * Register a callback function for unhandled promise rejection.
     *
     * @param handler A function that will be called when a promise rejects
     *                but is not handled.
     */
    setUnhandledPromiseRejectionHandler<T>(handler: UnhandledRejectionHandler<T>): void;

    /**
     * Register a callback function for multiple promise resolve.
     *
     * @param handler A function that will be called when a resolved
     *                promise resolves/rejects again.
     */
    setPromiseMultipleResolveHandler<T>(handler: MultipleResolveHandler<T>): void;

    /**
     * Evaluate a code snippet in the future.
     * The code snippet is treated as a task that will be pushed into task queue,
     * and the task queue will be checked (which means code snippet will be executed)
     * in the preparation stage of event loop.
     *
     * @param source Code snippet to be executed.
     */
    scheduleScriptEval(source: string): Promise<void>;

    /**
     * Evaluate a module in the future.
     * The module evaluation is treated as a task that will be pushed into task queue,
     * and the task queue will be checked (which means code snippet will be executed)
     * in the preparation stage of event loop.
     *
     * @param url A module URL to be evaluated.
     */
    scheduleModuleUrlEval(url: string): Promise<void>;

    /**
     * A simple print() implementation that writes contents to stdout directly.
     * @param str What to print. No extra newline will be appended.
     */
    print(str: string): void;

    /**
     * Test whether a native module is available.
     *
     * @param name Name of the module to be tested.
     * @returns true if the module is available, otherwise false.
     */
    hasNativeModule(name: string): boolean;

    /**
     * Get current stacktrace.
     *
     * @param frameLimit Specify the maximum number of stack frames.
     *                   If not specified, a default value which depends on Cocoa's
     *                   command line options will be used.
     * @returns An array containing stack frames.
     */
    rewind(frameLimit: number): Array<StackTraceFrame>;

    startProcessTracing(config: TracingConfig): void;
    finishProcessTracing(): void;
}

declare const introspect: Introspect;
