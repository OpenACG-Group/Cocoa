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

#ifndef COCOA_GALLIUM_FFI_INTERFACE_H
#define COCOA_GALLIUM_FFI_INTERFACE_H

#include <cstdint>
#include <string_view>
#include <string>

#include "Core/Errors.h"
#include "Gallium/Gallium.h"
#include "Gallium/ffi/InterfaceRegistry.h"
GALLIUM_FFI_NS_BEGIN

/**
 * Interface describes a certain structure of a JS object.
 * Using the `Cast<Interface<T>>` class, a C++ struct can be converted
 * to a corresponding JS object, and vice versa.
 */
template<typename T>
class Interface
{
public:
    static Interface<T> Allocate() {
        auto addr = std::make_unique<uint8_t[]>(sizeof(T));
        return Interface<T>(std::move(addr));
    }

    // Like `Allocate()`, but calls the corresponding constructor of `T`
    template<typename...ArgsT>
    static Interface<T> Construct(ArgsT&&...args) {
        auto addr = std::make_unique<uint8_t[]>(sizeof(T));
        ::new(addr.get()) T(std::forward<ArgsT>(args)...);
        return Interface<T>(std::move(addr));
    }

    // Only when `T` has a copy-constructor
    static Interface<T> Create(const Interface<T>& other) {
        auto addr = std::make_unique<uint8_t[]>(sizeof(T));
        ::new(addr.get()) T(other);
        return Interface<T>(std::move(addr));
    }

    // Only when `T` has a move-constructor
    static Interface<T> Create(Interface<T>&& rhs) {
        auto addr = std::make_unique<uint8_t[]>(sizeof(T));
        ::new(addr.get()) T(std::forward<Interface<T>>(rhs));
        return Interface<T>(std::move(addr));
    }

    explicit Interface(std::unique_ptr<uint8_t[]> memory, Local<v8::Object> object = {})
        : memory_(std::move(memory)), object_(object) {}
    ~Interface() = default;

    Interface(const Interface<T>&) = delete;
    Interface(Interface<T>&& rhs) noexcept
        : memory_(std::move(rhs.memory_)), object_(rhs.object_) {
        rhs.object_ = {};
    }

    Interface<T>& operator=(Interface<T>&& rhs) noexcept {
        memory_ = std::move(rhs.memory_);
        return *this;
    }

    T *operator->() const {
        return reinterpret_cast<T*>(memory_.get());
    }

    T *Get() const {
        return reinterpret_cast<T*>(memory_.get());
    }

    void *Address() const {
        return memory_.get();
    }

    const Local<v8::Object>& Object() const {
        return object_;
    }

private:
    std::unique_ptr<uint8_t[]> memory_;
    Local<v8::Object> object_;
};

template<typename T>
using IFace = Interface<T>;

template<typename T>
class Return<Interface<T>> : public ReturnValueBase
{
public:
    Return(Interface<T> iface)
        : ReturnValueBase(State::kOk), iface_(std::move(iface)) {}

    Return(Error err)
        : ReturnValueBase(State::kError, err), iface_(nullptr) {}

    Return<Interface<T>>& operator=(Return<Interface<T>>&& rhs) noexcept {
        state_ = rhs.state_;
        error_ = rhs.error_;
        iface_ = std::move(rhs.iface_);
        return *this;
    }

    g_nodiscard Interface<T>& Extract() {
        return iface_;
    }

    void AssignTo(v8::ReturnValue<Value> ret_value);

private:
    Interface<T> iface_;
};

template<typename T>
struct Cast<Interface<T>>
{
    constexpr static bool kWrapperCast = true;

    // An exception may be returned, so `Ret<...>` type is used instead of `Opt<...>`.
    static Ret<Interface<T>> From(Isolate *isolate, Local<Value> value) {
        if (!value->IsObject())
            return Fail(kTypeErr, "requires an interface (object)");

        ClassTypeInfo type_info = ClassTypeInfo::Get<T>();
        InterfaceRegistry *registry = InterfaceRegistry::FromIsolate(isolate);
        auto memory = std::make_unique<uint8_t[]>(sizeof(T));
        auto maybe_err = registry->FillStruct(type_info, memory.get(), value.As<Object>());
        if (maybe_err.HasError())
            return maybe_err.GetError();
        return Interface<T>(std::move(memory), value.As<Object>());
    }

    static RetLocal<Object> To(Isolate *isolate, const Interface<T>& iface) {
        ClassTypeInfo type_info = ClassTypeInfo::Get<T>();
        InterfaceRegistry *registry = InterfaceRegistry::FromIsolate(isolate);
        return registry->FillObject(type_info, iface.Get());
    }
};

template<typename T>
void Return<Interface<T>>::AssignTo(v8::ReturnValue<Value> ret_value)
{
    CHECK(!HasError());
    CHECK(iface_.Get() != nullptr);

    Isolate *isolate = ret_value.GetIsolate();
    RetLocal<Object> ret = Cast<Interface<T>>::To(isolate, iface_);
    if (ret.HasError())
    {
        ret.GetError().Throw(isolate);
        return;
    }

    ret.AssignTo(ret_value);
}

/**
 * An interface must be defined and registered before being used.
 * `DefineInterface<T>` is a helper class for interface definition.
 */
template<typename T>
class DefineInterface
{
public:
    explicit DefineInterface(Isolate *isolate, std::string iface_name);

    template<typename FieldT>
    DefineInterface& Field(const std::string_view& name, FieldT T::*designator,
                           bool search_prototype = false, bool readonly = false);

    void Finalize();

private:
    InterfaceRegistry *registry_;
    std::string iface_name;
    InterfaceRegistry::FieldInfoVec field_info_;
};

namespace fn {

template<typename T, typename Enable = void>
struct IFaceFieldAccessor;

// Fundamental types accessor.
template<typename T>
struct IFaceFieldAccessor<T, typename std::enable_if<std::is_fundamental_v<T>>::type>
{
    static constexpr bool kOptional = false;

    static Ret<void> Set(Isolate *isolate, void *target_addr, Local<Value> value,
                         v8::EscapableHandleScope&) {
        Opt<T> cast = Cast<T>::From(isolate, value);
        if (!cast)
            return Fail(kTypeErr, "wrong primitive type");
        std::memcpy(target_addr, &(*cast), sizeof(T));
        return {};
    }

    static RetLocal<Value> Get(Isolate *isolate, const void *addr) {
        if constexpr (Cast<T>::kShouldCheckTo) {
            auto ret = Cast<T>::To(isolate, *static_cast<const T*>(addr));
            if (ret.IsEmpty())
                return Fail(kTypeErr, "failed to convert data type");
            return ret.ToLocalChecked();
        } else {
            return Cast<T>::To(isolate, *static_cast<const T*>(addr));
        }
    }
};

// String type accessor (C-string is not supported)
template<typename Str>
struct IFaceFieldAccessor<Str, typename std::enable_if<is_string<Str>::value>::type>
{
    static constexpr bool kOptional = false;

    static Ret<void> Set(Isolate *isolate, void *target_addr, Local<Value> value,
                         v8::EscapableHandleScope&) {
        Opt<Str> cast = Cast<Str>::From(isolate, value);
        if (!cast)
            return Fail(kTypeErr, "requires `string` type");
        ::new(target_addr) Str(*cast);
        return {};
    }

    static RetLocal<Value> Get(Isolate *isolate, const void *addr) {
        Local<String> ret = Cast<Str>::To(isolate, *static_cast<const Str*>(addr));
        if (ret.IsEmpty())
            return Fail(kTypeErr, "failed to convert `string` type");
        return ret;
    }
};

// `Interface<T>` accessor.
template<typename T>
struct IFaceFieldAccessor<T, typename std::enable_if<Cast<T>::kWrapperCast>::type>
{
    static constexpr bool kOptional = false;

    static Ret<void> Set(Isolate *isolate, void *target_addr, Local<Value> value,
                         v8::EscapableHandleScope&) {
        Ret<T> ret = Cast<T>::From(isolate, value);
        if (ret.HasError())
            return ret.GetError();
        ::new(target_addr) T(std::move(ret.Extract()));
        return {};
    }

    static RetLocal<Value> Get(Isolate *isolate, const void *addr) {
        auto ret = Cast<T>::To(isolate, *static_cast<const T*>(addr));
        if (ret.HasError())
            return ret.GetError();
        return ret.Extract();
    }
};

// `vector<T>` accessor
template<typename T>
struct IFaceFieldAccessor<std::vector<T>>
{
    static constexpr bool kOptional = false;

    static Ret<void> Set(Isolate *isolate, void *target_addr, Local<Value> value,
                         v8::EscapableHandleScope& escape_scope)
    {
        Opt<std::vector<T>> cast = Cast<std::vector<T>>::From(isolate, value);
        if (!cast)
            return ffi::Fail(ffi::kTypeErr, "requires an valid array");
        ::new(target_addr) std::vector<T>(std::move(*cast));
        return {};
    }

    static RetLocal<Value> Get(Isolate *isolate, const void *addr) {
        Local<Array> arr = Cast<std::vector<T>>::To(
                isolate, *static_cast<const std::vector<T>*>(addr));
        if (arr.IsEmpty())
            return ffi::Fail(ffi::kErr, "failed to convert std::vector to a JS array");
        return arr;
    }
};

// `Local<T>` accessor.
template<typename T>
struct IFaceFieldAccessor<Local<T>>
{
    static constexpr bool kOptional = false;

    static Ret<void> Set(Isolate*, void *target_addr, Local<Value> value,
                         v8::EscapableHandleScope& escape_scope) {
        if (!IsCertainHandle<T>(value))
            return Fail(kTypeErr, "wrong type");
        // Construct a `Local<T>` object on the given address.
        // The setter function will be called in `InterfaceRegistry::FillStruct` function,
        // which has its own HandleScope. If we construct the `Local<T>` handle directly,
        // it will be a handle under that scope, which means the handle will be invalid
        // when `InterfaceRegistry::FillStruct` function returns. Consequently, an escapable
        // handle scope is used to push the constructed handle into the real caller's
        // handle scope.
        ::new(target_addr) Local<T>(escape_scope.Escape(value.As<T>()));
        return {};
    }

    static RetLocal<Value> Get(Isolate*, const void *addr) {
        // With typeinfo, deref and reinterpret the pointer
        return *static_cast<const Local<T>*>(addr);
    }
};

// `Opt<T>` accessor
template<typename T>
struct IFaceFieldAccessor<Opt<T>>
{
    static constexpr bool kOptional = true;

    static Ret<void> Set(Isolate *isolate, void *target_addr, Local<Value> value,
                         v8::EscapableHandleScope& escape_scope) {
        if (value->IsNullOrUndefined()) {
            // Construct an empty `std::optional`
            ::new(target_addr) Opt<T>();
            return {};
        }
        // Forward to other accessors.
        uint8_t store[sizeof(T)];
        auto maybe_err = IFaceFieldAccessor<T>::Set(isolate, store, value, escape_scope);
        if (maybe_err.HasError())
            return maybe_err.GetError();

        ::new(target_addr) Opt<T>(std::move(*static_cast<T*>(static_cast<void*>(store))));
        return {};
    }

    static RetLocal<Value> Get(Isolate *isolate, const void *addr) {
        const Opt<T>& ref = *static_cast<const Opt<T>*>(addr);
        if (!ref.has_value())
            return v8::Undefined(isolate);
        // Forward to other accessors.
        return IFaceFieldAccessor<T>::Get(isolate, &(*ref));
    }
};

} // namespace fn

template<typename T>
DefineInterface<T>::DefineInterface(Isolate *isolate, std::string iface_name)
    : registry_(InterfaceRegistry::FromIsolate(isolate))
    , iface_name(std::move(iface_name))
{
    CHECK(registry_);
}

template<typename T>
template<typename FieldT>
DefineInterface<T>&
DefineInterface<T>::Field(const std::string_view& name, FieldT T::*designator,
                          bool search_prototype, bool readonly)
{
    static_assert(!std::is_reference_v<FieldT>, "Field is a reference type");
    static_assert(!std::is_pointer_v<FieldT>, "Field is a pointer type");
    static_assert(!std::is_null_pointer_v<FieldT>, "Field is std::nullptr_t");

    InterfaceRegistry::PerFieldInfo info;
    info.name = name;
    info.iface_name = iface_name;
    info.search_prototype = search_prototype;
    info.offset = (uint64_t) &(static_cast<T*>(nullptr)->*designator);
    info.optional = fn::IFaceFieldAccessor<FieldT>::kOptional;
    info.getter = fn::IFaceFieldAccessor<FieldT>::Get;
    info.setter = fn::IFaceFieldAccessor<FieldT>::Set;
    info.readonly = readonly;

    field_info_.emplace_back(info);
    return *this;
}

template<typename T>
void DefineInterface<T>::Finalize()
{
    registry_->AddInterface(
            ClassTypeInfo::Get<T>(), std::move(field_info_));
}

GALLIUM_FFI_NS_END
#endif // COCOA_GALLIUM_FFI_INTERFACE_H
