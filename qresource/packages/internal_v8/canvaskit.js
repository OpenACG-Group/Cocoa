// %scope UserExecute:forbidden UserImport:allowed SysExecute:allowed

import * as std from 'core';

async function loadCanvasKitWASMWithPolyfill() {
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
    // a simpler replacement of `XMLHttpRequest`.
    global.fetch = (url) => {
        if (url !== PATH_PLACEHOLDER) {
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
    const initializer = global.exports["CanvasKitInit"];

    const result = await initializer({
        locateFile: (file) => {
            return PATH_PLACEHOLDER;
        }
    });

    // Delete our polyfills
    delete global.exports;
    delete global.window;
    delete global.document;
    delete global.fetch;

    return result;
}

// An instance of `CanvasKit` interface. Constructors and factories of other
// objects are accessible through this instance.
export const canvaskit = await loadCanvasKitWASMWithPolyfill();

const typefaceCacheMap = [];

canvaskit.registerRenderableTypeface = function(face) {
    if (!face) {
        throw new TypeError('Invalid Typeface object');
    }

    const key = {
        family: face._getFamilyName(),
        weight: face._getWeightInfo(),
        width: face._getWidthInfo(),
        slant: face._getSlantInfo(),
        _equal(other) {
            return (other.family === this.family &&
                    other.weight === this.weight &&
                    other.width === this.width &&
                    other.slant === this.slant);
        }
    };

    const faceWidthSerialized = {
        typeface: face,
        serialized: null
    };

    typefaceCacheMap.push({ key: key, value: faceWidthSerialized });
    return key;
};

canvaskit.getRenderableTypeface = function (key) {
    for (let pair of typefaceCacheMap) {
        if (pair.key._equal(key)) {
            return pair.value.typeface;
        }
    }
    return null;
};

canvaskit.bindWithInitializedRenderHost = function(renderHost) {
    renderHost.SetTypefaceTransferCallback((info) => {
        if (typeof info.family != 'string' || typeof info.weight != 'number' ||
            typeof info.width != 'number' || typeof info.slant != 'string')
        {
            throw new TypeError('Invalid TypefaceInfo object');
        }

        let foundFace = null;
        for (let pair of typefaceCacheMap) {
            if (pair.key._equal(info)) {
                foundFace = pair.value;
                break;
            }
        }
        if (!foundFace) {
            throw new Error(`Failed to find typeface '${info.family}' in cache map`);
        }

        if (!foundFace.serialized) {
            foundFace.serialized = foundFace.typeface.serialize();
        }

        return foundFace.serialized;
    });
};
