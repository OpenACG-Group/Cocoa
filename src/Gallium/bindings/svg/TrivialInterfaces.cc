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

#include <string_view>
#include <unordered_map>

#include "include/v8.h"

#include "Gallium/bindings/svg/TrivialInterfaces.h"
#include "Gallium/binder/Convert.h"
#include "Gallium/binder/ThrowExcept.h"
GALLIUM_BINDINGS_SVG_NS_BEGIN

#define CHECK_OBJECT_TYPE(typename, isolate, obj)           \
    if (!obj->IsObject()) {                                 \
        g_throw(TypeError, "Provided " #typename " is not an object");  \
    }

#define GET_PROPERTY(typename, isolate, ctx, obj, key, typechecker, store) \
    v8::Local<v8::Value> store; \
    if (!obj->Get(ctx, v8::String::NewFromUtf8Literal(isolate, key)).ToLocal(&store)) {          \
        g_throw(TypeError, "Missing property `" key "` on the provided `" #typename "` object"); \
    }                                                                      \
    if (!store->typechecker()) {                                           \
        g_throw(TypeError, "Wrong type of property `" key "` on the provided `" #typename "` object"); \
    }

#define THROW_ENUM_ERROR_OF_PROPERTY(key, typename) \
    g_throw(RangeError, "Invalid enumeration value of property `" key "` on the provided `" #typename "` object")

using ObjectProtoMap = std::unordered_map<std::string_view, v8::Local<v8::Value>>;

v8::Local<v8::Object> ISVGLength::New(v8::Isolate *isolate, const SkSVGLength& from)
{
    return binder::to_v8(isolate, ObjectProtoMap{
        { "value", v8::Number::New(isolate, from.value()) },
        { "unit", v8::Uint32::NewFromUnsigned(isolate, static_cast<uint32_t>(from.unit())) }
    });
}

SkSVGLength ISVGLength::Extract(v8::Isolate *isolate, v8::Local<v8::Value> from)
{
    CHECK_OBJECT_TYPE(SVGLength, isolate, from)

    v8::HandleScope scope(isolate);
    v8::Local<v8::Object> obj = from.As<v8::Object>();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

    GET_PROPERTY(SVGLength, isolate, ctx, obj, "value", IsNumber, value)
    GET_PROPERTY(SVGLength, isolate, ctx, obj, "unit", IsUint32, unit)

    if (unit->Uint32Value(ctx).ToChecked() > static_cast<uint32_t>(SkSVGLength::Unit::kPC))
        THROW_ENUM_ERROR_OF_PROPERTY("unit", SVGLength);

    return SkSVGLength(
        static_cast<SkScalar>(value->NumberValue(ctx).ToChecked()),
        static_cast<SkSVGLength::Unit>(unit->Uint32Value(ctx).ToChecked())
    );
}

v8::Local<v8::Object> ISize::New(v8::Isolate *isolate, const SkSize &from)
{
    return binder::to_v8(isolate, ObjectProtoMap{
        { "width", v8::Number::New(isolate, from.width()) },
        { "height", v8::Number::New(isolate, from.height()) }
    });
}

SkSize ISize::Extract(v8::Isolate *isolate, v8::Local<v8::Value> from)
{
    CHECK_OBJECT_TYPE(Size, isolate, from)

    v8::HandleScope scope(isolate);
    v8::Local<v8::Object> obj = from.As<v8::Object>();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

    GET_PROPERTY(Size, isolate, ctx, obj, "width", IsNumber, width)
    GET_PROPERTY(Size, isolate, ctx, obj, "height", IsNumber, height)

    return SkSize::Make(
        static_cast<SkScalar>(width->NumberValue(ctx).ToChecked()),
        static_cast<SkScalar>(height->NumberValue(ctx).ToChecked())
    );
}

GALLIUM_BINDINGS_SVG_NS_END
