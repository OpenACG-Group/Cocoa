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

#ifndef COCOA_GALLIUM_FFI_ARRAYBUFFER_H
#define COCOA_GALLIUM_FFI_ARRAYBUFFER_H

#include <type_traits>

#include "Gallium/Gallium.h"
#include "Gallium/ffi/DataTypes.h"
#include "Gallium/ffi/ReturnValue.h"
GALLIUM_FFI_NS_BEGIN

template<typename T>
struct as_arrbuf_type;

template<>
struct as_arrbuf_type<int8_t> { using type = v8::Int8Array; };

template<>
struct as_arrbuf_type<uint8_t> { using type = v8::Uint8Array; };

template<>
struct as_arrbuf_type<int16_t> { using type = v8::Int16Array; };

template<>
struct as_arrbuf_type<uint16_t> { using type = v8::Uint16Array; };

template<>
struct as_arrbuf_type<int32_t> { using type = v8::Int32Array; };

template<>
struct as_arrbuf_type<uint32_t> { using type = v8::Uint32Array; };

template<>
struct as_arrbuf_type<int64_t> { using type = v8::BigInt64Array; };

template<>
struct as_arrbuf_type<uint64_t> { using type = v8::BigUint64Array; };

template<>
struct as_arrbuf_type<float> { using type = v8::Float32Array; };

template<>
struct as_arrbuf_type<double> { using type = v8::Float64Array; };

template<typename T>
class Mem
{
public:
    static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value,
            "`T` must be an integral type in `Mem<T>`");

    using TArray = typename as_arrbuf_type<T>::type;

    template<typename SmartPtrLike>
    struct SmartPtrLikeMemRetainer
    {
        SmartPtrLike owned;
    };

    Mem()
        : ptr_(nullptr)
        , byte_size_(0)
        , size_(0)
        , byte_offset_(0) {}

    Mem(Local<TArray> typed_array,
        std::shared_ptr<v8::BackingStore> backing_store,
        uint8_t *ptr,
        size_t byte_size,
        size_t size_,
        size_t byte_offset)
        : typed_array_(typed_array)
        , backing_store_(std::move(backing_store))
        , ptr_(ptr)
        , byte_size_(byte_size)
        , size_(size_)
        , byte_offset_(byte_offset) {}

    template<typename SmartPtrLike>
    Mem(Isolate *isolate, SmartPtrLike owned, void *data, size_t size);

    g_nodiscard Local<TArray> TypedArray() const {
        return typed_array_;
    }

    g_nodiscard std::shared_ptr<v8::BackingStore> BackingStore() const {
        return backing_store_;
    }

    g_nodiscard T *Address() const {
        return reinterpret_cast<T*>(ptr_);
    }

    T& operator[](size_t idx) const {
        CHECK(idx < size_);
        return (reinterpret_cast<T*>(ptr_))[idx];
    }

    g_nodiscard size_t ByteSize() const {
        return byte_size_;
    }

    g_nodiscard size_t Size() const {
        return size_;
    }

    g_nodiscard size_t ByteOffset() const {
        return byte_offset_;
    }

private:
    Local<TArray>                       typed_array_;
    std::shared_ptr<v8::BackingStore>   backing_store_;
    uint8_t    *ptr_;
    size_t      byte_size_;
    size_t      size_;
    size_t      byte_offset_;
};

template<typename T>
template<typename SmartPtrLike>
Mem<T>::Mem(Isolate *isolate, SmartPtrLike owned, void *data, size_t size)
{
    CHECK(data && size >= sizeof(T));

    using Retainer = SmartPtrLikeMemRetainer<SmartPtrLike>;
    auto deleter = +[](void *, size_t, void *closure) {
        delete reinterpret_cast<Retainer*>(closure);
    };

    Retainer *closure = new Retainer{ owned };
    std::shared_ptr<v8::BackingStore> bs =
            v8::ArrayBuffer::NewBackingStore(data, size, deleter, closure);
    Local<v8::ArrayBuffer> ab = v8::ArrayBuffer::New(isolate, bs);

    typed_array_ = TArray::New(ab, 0, size / sizeof(T));
    backing_store_ = std::move(bs);
    ptr_ = static_cast<uint8_t*>(data);
    byte_size_ = size;
    size_ = size / sizeof(T);
    byte_offset_ = 0;
}

template<typename T>
class Return<Mem<T>> : public ReturnValueBase
{
public:
    Return(const Mem<T>& mem) : ReturnValueBase(State::kOk), mem_(mem) {}
    Return(Error err) : ReturnValueBase(State::kError, err) {}

    g_nodiscard Mem<T>& Extract() {
        CHECK(!HasError());
        return mem_;
    }

    void AssignTo(v8::ReturnValue<Value> ret_value) {
        CHECK(!HasError() && !mem_.TypedArray().IsEmpty());
        ret_value.Set(mem_.TypedArray());
    }

private:
    Mem<T> mem_;
};

template<typename T>
struct Cast<Mem<T>>
{
    constexpr static bool kWrapperCast = true;

    using TArray = typename Mem<T>::TArray;

    static Ret<Mem<T>> From(Isolate*, Local<Value> value) {
        if (!IsCertainHandle<TArray>(value))
            return Fail(kTypeErr, "requires a TypedArray");

        Local<TArray> array = value.As<TArray>();
        Local<v8::ArrayBuffer> ab = array->Buffer();
        if (ab->WasDetached())
            return Fail(kErr, "underlying ArrayBuffer was detached");

        std::shared_ptr<v8::BackingStore> bs = ab->GetBackingStore();
        size_t byte_offset = array->ByteOffset();
        return Mem<T>(
            array,
            ab->GetBackingStore(),
            reinterpret_cast<uint8_t*>(bs->Data()) + byte_offset,
            array->ByteLength(),
            array->Length(),
            byte_offset
        );
    }

    static RetLocal<Value> To(Isolate*, const Mem<T>& mem) {
        if (mem.TypedArray().IsEmpty())
            return Fail(kErr, "null memory handle");
        return mem.TypedArray();
    }
};

GALLIUM_FFI_NS_END
#endif //COCOA_GALLIUM_FFI_ARRAYBUFFER_H
