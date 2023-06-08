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

#ifndef COCOA_GALLIUM_BINDER_TYPETRAITS_H
#define COCOA_GALLIUM_BINDER_TYPETRAITS_H

#include <optional>
#include <type_traits>

#include "include/v8.h"
#include "Gallium/Gallium.h"
GALLIUM_BINDER_NS_BEGIN

template<typename T>
bool IsSome(v8::Local<v8::Value> value, T* = nullptr)
{
    static_assert(std::is_base_of<v8::Data, T>::value, "Typecheck failed");
    return false;
}

#define IS_SOME_SPECIALIZE(type)                                    \
    template<> inline bool IsSome(v8::Local<v8::Value> value, v8::type*) { \
        return value->Is##type();                                   \
    }

IS_SOME_SPECIALIZE(Name)
IS_SOME_SPECIALIZE(String)
IS_SOME_SPECIALIZE(Symbol)
IS_SOME_SPECIALIZE(Function)
IS_SOME_SPECIALIZE(Array)
IS_SOME_SPECIALIZE(Object)
IS_SOME_SPECIALIZE(BigInt)
IS_SOME_SPECIALIZE(Boolean)
IS_SOME_SPECIALIZE(Number)
IS_SOME_SPECIALIZE(External)
IS_SOME_SPECIALIZE(Int32)
IS_SOME_SPECIALIZE(Uint32)
IS_SOME_SPECIALIZE(Date)
IS_SOME_SPECIALIZE(BigIntObject)
IS_SOME_SPECIALIZE(BooleanObject)
IS_SOME_SPECIALIZE(NumberObject)
IS_SOME_SPECIALIZE(StringObject)
IS_SOME_SPECIALIZE(SymbolObject)
IS_SOME_SPECIALIZE(RegExp)
IS_SOME_SPECIALIZE(Promise)
IS_SOME_SPECIALIZE(Map)
IS_SOME_SPECIALIZE(Set)
IS_SOME_SPECIALIZE(ArrayBuffer)
IS_SOME_SPECIALIZE(ArrayBufferView)
IS_SOME_SPECIALIZE(TypedArray)
IS_SOME_SPECIALIZE(Uint8Array)
IS_SOME_SPECIALIZE(Uint8ClampedArray)
IS_SOME_SPECIALIZE(Int8Array)
IS_SOME_SPECIALIZE(Uint16Array)
IS_SOME_SPECIALIZE(Int16Array)
IS_SOME_SPECIALIZE(Uint32Array)
IS_SOME_SPECIALIZE(Int32Array)
IS_SOME_SPECIALIZE(Float32Array)
IS_SOME_SPECIALIZE(Float64Array)
IS_SOME_SPECIALIZE(BigInt64Array)
IS_SOME_SPECIALIZE(BigUint64Array)
IS_SOME_SPECIALIZE(DataView)
IS_SOME_SPECIALIZE(SharedArrayBuffer)
IS_SOME_SPECIALIZE(Proxy)
IS_SOME_SPECIALIZE(WasmMemoryObject)
IS_SOME_SPECIALIZE(WasmModuleObject)

// IS_SOME_SPECIALIZE(Module)
// IS_SOME_SPECIALIZE(FixedArray)
// IS_SOME_SPECIALIZE(Private)
// IS_SOME_SPECIALIZE(ObjectTemplate)
// IS_SOME_SPECIALIZE(FunctionTemplate)
// IS_SOME_SPECIALIZE(Context)

#undef IS_SOME_SPECIALIZE

template<typename T>
struct TypedArrayMemory
{
    v8::Local<T> array                        = {};
    std::shared_ptr<v8::BackingStore> memory  = {};
    uint8_t *ptr                              = nullptr;
    size_t byte_size                          = 0;
    size_t size                               = 0;
    size_t byte_offset                        = 0;
};

template<typename T>
std::optional<TypedArrayMemory<T>> GetTypedArrayMemory(v8::Local<v8::Value> v)
{
    if (!binder::IsSome<T>(v))
        return {};
    v8::Local<T> array = v8::Local<T>::Cast(v);
    if (!array->HasBuffer())
        return {};
    v8::Local<v8::ArrayBuffer> ab = array->Buffer();
    TypedArrayMemory<T> res;
    res.array = array;
    res.memory = ab->GetBackingStore();
    res.byte_size = array->ByteLength();
    res.size = array->Length();
    res.byte_offset = array->ByteOffset();
    res.ptr = reinterpret_cast<uint8_t*>(res.memory->Data()) + res.byte_offset;
    return std::make_optional<TypedArrayMemory<T>>(res);
}

template<typename SmartPtr>
struct SmartPtrMemoryHolder
{
    SmartPtr owned;
};

template<typename SmartPtr>
std::shared_ptr<v8::BackingStore>
CreateBackingStoreFromSmartPtrMemory(SmartPtr owned, void *data, size_t size)
{
    using TypedHolder = SmartPtrMemoryHolder<SmartPtr>;

    auto deleter = +[](void *, size_t, void *closure) {
        // Destruct the `owned` object to release the smart pointer
        delete reinterpret_cast<TypedHolder*>(closure);
    };

    auto *closure = new TypedHolder{ owned };
    return v8::ArrayBuffer::NewBackingStore(data, size, deleter, closure);
}

GALLIUM_BINDER_NS_END
#endif //COCOA_GALLIUM_BINDER_TYPETRAITS_H
