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

#ifndef COCOA_GALLIUM_FFI_JSOBJECT_H
#define COCOA_GALLIUM_FFI_JSOBJECT_H

#include "Gallium/Gallium.h"
#include "Gallium/ffi/DataTypes.h"
#include "Gallium/ffi/ClassMetadata.h"
#include "Gallium/ffi/ReturnValue.h"
GALLIUM_FFI_NS_BEGIN

class JSTransferData
{
public:
    virtual ~JSTransferData() = default;
    virtual RetLocal<Object> Construct(Isolate *isolate, Local<Context> ctx) = 0;
};

/**
 * Base class of classes that are exported into JavaScript.
 */
class JSObject
{
public:
    static Local<Object> Wrap(Isolate *isolate, const ClassTypeInfo& type_info,
                              JSObject *base_ptr, void *ptr);

    static void *Unwrap(Isolate *isolate, const ClassTypeInfo& type_info, Local<Object> object);

    static JSObject *UnwrapBaseUnsafe(Isolate *isolate, Local<Object> object);

    template<typename T, typename...ArgsT>
    static Local<Object> New(Isolate *isolate, ArgsT&&...args);

    template<typename T>
    static Local<Object> Wrap(Isolate *isolate, T *ptr);

    template<typename T>
    static T *Unwrap(Isolate *isolate, Local<Object> object);

    enum class DisposeState
    {
        kNot,
        kDisposed,
        kDisposing
    };

    JSObject();
    virtual ~JSObject() = default;

    g_nodiscard Local<Object> GetThisHandle(Isolate *isolate) const {
        CHECK(!self_weak_.IsEmpty());
        return self_weak_.Get(isolate);
    }

    g_nodiscard DisposeState GetDisposeState() const {
        return dispose_state_;
    }

    g_nodiscard uint32_t GetObjectAttributes() const {
        return class_metadata_->attrs;
    }

    enum class SerializeType
    {
        kTransfer,
        kClone
    };

    std::unique_ptr<JSTransferData> SerializeObject(Isolate *isolate, SerializeType type);

protected:
    // Implement this if class has attribute `kTransferable_Attr`.
    virtual std::unique_ptr<JSTransferData> OnObjectTransfer(Isolate *isolate);

    // Implement this if class has attribute `kCloneable_Attr`.
    virtual std::unique_ptr<JSTransferData> OnObjectClone(Isolate *isolate);

    void NotifyDisposeState(DisposeState state);

private:
    friend class ClassRegistry;

    // The metadata is owned by `ClassRegistry`. Each class has a corresponding
    // metadata, and each instance of the same class shares the same metadata
    // (their `class_metadata_` pointer is the same). The metadata pointer is set
    // by `ClassRegistry::WrapObject()` after the construction.
    ClassMetadata    *class_metadata_;

    // Instance data
    Global<Object>    self_weak_;
    DisposeState      dispose_state_;
};

#define FFI_JSOBJECT_ATTRS(value) \
    static constexpr int kJSObjectAttrs = value;

template<typename T, typename...ArgsT>
Local<Object> JSObject::New(v8::Isolate *isolate, ArgsT&&...args)
{
    static_assert(std::is_base_of<JSObject, T>::value, "`T` must be a subclass of JSObject");
    auto *instance_ptr = new T(std::forward<ArgsT>(args)...);
    return Wrap(isolate, ClassTypeInfo::Get<T>(), instance_ptr, instance_ptr);
}

template<typename T>
Local<Object> JSObject::Wrap(v8::Isolate *isolate, T *ptr)
{
    static_assert(std::is_base_of<JSObject, T>::value, "`T` must be a subclass of JSObject");
    return Wrap(isolate, ffi::ClassTypeInfo::Get<T>(), ptr, ptr);
}

template<typename T>
T *JSObject::Unwrap(v8::Isolate *isolate, Local<v8::Object> object)
{
    static_assert(std::is_base_of<JSObject, T>::value, "`T` must be a subclass of JSObject");
    return static_cast<T*>(Unwrap(isolate, ffi::ClassTypeInfo::Get<T>(), object));
}

GALLIUM_FFI_NS_END
#endif //COCOA_GALLIUM_FFI_JSOBJECT_H
