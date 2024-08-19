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

#include "Gallium/ffi/InterfaceRegistry.h"
#include "Gallium/ffi/Context.h"
GALLIUM_FFI_NS_BEGIN

InterfaceRegistry *InterfaceRegistry::FromIsolate(Isolate *isolate)
{
    return TypeContext::FromIsolate(isolate)->GetInterfaceRegistry();
}

InterfaceRegistry::InterfaceRegistry(Isolate *isolate)
    : isolate_(isolate)
{
}

void InterfaceRegistry::AddInterface(const ClassTypeInfo& type_info,
                                     InterfaceRegistry::FieldInfoVec fields)
{
    CHECK(type_info_map_.count(type_info) == 0 && "Interface registered");
    type_info_map_[type_info] = std::move(fields);
}

Ret<void> InterfaceRegistry::FillStruct(const ClassTypeInfo& type_info, void *addr,
                                        Local<Object> object)
{
    auto itr = type_info_map_.find(type_info);
    CHECK(itr != type_info_map_.end() && "Struct type not found");

    uint8_t *u8addr = static_cast<uint8_t*>(addr);
    Local<Context> ctx = isolate_->GetCurrentContext();
    for (const PerFieldInfo& per_field : itr->second)
    {
        v8::EscapableHandleScope escape_scope(isolate_);

        Local<String> key = Cast<std::string>::ToChecked(isolate_, per_field.name);
        Local<Value> value = v8::Undefined(isolate_);
        bool has_prop;

        v8::TryCatch try_catch(isolate_);
        if (per_field.search_prototype
            ? !object->Has(ctx, key).To(&has_prop)
            : !object->HasOwnProperty(ctx, key).To(&has_prop))
        {
            return ffi::Fail(try_catch);
        }

        if (!has_prop && !per_field.optional)
        {
            return ffi::Fail(ffi::kErr, fmt::format("Interface `{}` requires field `{}`",
                             per_field.iface_name, per_field.name));
        }

        if (has_prop && !object->Get(ctx, key).ToLocal(&value))
        {
            return ffi::Fail(try_catch, fmt::format("Field `{}` of interface `{}`: ",
                             per_field.name, per_field.iface_name));
        }

        auto maybe_err = per_field.setter(isolate_, u8addr + per_field.offset, value, escape_scope);
        if (maybe_err.HasError())
        {
            return ffi::Fail(ffi::kErr, fmt::format("Field `{}` of interface `{}`: {}",
                            per_field.name, per_field.iface_name, maybe_err.GetError().message));
        }
    }

    return {};
}

RetLocal<Object> InterfaceRegistry::FillObject(const ClassTypeInfo& type_info, const void *addr)
{
    auto itr = type_info_map_.find(type_info);
    CHECK(itr != type_info_map_.end() && "Struct type not found");

    v8::Local<v8::Object> result = v8::Object::New(isolate_);
    v8::Local<v8::Context> ctx = isolate_->GetCurrentContext();
    result->SetPrototype(ctx, v8::Null(isolate_)).Check();

    const uint8_t *u8addr = static_cast<const uint8_t*>(addr);
    for (const PerFieldInfo& per_field : itr->second)
    {
        Local<String> key = Cast<std::string>::ToChecked(isolate_, per_field.name);
        auto value = per_field.getter(isolate_, u8addr + per_field.offset);
        if (value.HasError())
        {
            return ffi::Fail(ffi::kErr, fmt::format("Field `{}` of interface `{}`: {}",
                                                    per_field.name, per_field.iface_name,
                                                    value.GetError().message));
        }

        uint32_t attributes = v8::PropertyAttribute::None;
        if (per_field.readonly)
            attributes |= v8::PropertyAttribute::ReadOnly | v8::PropertyAttribute::DontDelete;

        result->DefineOwnProperty(ctx, key, value.Extract(),
                                  static_cast<v8::PropertyAttribute>(attributes)).Check();
    }
    return result;
}

GALLIUM_FFI_NS_END
