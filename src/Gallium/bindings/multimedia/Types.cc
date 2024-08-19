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

#include "Gallium/ffi/Context.h"
#include "Gallium/bindings/multimedia/Types.h"
#include "Gallium/bindings/multimedia/Module.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

ffi::Ret<AVRational> UnwrapJSRational(v8::Isolate *isolate, v8::Local<v8::Value> value)
{
    static const char *kError = "could not reinterpret the value as a Rational instance";
    if (!value->IsObject())
        return ffi::Fail(ffi::kErr, kError);
    auto object = value.As<v8::Object>();
    v8::Local<v8::Context> jsctx = isolate->GetCurrentContext();

    v8::Local<v8::Value> num, den;
    if (!object->Get(jsctx, v8::String::NewFromUtf8Literal(isolate, "num")).ToLocal(&num) ||
        !object->Get(jsctx, v8::String::NewFromUtf8Literal(isolate, "den")).ToLocal(&den))
    {
        return ffi::Fail(ffi::kTypeErr, kError);
    }
    if (!num->IsInt32() || !den->IsInt32())
        return ffi::Fail(ffi::kTypeErr, kError);

    return av_make_q(num->Int32Value(jsctx).ToChecked(),
                     den->Int32Value(jsctx).ToChecked());
}

v8::Local<v8::Object> CreateJSRational(v8::Isolate *isolate, const AVRational& q)
{
    auto *storage = ffi::TypeContext::FromIsolate(isolate)->GetValueStorage();
    v8::Local<v8::Value> ctor = storage->Load(FFI_GVSTORE_USE_ID(ctor_Rational));
    CHECK(!ctor.IsEmpty() && ctor->IsFunction());
    v8::Local<v8::Context> jsctx = isolate->GetCurrentContext();

    v8::Local<v8::Value> args[] = {
        v8::Int32::New(isolate, q.num), v8::Int32::New(isolate, q.den)
    };
    return ctor.As<v8::Function>()->CallAsConstructor(jsctx, 2, args)
        .ToLocalChecked().As<v8::Object>();
}

ffi::Ret<AVRationalAdapter>
AVRationalAdapter::Cast(v8::Isolate *isolate, v8::Local<v8::Value> value)
{
    auto rational = UnwrapJSRational(isolate, value);
    if (rational.HasError())
        return rational.GetError();
    return AVRationalAdapter{.q = rational.Extract()};
}

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
