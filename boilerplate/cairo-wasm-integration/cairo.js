import * as std from 'core';

const wasm = std.File.ReadFileSync('./out/CairoBindings.wasm');

global['window'] = global;
global['fetch'] = () => {
    return Promise.resolve({
        ok: true,
        arrayBuffer() {
            return Promise.resolve(wasm.byteArray.buffer);
        }
    });
};

const mod = await import('./out/CairoBindings.js');
const module = await mod.default({
    // As `URL` constructor does not exist in Cocoa,
    // we should provide a dummy `locateFile` to prevent
    // Emscripten resolving WASM's URL by `new URL(...)`.
    locateFile() {
        return '<file>';
    }
});

delete global['window'];
delete global['fetch'];

// Memory management

module.Malloc = function(typedArrayCtor, length) {
    const byteLength = length * typedArrayCtor.BYTES_PER_ELEMENT;

    // `_malloc` function was exported by linking option `-sEXPORTED_FUNCTIONS=...`,
    // and it is implemented by Emscripten itself. We call this function to
    // allocate memory on the WASM heap so that the allocated memory can be shared
    // with JavaScript (JavaScript can access the whole memory space of WASM, but WASM
    // cannot access JavaScript heap freely).
    const ptr = module._malloc(byteLength);

    return {
        // To indicate that it is allocated by `Malloc`
        __wasm_heap_mem: true,
        __wasm_heap_ptr: ptr,
        __typed_array: null,

        length: length,
        byteOffset: ptr,

        subarray: function (start, end) {
            const sa = this.toTypedArray().subarray(start, end);
            sa['__wasm_heap_mem'] = true;
            return sa;
        },

        toTypedArray: function() {
            if (this.__typed_array && this.__typed_array.length) {
                return this.__typed_array;
            }
            this.__typed_array = new typedArrayCtor(module.HEAPU8.buffer, ptr, length);
            // Also add a marker that this was allocated by `Malloc`
            this.__typed_array['__wasm_heap_mem'] = true;
            return this.__typed_array;
        }
    };
}

module.Free = function(memory) {
    module._free(memory.byteOffset);
    memory.byteOffset = null;
    memory.subarray = null;
    memory.toTypedArray = null;
    memory.__typed_array = null;
    memory.__wasm_heap_mem = false;
    memory.__wasm_heap_ptr = null;
}

export default module;
