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

#include <unordered_map>
#include <string_view>

#include "Gallium/bindings/typetraits/Exports.h"
#include "Gallium/ffi/DataTypes.h"
#include "Gallium/ffi/Function.h"
GALLIUM_BINDINGS_TYPETRAITS_NS_BEGIN

#define FUNC_IMPL(type)                         \
    ffi::Ret<bool> Is##type(v8::Local<v8::Value> v) {     \
        return v->Is##type();                   \
    }

TYPES_METHOD_MAP(FUNC_IMPL)

ffi::Ret<bool> IsAnyArrayBuffer(v8::Local<v8::Value> v)
{
    return v->IsArrayBuffer() || v->IsSharedArrayBuffer();
}

ffi::Ret<bool> IsBoxedPrimitive(v8::Local<v8::Value> v)
{
    return v->IsNumberObject() ||
           v->IsStringObject() ||
           v->IsBooleanObject() ||
           v->IsBigIntObject() ||
           v->IsSymbolObject();
}

ffi::RetLocal<v8::Value> GetOwnNonIndexProperties(v8::Local<v8::Object> obj, int32_t filter)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    v8::Local<v8::Array> properties;
    auto filter_v = static_cast<v8::PropertyFilter>(filter);

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    if (!obj->GetPropertyNames(ctx, v8::KeyCollectionMode::kOwnOnly,
                               filter_v, v8::IndexFilter::kSkipIndices)
                               .ToLocal(&properties))
    {
        return ffi::Fail(ffi::kErr, "Failed to get owned property names, invalid filter?");
    }

    return properties;
}

ffi::RetLocal<v8::Value> GetConstructorName(v8::Local<v8::Object> obj)
{
    return obj->GetConstructorName();
}

ffi::RetLocal<v8::Value> GetPromiseDetails(v8::Local<v8::Promise> promise)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    v8::Promise::PromiseState state = promise->State();

    std::unordered_map<std::string, v8::Local<v8::Value>> map{
        { "state", ffi::Cast<int32_t>::To(isolate, static_cast<int32_t>(state)) }
    };

    if (state != v8::Promise::PromiseState::kPending)
        map["result"] = promise->Result();

    return ffi::Cast<decltype(map)>::ToChecked(isolate, map);
}

ffi::RetLocal<v8::Value> GetProxyDetails(v8::Local<v8::Proxy> proxy)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    std::unordered_map<std::string, v8::Local<v8::Value>> map{
        { "target",  proxy->GetTarget()  },
        { "handler", proxy->GetHandler() }
    };

    return ffi::Cast<decltype(map)>::ToChecked(isolate, map);
}

ffi::RetLocal<v8::Value> PreviewEntries(v8::Local<v8::Object> obj)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    v8::Local<v8::Array> entries;
    bool is_key_value;
    if (!obj->PreviewEntries(&is_key_value).ToLocal(&entries))
        return ffi::Fail(ffi::kErr, "Failed to preview entries of object");

    std::unordered_map<std::string, v8::Local<v8::Value>> map{
        { "entries", entries },
        { "isKeyValue", ffi::Cast<bool>::To(isolate, is_key_value) }
    };

    return ffi::Cast<decltype(map)>::ToChecked(isolate, map);
}

GALLIUM_BINDINGS_TYPETRAITS_NS_END
