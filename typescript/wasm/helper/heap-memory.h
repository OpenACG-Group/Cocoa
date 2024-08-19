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

#ifndef WASM_HELPER_HEAP_MEMORY_H
#define WASM_HELPER_HEAP_MEMORY_H

#include <emscripten/val.h>

namespace wasmhelper {

/**
 * Help to receive a memory buffer allocated by JavaScript using `malloc` API.
 * The corresponding malloc API for JavaScript is implemented in `heap-memory.js`.
 */
class HeapMemory
{
public:
    explicit HeapMemory(const emscripten::val& heap_mem)
    {
        if (!heap_mem["__wasm_heap_mem"].as<bool>())
            throw std::runtime_error("Memory is not allocated from WASM heap");
        
        uintptr_t ptr = heap_mem["__wasm_heap_ptr"].as<uintptr_t>();
        heap_ptr_ = reinterpret_cast<uint8_t*>(ptr);
        heap_length_ = heap_mem["length"].as<size_t>();
    }

    ~HeapMemory() = default;

    inline size_t GetLength() const {
        return heap_length_;
    }

    inline void *GetPtr() const {
        return heap_ptr_;
    }

    inline uint8_t *GetU8Ptr() const {
        return heap_ptr_;
    }

private:
    uint8_t *heap_ptr_;
    size_t   heap_length_;
};

} // namespace wasmhelper

#endif // WASM_HELPER_HEAP_MEMORY_H
