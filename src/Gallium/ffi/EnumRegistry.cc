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

#include "Gallium/ffi/EnumRegistry.h"
#include "Gallium/ffi/Context.h"
GALLIUM_FFI_NS_BEGIN

EnumRegistry::EnumRegistry(v8::Isolate *isolate)
    : isolate_(isolate)
{
}

EnumRegistry *EnumRegistry::FromIsolate(Isolate *isolate)
{
    return TypeContext::FromIsolate(isolate)->GetEnumRegistry();
}

Local<Object> EnumRegistry::AddEnum(const ClassTypeInfo& type_info, EnumInfo info)
{
    CHECK(type_info_map_.count(type_info) == 0 && "Enum has been registered");

    info.max_item = 0;
    info.min_item = std::numeric_limits<int64_t>::max();
    v8::Local<v8::Object> object = v8::Object::New(isolate_);
    v8::Local<v8::Context> jsctx = isolate_->GetCurrentContext();
    for (const auto& [name, value] : info.items)
    {
        if (value < info.min_item)
            info.min_item = value;
        if (value > info.max_item)
            info.max_item = value;

        info.value_set.insert(value);

        // We emulate the TypeScript's behavior: for an enum type `T`, `T[T.A] == 'A'`
        auto e_name = Cast<std::string>::ToChecked(isolate_, name);
        auto e_value = Cast<int64_t>::ToChecked(isolate_, value);

        object->Set(jsctx, e_name, e_value).ToChecked();
        object->Set(jsctx, value, e_name).ToChecked();
    }

    type_info_map_.emplace(type_info, std::move(info));
    return object;
}

Ret<int64_t> EnumRegistry::CastEnumValueSafe(const ClassTypeInfo& type_info, Local<Value> value)
{
    auto itr = type_info_map_.find(type_info);
    CHECK(itr != type_info_map_.end() && "Enum not registered");
    const EnumInfo& enum_info = itr->second;

    if (!value->IsInt32() && !value->IsUint32())
        return Fail(kTypeErr, "only integer can be assigned to enum type");

    int64_t i64v = ffi::Cast<int64_t>::FromChecked(isolate_, value);
    if (enum_info.bitfield)
        return i64v;
    if (i64v < enum_info.min_item || i64v > enum_info.max_item)
    {
        return Fail(kRangeErr, fmt::format(
                "enum `{}`: enumeration value out of range", enum_info.name));
    }

    if (enum_info.value_set.count(i64v) == 0)
    {
        return Fail(kRangeErr, fmt::format(
                "enum `{}`: invalid enumeration value", enum_info.name));
    }

    return i64v;
}

GALLIUM_FFI_NS_END
