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

#ifndef COCOA_GALLIUM_BINDER_CLASS_H
#define COCOA_GALLIUM_BINDER_CLASS_H

#include <algorithm>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "Gallium/binder/Factory.h"
#include "Gallium/binder/Function.h"
#include "Gallium/binder/Property.h"
#include "Gallium/binder/PtrTraits.h"
#include "Gallium/bindings/ExportableObjectBase.h"

GALLIUM_BINDER_NS_BEGIN

enum ObjectInternalFields
{
    kObjectPtr_InternalFields           = 0,
    kObjectRegistryPtr_InternalFields   = 1,
    kObjectDescriptorPtr_InternalFields = 2,

    kInternalFieldsCount                = 3
};

namespace detail {
struct ClassInfo
{
    type_info const type;
    type_info const traits;

    ClassInfo(type_info const& type, type_info const& traits);

    virtual ~ClassInfo() = default; // make virtual to delete derived object_registry

    g_nodiscard std::string class_name() const;
};

template<typename Traits>
class ObjectRegistry final : public ClassInfo
{
public:
    using pointer_type = typename Traits::pointer_type;
    using const_pointer_type = typename Traits::const_pointer_type;
    using object_id = typename Traits::object_id;
    using descriptor = bindings::ExportableObjectBase::Descriptor;

    using ctor_function = std::function<std::pair<pointer_type, descriptor*>(
                                        v8::FunctionCallbackInfo<v8::Value> const& args)>;

    using dtor_function = std::function<void(v8::Isolate *, pointer_type const&)>;
    using cast_function = pointer_type (*)(pointer_type const&);

    ObjectRegistry(v8::Isolate *isolate, type_info const& type, dtor_function&& dtor);
    ObjectRegistry(ObjectRegistry const&) = delete;
    ObjectRegistry(ObjectRegistry&& src) = default;
    ObjectRegistry& operator=(ObjectRegistry const&) = delete;
    ObjectRegistry& operator=(ObjectRegistry&&) = delete;

    ~ObjectRegistry() override;

    v8::Isolate *isolate()
    { return isolate_; }

    v8::Local<v8::FunctionTemplate> class_function_template()
    {
        return to_local(isolate_, func_);
    }

    v8::Local<v8::FunctionTemplate> js_function_template()
    {
        return to_local(isolate_, js_func_);
    }

    void set_auto_wrap_objects(bool auto_wrap)
    { auto_wrap_objects_ = auto_wrap; }

    g_nodiscard bool auto_wrap_objects() const
    { return auto_wrap_objects_; }

    void set_ctor(ctor_function&& ctor)
    { ctor_ = std::move(ctor); }

    void add_base(ObjectRegistry& info, cast_function cast);

    bool cast(pointer_type& ptr, type_info const& type) const;

    void remove_object(object_id const& obj);

    void remove_objects();

    pointer_type find_object(object_id id, type_info const& type) const;

    v8::Local<v8::Object> find_v8_object(pointer_type const& ptr) const;

    v8::Local<v8::Object> wrap_object(pointer_type const& object,
                                      bindings::ExportableObjectBase::Descriptor *object_descriptor,
                                      bool call_dtor);

    v8::Local<v8::Object> wrap_object(v8::FunctionCallbackInfo<v8::Value> const& args);

    pointer_type unwrap_object(v8::Local<v8::Value> value);

private:
    struct WrappedObject
    {
        v8::Global<v8::Object> pobj;
        bool call_dtor;
    };

    void reset_object(pointer_type const& object, WrappedObject& wrapped);

    struct base_class_info
    {
        ObjectRegistry& info;
        cast_function cast;

        base_class_info(ObjectRegistry& info, cast_function cast)
                : info(info), cast(cast)
        {
        }
    };

    std::vector<base_class_info> bases_;
    std::vector<ObjectRegistry *> derivatives_;
    std::unordered_map<pointer_type, WrappedObject> objects_;

    v8::Isolate *isolate_;
    v8::Global<v8::FunctionTemplate> func_;
    v8::Global<v8::FunctionTemplate> js_func_;

    ctor_function ctor_;
    dtor_function dtor_;
    bool auto_wrap_objects_;
};

class Classes
{
public:
    template<typename Traits>
    static ObjectRegistry<Traits>& add(v8::Isolate *isolate, type_info const& type,
                                       typename ObjectRegistry<Traits>::dtor_function&& dtor);

    template<typename Traits>
    static void remove(v8::Isolate *isolate, type_info const& type);

    template<typename Traits>
    static ObjectRegistry<Traits>& find(v8::Isolate *isolate, type_info const& type);

    static void remove_all(v8::Isolate *isolate);

private:
    using classes_info = std::vector<std::unique_ptr<ClassInfo>>;
    classes_info classes_;

    classes_info::iterator find(type_info const& type);

    enum class operation
    {
        get, add, remove
    };

    static Classes *instance(operation op, v8::Isolate *isolate);
};
} // namespace detail

/**
 * Interface to access C++ classes bound to V8.
 * @note JavaScript exceptions shouldn't be thrown in constructor.
 *       Instead, throwing C++ native exceptions is allowed, which can
 *       be caught and translated into JavaScript exceptions automatically.
 */
template<typename T, typename Traits = raw_ptr_traits>
class Class
{
    static_assert(is_wrapped_class<T>::value, "T must be a user-defined class");

    using object_registry = detail::ObjectRegistry<Traits>;
    object_registry& class_info_;

    using object_id = typename object_registry::object_id;
    using pointer_type = typename object_registry::pointer_type;
    using const_pointer_type = typename object_registry::const_pointer_type;

public:
    using object_pointer_type = typename Traits::template object_pointer_type<T>;
    using object_const_pointer_type = typename Traits::template object_const_pointer_type<T>;

    template<typename ...Args>
    struct factory_create
    {
        static object_pointer_type call(v8::FunctionCallbackInfo<v8::Value> const& args)
        {
            using ctor_function_local = object_pointer_type (*)(v8::Isolate *isolate, Args...);
            return detail::call_from_v8<Traits, ctor_function_local>(&Factory<T, Traits>::create, args);
        }
    };

    using ctor_function = std::function<object_pointer_type(v8::FunctionCallbackInfo<v8::Value> const& args)>;
    using dtor_function = std::function<void(v8::Isolate *isolate, object_pointer_type const& obj)>;

    explicit Class(v8::Isolate *isolate, detail::type_info const& existing)
            : class_info_(detail::Classes::find<Traits>(isolate, existing))
    {
    }

public:
    explicit Class(v8::Isolate *isolate, dtor_function destroy = &Factory<T, Traits>::destroy)
            : class_info_(detail::Classes::add<Traits>(isolate, detail::type_id<T>(),
                                                       [destroy = std::move(destroy)](v8::Isolate *isolate,
                                                                                      pointer_type const& obj) {
                                                           destroy(isolate,
                                                                   Traits::template static_pointer_cast<T>(obj));
                                                       }))
    {
    }

    /// Find existing class_ to extend bindings
    static Class extend(v8::Isolate *isolate)
    {
        return Class(isolate, detail::type_id<T>());
    }

    /// Set class constructor signature
    template<typename ...Args, typename Create = factory_create<Args...>>
    Class& constructor(ctor_function create = &Create::call)
    {
        class_info_.set_ctor([create = std::move(create)](v8::FunctionCallbackInfo<v8::Value> const& args) {
            object_pointer_type ptr = create(args);
            return std::make_pair<pointer_type, bindings::ExportableObjectBase::Descriptor*>(
                    ptr, ptr->GetObjectDescriptor());
        });
        return *this;
    }

    /// Inherit from C++ class U
    template<typename U>
    Class& inherit()
    {
        using namespace detail;
        static_assert(std::is_base_of<U, T>::value,
                      "Class U should be base for class T");
        //TODO: std::is_convertible<T*, U*> and check for duplicates in hierarchy?
        object_registry& base = Classes::find<Traits>(isolate(), type_id<U>());
        class_info_.add_base(base, [](pointer_type const& ptr) -> pointer_type {
            return pointer_type(Traits::template static_pointer_cast<U>(
                    Traits::template static_pointer_cast<T>(ptr)));
        });
        class_info_.js_function_template()->Inherit(base.class_function_template());
        return *this;
    }

    /// Enable new C++ objects auto-wrapping
    Class& auto_wrap_objects(bool auto_wrap = true)
    {
        class_info_.set_auto_wrap_objects(auto_wrap);
        return *this;
    }

    /// Set C++ class member function
    template<typename Method>
    typename std::enable_if<std::is_member_function_pointer<Method>::value, Class&>::type
    set(v8::Local<v8::Name> name, Method mem_func, v8::PropertyAttribute attr = v8::None)
    {
        using mem_func_type =
        typename detail::function_traits<Method>::template pointer_type<T>;
        mem_func_type mf(mem_func);
        class_info_.class_function_template()->PrototypeTemplate()->Set(
                name,
                v8::FunctionTemplate::New(isolate(),
                                          &detail::forward_function<Traits, mem_func_type>,
                                          detail::external_data::set(isolate(), std::forward<mem_func_type>(mf))),
                attr);
        return *this;
    }

    template<typename Method>
    typename std::enable_if<std::is_member_function_pointer<Method>::value, Class&>::type
    set(std::string_view name, Method mem_func, v8::PropertyAttribute attr = v8::None)
    {
        return set(to_v8(isolate(), name), std::forward<Method>(mem_func), attr);
    }

    /// Set static class function
    template<typename Function,
            typename Func = typename std::decay<Function>::type>
    typename std::enable_if<detail::is_callable<Func>::value, Class&>::type
    set_static_func(std::string_view name, Function&& func, v8::PropertyAttribute attr = v8::None)
    {
        v8::HandleScope scope(isolate());

        v8::Local<v8::Context> ctx = isolate()->GetCurrentContext();
        v8::Local<v8::String> v8_name = to_v8(isolate(), name);

        v8::Local<v8::Value> wrapped_fun = wrap_function(isolate(), name, func);

        class_info_.js_function_template()->GetFunction(ctx).ToLocalChecked()
            ->DefineOwnProperty(ctx, v8_name, wrapped_fun, attr).FromJust();
        return *this;
    }

    /// Set class member data
    template<typename Attribute>
    typename std::enable_if<std::is_member_object_pointer<Attribute>::value, Class&>::type
    set(std::string_view name, Attribute attribute, bool readonly = false)
    {
        v8::HandleScope scope(isolate());

        using attribute_type = typename
        detail::function_traits<Attribute>::template pointer_type<T>;
        attribute_type attr(attribute);
        v8::AccessorGetterCallback getter = &member_get<attribute_type>;
        v8::AccessorSetterCallback setter = &member_set<attribute_type>;
        if (readonly)
        {
            setter = nullptr;
        }

        class_info_.class_function_template()->PrototypeTemplate()
                ->SetAccessor(to_v8(isolate(), name), getter, setter,
                              detail::external_data::set(isolate(), std::forward<attribute_type>(attr)),
                              v8::DEFAULT,
                              v8::PropertyAttribute(v8::DontDelete | (setter ? 0 : v8::ReadOnly)));
        return *this;
    }

    /// Set read/write class property with getter and setter
    template<typename GetMethod, typename SetMethod>
    typename std::enable_if<std::is_member_function_pointer<GetMethod>::value
                            && std::is_member_function_pointer<SetMethod>::value, Class&>::type
    set(std::string_view name, PropertyObj<GetMethod, SetMethod>&& property)
    {
        v8::HandleScope scope(isolate());

        using property_type = PropertyObj<
                typename detail::function_traits<GetMethod>::template pointer_type<T>,
                typename detail::function_traits<SetMethod>::template pointer_type<T>
        >;
        property_type prop(property);
        v8::AccessorGetterCallback getter = property_type::template get<Traits>;
        v8::AccessorSetterCallback setter = property_type::template set<Traits>;
        if (prop.is_readonly)
        {
            setter = nullptr;
        }

        class_info_.class_function_template()->PrototypeTemplate()
                ->SetAccessor(to_v8(isolate(), name), getter, setter,
                              detail::external_data::set(isolate(), std::forward<property_type>(prop)),
                              v8::DEFAULT,
                              v8::PropertyAttribute(v8::DontDelete | (setter ? 0 : v8::ReadOnly)));
        return *this;
    }

    /// Set value as a read-only property
    template<typename Value>
    Class& set_const(std::string_view name, Value const& value)
    {
        v8::HandleScope scope(isolate());

        class_info_.class_function_template()->PrototypeTemplate()
                ->Set(to_v8(isolate(), name), to_v8(isolate(), value),
                      v8::PropertyAttribute(v8::ReadOnly | v8::DontDelete));
        return *this;
    }

    /// Set a static value
    template<typename Value>
    Class& set_static(std::string_view name, Value const& value, bool readonly = false)
    {
        v8::HandleScope scope(isolate());

        class_info_.js_function_template()->GetFunction(isolate()->GetCurrentContext()).ToLocalChecked()
                ->DefineOwnProperty(isolate()->GetCurrentContext(),
                                    to_v8(isolate(), name), to_v8(isolate(), value),
                                    v8::PropertyAttribute(v8::DontDelete | (readonly ? v8::ReadOnly : 0))).FromJust();
        return *this;
    }

    /// v8::Isolate where the class bindings belongs
    v8::Isolate *isolate()
    { return class_info_.isolate(); }

    v8::Local<v8::FunctionTemplate> class_function_template()
    {
        return class_info_.class_function_template();
    }

    v8::Local<v8::FunctionTemplate> js_function_template()
    {
        return class_info_.js_function_template();
    }

    /// Create JavaScript object which references externally created C++ class.
    /// It will not take ownership of the C++ pointer.
    static v8::Local<v8::Object> reference_external(v8::Isolate *isolate,
                                                    object_pointer_type const& ext)
    {
        using namespace detail;
        return Classes::find<Traits>(isolate, type_id<T>()).wrap_object(
                ext, ext->GetObjectDescriptor(), false);
    }

    /// Remove external reference from JavaScript
    static void unreference_external(v8::Isolate *isolate, object_pointer_type const& ext)
    {
        using namespace detail;
        return Classes::find<Traits>(isolate, type_id<T>()).remove_object(Traits::pointer_id(ext));
    }

    /// As reference_external but delete memory for C++ object
    /// when JavaScript object is deleted. You must use `factory<T>::create()`
    /// to Allocate `ext`
    static v8::Local<v8::Object> import_external(v8::Isolate *isolate, object_pointer_type const& ext)
    {
        using namespace detail;
        return Classes::find<Traits>(isolate, type_id<T>()).wrap_object(
                ext, ext->GetObjectDescriptor(), true);
    }

    /// Get wrapped object from V8 value, may return nullptr on fail.
    static object_pointer_type unwrap_object(v8::Isolate *isolate, v8::Local<v8::Value> value)
    {
        using namespace detail;
        return Traits::template static_pointer_cast<T>(
                Classes::find<Traits>(isolate, type_id<T>()).unwrap_object(value));
    }

    /// Create a wrapped C++ object and import it into JavaScript
    template<typename ...Args>
    static v8::Local<v8::Object> create_object(v8::Isolate *isolate, Args&& ... args)
    {
        return import_external(isolate,
                               Factory<T, Traits>::create(isolate, std::forward<Args>(args)...));
    }

    /// Find V8 object handle for a wrapped C++ object, may return empty handle on fail.
    static v8::Local<v8::Object> find_object(v8::Isolate *isolate,
                                             object_const_pointer_type const& obj)
    {
        using namespace detail;
        return Classes::find<Traits>(isolate, type_id<T>())
                .find_v8_object(Traits::const_pointer_cast(obj));
    }

    /// Find V8 object handle for a wrapped C++ object, may return empty handle on fail
    /// or wrap a copy of the obj if class_.auto_wrap_objects()
    static v8::Local<v8::Object> find_object(v8::Isolate *isolate, T const& obj)
    {
        using namespace detail;
        detail::ObjectRegistry<Traits>& class_info = Classes::find<Traits>(isolate, type_id<T>());
        v8::Local<v8::Object> wrapped_object = class_info.find_v8_object(Traits::key(const_cast<T *>(&obj)));
        if (wrapped_object.IsEmpty() && class_info.auto_wrap_objects())
        {
            object_pointer_type clone = Traits::clone(obj);
            if (clone)
            {
                wrapped_object = class_info.wrap_object(
                        clone, clone->GetObjectDescriptor(), true);
            }
        }
        return wrapped_object;
    }

    /// Destroy wrapped C++ object
    static void destroy_object(v8::Isolate *isolate, object_pointer_type const& obj)
    {
        using namespace detail;
        Classes::find<Traits>(isolate, type_id<T>()).remove_object(Traits::pointer_id(obj));
    }

    /// Destroy all wrapped C++ objects of this class
    static void destroy_objects(v8::Isolate *isolate)
    {
        using namespace detail;
        Classes::find<Traits>(isolate, type_id<T>()).remove_objects();
    }

    /// Destroy all wrapped C++ objects and this binding class_
    static void destroy(v8::Isolate *isolate)
    {
        using namespace detail;
        Classes::remove<Traits>(isolate, type_id<T>());
    }

private:
    template<typename Attribute>
    static void member_get(v8::Local<v8::String>,
                           v8::PropertyCallbackInfo<v8::Value> const& info)
    {
        v8::Isolate *isolate = info.GetIsolate();

        try
        {
            auto self = unwrap_object(isolate, info.This());
            Attribute attr = detail::external_data::get<Attribute>(info.Data());
            info.GetReturnValue().Set(to_v8(isolate, (*self).*attr));
        }
        catch (const JSException& ex)
        {
            info.GetReturnValue().Set(JSException::TakeOver(ex));
        }
        catch (std::exception const& ex)
        {
            info.GetReturnValue().Set(throw_(isolate, ex.what()));
        }
    }

    template<typename Attribute>
    static void member_set(v8::Local<v8::String>, v8::Local<v8::Value> value,
                           v8::PropertyCallbackInfo<void> const& info)
    {
        v8::Isolate *isolate = info.GetIsolate();

        try
        {
            auto self = unwrap_object(isolate, info.This());
            Attribute ptr = detail::external_data::get<Attribute>(info.Data());
            using attr_type = typename detail::function_traits<Attribute>::return_type;
            (*self).*ptr = from_v8<attr_type>(isolate, value);
        }
        catch (const JSException& ex)
        {
            info.GetReturnValue().Set(JSException::TakeOver(ex));
        }
        catch (std::exception const& ex)
        {
            info.GetReturnValue().Set(throw_(isolate, ex.what()));
        }
    }
};

template<typename T, typename Traits = raw_ptr_traits>
T *UnwrapObject(v8::Isolate *isolate, v8::Local<v8::Value> value)
{
    return Class<T, Traits>::unwrap_object(isolate, value);
}

g_inline bindings::ExportableObjectBase::Descriptor*
UnwrapObjectDescriptor(v8::Isolate *isolate, v8::Local<v8::Value> value)
{
    if (!value->IsObject())
        return nullptr;
    auto obj = value.As<v8::Object>();

    if (obj->InternalFieldCount() != kInternalFieldsCount)
        return nullptr;

    return reinterpret_cast<bindings::ExportableObjectBase::Descriptor*>(
            obj->GetAlignedPointerFromInternalField(kObjectDescriptorPtr_InternalFields));
}

template<typename T>
v8::Local<v8::Object> FindObjectRawPtr(v8::Isolate *isolate, T *ptr)
{
    return Class<T, raw_ptr_traits>::find_object(isolate, ptr);
}

template<typename T, typename...ArgsT>
v8::Local<v8::Object> NewObject(v8::Isolate *isolate, ArgsT&&...args)
{
    return Class<T, raw_ptr_traits>::create_object(
            isolate, std::forward<ArgsT>(args)...);
}

/// Interface to access C++ classes bound to V8
/// Objects are stored in std::shared_ptr
template<typename T>
using SharedClass = Class<T, shared_ptr_traits>;

void Cleanup(v8::Isolate *isolate);

GALLIUM_BINDER_NS_END
#endif //COCOA_GALLIUM_BINDER_CLASS_H
