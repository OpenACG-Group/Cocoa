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

#ifndef COCOA_GALLIUM_FFI_CLASS_H
#define COCOA_GALLIUM_FFI_CLASS_H

#include "include/v8-template.h"

#include "Gallium/Gallium.h"
#include "Gallium/ffi/DataTypes.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/ClassRegistry.h"
GALLIUM_FFI_NS_BEGIN

/**
 * `Class<T>` should only be used to temporarily store the address of a
 * class instance, for example, as function arguments of FFI interface.
 * It does NOT keep the ownership of the instance; the lifetime of instance is
 * managed by v8 garbage collector.
 * To hold the ownership of an instance, use `ClassHandle<T>` instead.
 */
template<typename T>
class Class
{
    static_assert(std::is_base_of<JSObject, T>::value, "`T` must be derived from `JSObject`");
public:
    explicit Class(T *instance_ptr) : address_(instance_ptr) {}

    g_nodiscard T *Get() const {
        return address_;
    }

    T *operator->() const {
        return address_;
    }

    T *operator*() const {
        return address_;
    }

private:
    T *address_;
};

/**
 * Similar to `std::unique_ptr`, `ClassInstance<T>` holds a `v8::Global` handle
 * of a C++ class instance that is exported into JavaScript, and a bare-pointer
 * of that instance. It is a wrapper of `v8::Global` that provides a convenient
 * way to access the underlying C++ instance.
 */
template<typename T> class ClassInstance
{
    static_assert(std::is_base_of<JSObject, T>::value, "`T` must be derived from `JSObject`");
public:
    // Empty constructor
    ClassInstance() : address_(nullptr) {}

    ClassInstance(Isolate *isolate, Local<Object> handle) {
        Reset(isolate, handle);
    }

    // Noncopyable
    ClassInstance(const ClassInstance<T>&) = delete;

    template<typename U>
    ClassInstance(ClassInstance<U>&& rhs) noexcept
        : handle_(std::move(rhs.handle_))
        , address_(static_cast<T*>(rhs.address_)) { rhs.address_ = nullptr; }

    ~ClassInstance() = default;

    g_nodiscard bool IsEmpty() const {
        return !address_;
    }

    g_nodiscard T *GetAddress() const {
        CHECK(address_ && "empty handle");
        return address_;
    }

    g_nodiscard Local<Object> GetLocalHandle(Isolate *isolate) const {
        CHECK(!handle_.IsEmpty() && "empty handle");
        return handle_.Get(isolate);
    }

    void Reset() {
        handle_.Reset();
        address_ = nullptr;
    }

    void Reset(Isolate *isolate, Local<Object> handle) {
        T *pointer = JSObject::Unwrap<T>(isolate, handle);
        CHECK(pointer && "No underlying C++ instance");
        handle_.Reset(isolate, handle);
        address_ = pointer;
    }

    T *operator->() const {
        CHECK(address_ && "empty handle");
        return address_;
    }

private:
    Global<Object>  handle_;
    T              *address_;
};

template<typename T>
class Return<Class<T>> : public ReturnValueBase
{
public:
    Return(const Class<T>& instance)
        : ReturnValueBase(State::kOk), class_(instance.Get()) {}

    Return(Error err)
        : ReturnValueBase(State::kError, err), class_(nullptr) {}

    g_nodiscard Class<T>& Extract() {
        return class_;
    }

    void AssignTo(v8::ReturnValue<Value> ret_value);

private:
    Class<T> class_;
};

template<typename T>
struct Cast<Class<T>>
{
    constexpr static bool kWrapperCast = true;

    static Ret<Class<T>> From(Isolate *isolate, Local<Value> value) {
        if (!value->IsObject())
            return Fail(kTypeErr, "requires an object");
        T *instance = JSObject::Unwrap<T>(isolate, value.As<Object>());
        if (!instance) {
            return Fail(kTypeErr, fmt::format("failed to unwrap class type `{}`",
                                              ClassTypeInfo::Get<T>().Name()));
        }
        return Class<T>(instance);
    }

    static RetLocal<Object> To(Isolate *isolate, const Class<T>& cl) {
        if (!cl.Get())
            return Fail(kErr, "null instance pointer");
        return cl->GetThisHandle(isolate);
    }
};

template<typename T>
void Return<Class<T>>::AssignTo(v8::ReturnValue<Value> ret_value)
{
    CHECK(!HasError());
    CHECK(class_.Get() != nullptr);

    Isolate *isolate = ret_value.GetIsolate();
    RetLocal<Object> ret = Cast<Class<T>>::To(isolate, class_);
    if (ret.HasError())
    {
        ret.GetError().Throw(isolate);
        return;
    }

    ret.AssignTo(ret_value);
}

GALLIUM_FFI_NS_END
#endif //COCOA_GALLIUM_FFI_CLASS_H
