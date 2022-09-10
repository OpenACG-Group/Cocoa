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

#ifndef COCOA_GALLIUM_BINDER_MODULE_H
#define COCOA_GALLIUM_BINDER_MODULE_H

#include "Gallium/Gallium.h"
#include "Gallium/binder/Function.h"
#include "Gallium/binder/Property.h"

GALLIUM_BINDER_NS_BEGIN

template<typename T, typename Traits>
class Class;

/// Module (similar to v8::ObjectTemplate)
class Module
{
public:
    /// Create new module in the specified V8 GetIsolate
    explicit Module(v8::Isolate *isolate)
            : isolate_(isolate), obj_(v8::ObjectTemplate::New(isolate))
    {
    }

    /// Create new module in the specified V8 GetIsolate for existing ObjectTemplate
    explicit Module(v8::Isolate *isolate, v8::Local<v8::ObjectTemplate> obj)
            : isolate_(isolate), obj_(obj)
    {
    }

    /// v8::Isolate where the module belongs
    v8::Isolate *isolate()
    { return isolate_; }

    /// Set a V8 value in the module with specified name
    template<typename Data>
    Module& set(std::string_view name, v8::Local<Data> value)
    {
        obj_->Set(to_v8(isolate_, name), value);
        return *this;
    }

    /// Set submodule in the module with specified name
    Module& set(std::string_view name, Module& m)
    {
        return set(name, m.obj_);
    }

    /// Set wrapped C++ class in the module with specified name
    template<typename T, typename Traits>
    Module& set(std::string_view name, Class <T, Traits>& cl)
    {
        v8::HandleScope scope(isolate_);

        cl.class_function_template()->SetClassName(to_v8(isolate_, name));
        return set(name, cl.js_function_template());
    }

    /// Set a C++ function in the module with specified name
    template<typename Function, typename Fun = typename std::decay<Function>::type>
    typename std::enable_if<detail::is_callable<Fun>::value, Module&>::type
    set(std::string_view name, Function&& func)
    {
        return set(name, wrap_function_template(isolate_, std::forward<Fun>(func)));
    }

    /// Set a C++ variable in the module with specified name
    template<typename Variable>
    typename std::enable_if<!detail::is_callable<Variable>::value, Module&>::type
    set(std::string_view name, Variable& var, bool readonly = false)
    {
        v8::HandleScope scope(isolate_);

        v8::AccessorGetterCallback getter = &var_get<Variable>;
        v8::AccessorSetterCallback setter = &var_set<Variable>;
        if (readonly)
        {
            setter = nullptr;
        }

        obj_->SetAccessor(to_v8(isolate_, name), getter, setter,
                          detail::external_data::set(isolate_, &var),
                          v8::DEFAULT,
                          v8::PropertyAttribute(v8::DontDelete | (setter ? 0 : v8::ReadOnly)));
        return *this;
    }

    /// Set property in the module with specified name
    template<typename GetFunction, typename SetFunction>
    Module& set(std::string_view name, PropertyObj <GetFunction, SetFunction>&& property)
    {
        using property_type = PropertyObj<GetFunction, SetFunction>;

        v8::HandleScope scope(isolate_);

        v8::AccessorGetterCallback getter = property_type::get;
        v8::AccessorSetterCallback setter = property_type::set;
        if (property_type::is_readonly)
        {
            setter = nullptr;
        }

        obj_->SetAccessor(to_v8(isolate_, name), getter, setter,
                          detail::external_data::set(isolate_, std::forward<property_type>(property)),
                          v8::DEFAULT,
                          v8::PropertyAttribute(v8::DontDelete | (setter ? 0 : v8::ReadOnly)));
        return *this;
    }

    /// Set another module as a read-only property
    Module& set_const(std::string_view name, Module& m)
    {
        v8::HandleScope scope(isolate_);

        obj_->Set(to_v8(isolate_, name), m.obj_,
                  v8::PropertyAttribute(v8::ReadOnly | v8::DontDelete));
        return *this;
    }

    /// Set a value convertible to JavaScript as a read-only property
    template<typename Value>
    Module& set_const(std::string_view name, Value const& value)
    {
        v8::HandleScope scope(isolate_);

        obj_->Set(to_v8(isolate_, name), to_v8(isolate_, value),
                  v8::PropertyAttribute(v8::ReadOnly | v8::DontDelete));
        return *this;
    }

    /// Create a new module instance in V8
    v8::Local<v8::Object> new_instance()
    {
        return obj_->NewInstance(isolate_->GetCurrentContext()).ToLocalChecked();
    }

private:
    template<typename Variable>
    static void var_get(v8::Local<v8::String>,
                        v8::PropertyCallbackInfo<v8::Value> const& info)
    {
        v8::Isolate *isolate = info.GetIsolate();

        Variable *var = detail::external_data::get<Variable *>(info.Data());
        info.GetReturnValue().Set(to_v8(isolate, *var));
    }

    template<typename Variable>
    static void var_set(v8::Local<v8::String>, v8::Local<v8::Value> value,
                        v8::PropertyCallbackInfo<void> const& info)
    {
        v8::Isolate *isolate = info.GetIsolate();

        Variable *var = detail::external_data::get<Variable *>(info.Data());
        *var = from_v8<Variable>(isolate, value);
    }

    v8::Isolate *isolate_;
    v8::Local<v8::ObjectTemplate> obj_;
};


GALLIUM_BINDER_NS_END
#endif //COCOA_GALLIUM_BINDER_MODULE_H
