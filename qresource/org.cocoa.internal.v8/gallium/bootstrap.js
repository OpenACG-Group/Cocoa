// %scope UserExecute:forbidden UserImport:forbidden SysExecute:allowed

import * as std from 'core';

const BOOTSTRAP_ID = 'Gallium Standard Bootstrap Script';
const BOOTSTRAP_VERSION = '1.0.0-alpha';

global.getGalliumRuntimeInfo = function() {
    return {
        implementation: __runtime__.implementation,
        platform: __runtime__.platform,
        version: __runtime__.version,
        bootstrapVersion: BOOTSTRAP_VERSION,
        bootstrapId: BOOTSTRAP_ID
    };
};

function printErrorObject(error) {
    if (!error.hasOwnProperty('stack')) {
        std.print(`${error}\n`);
        return;
    }

    std.print(`${error.stack}\n`);
}

function bootstrapDefaultUncaughtExceptionHandler(error) {
    printErrorObject(error);
}

function bootstrapDefaultPromiseRejectionHandler(promise, value) {
    std.print('Promise reject: ');
    if (value instanceof Error) {
        printErrorObject(value);
    } else {
        std.print(`${value}\n`);
    }
}

global.useBootstrapDefaultHandlers = function() {
    introspect.setUncaughtExceptionHandler(bootstrapDefaultUncaughtExceptionHandler);
    introspect.setUnhandledPromiseRejectionHandler(bootstrapDefaultPromiseRejectionHandler);
};

useBootstrapDefaultHandlers();

/* Web JavaScript polyfills */

// Polyfill for `performance.now()` to measure time in milliseconds.
// Some internal libraries like CanvasKit (animation) depend on that.
global.performance = {
    now: () => {
        // Cocoa's implementation (infrastructure module) to measure time in milliseconds.
        return getMillisecondTimeCounter();
    }
};
