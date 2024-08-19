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

#include "Core/Errors.h"
#include "Gallium/bindings/renderer/BufferUtils.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

namespace {

struct SharedMemRetainer
{
    static std::unique_ptr<SharedMemRetainer>
    New(std::shared_ptr<v8::BackingStore> store) {
        // Use `new` instead of `make_unique` since we will use `delete`
        // operator to release the memory.
        auto *ptr = new SharedMemRetainer{std::move(store)};
        return std::unique_ptr<SharedMemRetainer>(ptr);
    }

    static void Release(const void*, void *ctx) {
        CHECK(ctx);
        auto *retainer = static_cast<SharedMemRetainer*>(ctx);
        delete retainer;
    }

    std::shared_ptr<v8::BackingStore> store;
};

} // namespace anonymous

sk_sp<SkData>
WrapArrayBufferToSkData(v8::Isolate *isolate, v8::Local<v8::ArrayBufferView> view,
                        uint32_t memory_flag, v8::Local<v8::Value> detach_key)
{
    CHECK(isolate && !view.IsEmpty());

    v8::Local<v8::ArrayBuffer> buffer = view->Buffer();
    if (buffer->WasDetached())
        return nullptr;

    if ((memory_flag & kDetachBuffer_MemoryFlag) && !buffer->IsDetachable())
        return nullptr;

    size_t byte_size = view->ByteLength();
    size_t byte_offset = view->ByteOffset();

    sk_sp<SkData> result;
    if (memory_flag & kNoCopy_MemoryFlag)
    {
        auto backing_store = buffer->GetBackingStore();

        // If the buffer is resizable by user JavaScript, it will have a variable
        // memory address, but we never know when the address changes and cannot
        // update the address stored in SkData.
        if (backing_store->IsResizableByUserJavaScript())
            return nullptr;

        auto retainer = SharedMemRetainer::New(backing_store);
        uint8_t *address = static_cast<uint8_t*>(buffer->Data()) + byte_offset;
        result = SkData::MakeWithProc(address, byte_size, SharedMemRetainer::Release, retainer.get());
        CHECK(result);
        (void) retainer.release();
    }
    else
    {
        result = SkData::MakeUninitialized(byte_size);
        CHECK(result);
        CHECK(view->CopyContents(result->writable_data(), byte_size) == byte_size);
    }

    if (memory_flag & kDetachBuffer_MemoryFlag)
    {
        if (buffer->Detach(detach_key).IsNothing())
            return nullptr;
    }

    return result;
}

GALLIUM_BINDINGS_RENDERER_NS_END
