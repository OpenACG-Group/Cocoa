// %native_script:introspect

/**
 * A Callback function of `scheduleScriptEvaluate` and `scheduleModuleUrlEvaluate`.
 * @param value Result of the evaluation.
 */
type EvalResolvedCb = (value: any) => void;

/**
 * A Callback function of `scheduleScriptEvaluate` and `scheduleModuleUrlEvaluate`.
 * @param except Thrown exception.
 */
type EvalRejectedCb = (except: any) => void;

type UnhandledRejectionHandler<T> = (promise: Promise<T>, value: any) => void;

type MultipleResolveAction = 'resolve' | 'reject';
type MultipleResolveHandler<T> = (promise: Promise<T>, action: MultipleResolveAction) => void;

type JournalLogLevel = 'debug' | 'info' | 'warning' | 'warn' |
                       'error' | 'err' | 'exception' | 'except';

type IntrospectSecurityPolicy = 'AllowLoadingSharedObject' | 'ForbidLoadingSharedObject' |
                                'AllowWritingToJournal' | 'ForbidWritingToJournal';

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
     * Load a dynamic shared object as a language binding.
     * 
     * @param path Path of the dynamic library file.
     *
     * @note Security policy "{Allow|Forbid}LoadingSharedObject" can enable/disable
     *       this action.
     */
    loadSharedObject(path: string): void;

    /**
     * Evaluate a code snippet in the future.
     * The code snippet is treated as a task that will be pushed into task queue,
     * and the task queue will be checked (which means code snippet will be executed)
     * in the preparation stage of event loop.
     * 
     * @param source Code snippet to be executed.
     * @param resolved A callback function that will be called when the code snippet
     *                 finishes executing.
     * @param rejected A callback function that will be called when the code snippet
     *                 throws an uncaught exception.
     * 
     * @note Exceptions thrown by callbacks will be swallowed.
     */
    scheduleScriptEvaluate(source: string, resolved?: EvalResolvedCb, rejected?: EvalRejectedCb): void;

    /**
     * Evaluate a module in the future.
     * The module evaluation is treated as a task that will be pushed into task queue,
     * and the task queue will be checked (which means code snippet will be executed)
     * in the preparation stage of event loop.
     * 
     * @param url A module URL to be evaluated.
     * @param resolved A callback function that will be called when the module
     *                 finishes evaluating.
     * @param rejected A callback function that will be called when the module
     *                 throws an uncaught exception.
     * 
     * @note Exceptions thrown by callbacks will be swallowed.
     */
    scheduleModuleUrlEvaluate(url: string, resolved?: EvalResolvedCb, rejected?: EvalRejectedCb): void;
    
    /**
     * A simple print() implementation that writes contents to stdout
     * directly.
     * @param str What to print. No extra newline will be appended.
     */
    print(str: string): void;

    /**
     * Test whether a synthetic module is available.
     * 
     * @param name Name of the module to be tested.
     * @returns true if the module is available, otherwise false.
     */
    hasSyntheticModule(name: string): boolean;

    /**
     * Write message to Cocoa's log.
     *
     * @param level Message level, messages with some levels may be swallowed
     *              according to current log level.
     * @param message A string to write into log. Decoration syntax is available.
     * 
     * @note Security policy "{Allow|Forbid}WritingToJournal" can enable/disable
     *       this action.
     */
    writeToJournal(level: JournalLogLevel, message: string): void;

    /**
     * Test whether a security policy is enabled.
     * 
     * @param policy A string of policy name to be tested.
     * @returns true if the policy is enabled, otherwise false.
     */
    hasSecurityPolicy(policy: IntrospectSecurityPolicy): boolean;

    /**
     * Get current stacktrace.
     *
     * @param frameLimit Specify the maximum number of stack frames.
     *                   If not specified, a default value which depends on Cocoa's
     *                   command line options will be used.
     * @returns An array containing stack frames.
     */
    inspectStackTrace(frameLimit?: number): Array<StackTraceFrame>;
}

declare let introspect: Introspect;
