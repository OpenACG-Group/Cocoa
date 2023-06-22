/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

Module['malloc'] = function(typedArrayCtor, length) {
    const byteLength = length * typedArrayCtor.BYTES_PER_ELEMENT;

    // `_malloc` function was exported by linking option `-sEXPORTED_FUNCTIONS=...`,
    // and it is implemented by Emscripten itself. We call this function to
    // allocate memory on the WASM heap so that the allocated memory can be shared
    // with JavaScript (JavaScript can access the whole memory space of WASM, but WASM
    // cannot access JavaScript heap freely).
    const ptr = Module._malloc(byteLength);

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
            this.__typed_array = new typedArrayCtor(Module.HEAPU8.buffer, ptr, length);
            // Also add a marker that this was allocated by `Malloc`
            this.__typed_array['__wasm_heap_mem'] = true;
            return this.__typed_array;
        }
    };
}

Module['free'] = function(memory) {
    Module._free(memory.byteOffset);
    memory.byteOffset = null;
    memory.subarray = null;
    memory.toTypedArray = null;
    memory.__typed_array = null;
    memory.__wasm_heap_mem = false;
    memory.__wasm_heap_ptr = null;
}
