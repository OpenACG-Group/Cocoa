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
#include "Gallium/binder/Convert.h"
#include "Gallium/binder/ThrowExcept.h"
GALLIUM_BINDINGS_TYPETRAITS_NS_BEGIN

void SetInstanceProperties(v8::Local<v8::Object> instance)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

    std::unordered_map<std::string_view, int32_t> constants{
        { "PROPERTY_FILTER_ALL_PROPERTIES",     v8::PropertyFilter::ALL_PROPERTIES      },
        { "PROPERTY_FILTER_ONLY_WRITABLE",      v8::PropertyFilter::ONLY_WRITABLE       },
        { "PROPERTY_FILTER_ONLY_ENUMERABLE",    v8::PropertyFilter::ONLY_ENUMERABLE     },
        { "PROPERTY_FILTER_ONLY_CONFIGURABLE",  v8::PropertyFilter::ONLY_CONFIGURABLE   },
        { "PROPERTY_FILTER_SKIP_STRINGS",       v8::PropertyFilter::SKIP_STRINGS        },
        { "PROPERTY_FILTER_SKIP_SYMBOLS",       v8::PropertyFilter::SKIP_SYMBOLS        },

        { "PROMISE_STATE_FULFILLED",    v8::Promise::PromiseState::kFulfilled   },
        { "PROMISE_STATE_PENDING",      v8::Promise::PromiseState::kPending     },
        { "PROMISE_STATE_REJECTED",     v8::Promise::PromiseState::kRejected    }
    };

    v8::Local<v8::Value> constants_value = binder::to_v8(isolate, constants);
    instance->Set(ctx, binder::to_v8(isolate, "Constants"), constants_value).Check();
}

#define FUNC_IMPL(type)                         \
    bool Is##type(v8::Local<v8::Value> v) {     \
        return v->Is##type();                   \
    }

TYPES_METHOD_MAP(FUNC_IMPL)

bool IsAnyArrayBuffer(v8::Local<v8::Value> v)
{
    return v->IsArrayBuffer() || v->IsSharedArrayBuffer();
}

bool IsBoxedPrimitive(v8::Local<v8::Value> v)
{
    return v->IsNumberObject() ||
           v->IsStringObject() ||
           v->IsBooleanObject() ||
           v->IsBigIntObject() ||
           v->IsSymbolObject();
}

v8::Local<v8::Value> GetOwnNonIndexProperties(v8::Local<v8::Value> obj, int32_t filter)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!obj->IsObject())
        g_throw(TypeError, "Argument `obj` must be an object");

    v8::Local<v8::Array> properties;
    auto filter_v = static_cast<v8::PropertyFilter>(filter);

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    if (!obj.As<v8::Object>()->GetPropertyNames(ctx, v8::KeyCollectionMode::kOwnOnly,
                                                filter_v, v8::IndexFilter::kSkipIndices)
                                                .ToLocal(&properties))
    {
        g_throw(Error, "Failed to get owned property names, invalid filter?");
    }

    return properties;
}

v8::Local<v8::Value> GetConstructorName(v8::Local<v8::Value> obj)
{
    if (!obj->IsObject())
        g_throw(TypeError, "Argument `obj` must be an object");
    return obj.As<v8::Object>()->GetConstructorName();
}

v8::Local<v8::Value> GetPromiseDetails(v8::Local<v8::Value> promise)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!promise->IsPromise())
        g_throw(TypeError, "Argument `promise` must be a Promise");

    v8::Local<v8::Promise> pv = promise.As<v8::Promise>();
    v8::Promise::PromiseState state = pv->State();

    std::unordered_map<std::string_view, v8::Local<v8::Value>> map{
        { "state", binder::to_v8(isolate, static_cast<int32_t>(state)) }
    };

    if (state != v8::Promise::PromiseState::kPending)
        map["result"] = pv->Result();

    return binder::to_v8(isolate, map);
}

v8::Local<v8::Value> GetProxyDetails(v8::Local<v8::Value> proxy)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!proxy->IsProxy())
        g_throw(TypeError, "Argument `proxy` must be a Proxy object");

    v8::Local<v8::Proxy> vp = proxy.As<v8::Proxy>();
    std::unordered_map<std::string_view, v8::Local<v8::Value>> map{
        { "target",  vp->GetTarget()  },
        { "handler", vp->GetHandler() }
    };

    return binder::to_v8(isolate, map);
}

v8::Local<v8::Value> PreviewEntries(v8::Local<v8::Value> obj)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!obj->IsObject())
        g_throw(TypeError, "Argument `obj` must be an object");

    v8::Local<v8::Array> entries;
    bool is_key_value;
    if (!obj.As<v8::Object>()->PreviewEntries(&is_key_value).ToLocal(&entries))
        g_throw(Error, "Failed to preview entries of object");

    std::unordered_map<std::string_view, v8::Local<v8::Value>> map{
        { "entries", entries },
        { "isKeyValue", binder::to_v8(isolate, is_key_value) }
    };

    return binder::to_v8(isolate, map);
}

GALLIUM_BINDINGS_TYPETRAITS_NS_END
