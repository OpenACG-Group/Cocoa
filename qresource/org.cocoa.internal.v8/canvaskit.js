// %scope UserExecute:forbidden UserImport:allowed SysExecute:allowed

import * as std from 'core';

// Polyfill the `window` object to cheat the WASM loader that we are
// in the web environment.
global.window = {};

// Polyfill the `document` object since the WASM loader requires
// `document.currentScript.src` to locate itself.
global.document = {
    currentScript: {
        // We just provide a fake source location here because the
        // WASM loader will not actually use it to locate files.
        src: '<fake-url>'
    }
};

const PATH_PLACEHOLDER = "internal:unused-path";

// Polyfill the `fetch` function by which the WASM loader reads the
// filesystem. Although the specification says that the `fetch` function
// should not be used to fetch filesystem data, we still use it as
// an simpler replacement of `XMLHttpRequest`.
global.fetch = (url) => {
    if (url != PATH_PLACEHOLDER) {
        return Promise.reject("Invalid path for polyfill implementation. " +
                              "This is an internal error and should be reported to the developers.");
    }

    // Load the WASM file from the crpkg package
    let buffer = std.Buffer.MakeFromPackageFile("org.cocoa.internal.v8", "/canvaskit/canvaskit.wasm");

    // Resolve the promise with an emulated `Response` object.
    return Promise.resolve({
        ok: true,
        arrayBuffer: () => {
            return buffer.byteArray;
        }
    });
};

global.exports = {};

await import('internal:///canvaskit/canvaskit.js');
const _CanvasKitInit = global.exports["CanvasKitInit"];

const _CanvasKit = await _CanvasKitInit({
    locateFile: (file) => {
        return PATH_PLACEHOLDER;
    }
});

// Delete our polyfills
delete global.exports;
delete global.window;
delete global.document;
delete global.fetch;

// An instance of `CanvasKit` interface. Constructors and factories of other
// objects are accessible through this instance.
export const canvaskit = _CanvasKit;
