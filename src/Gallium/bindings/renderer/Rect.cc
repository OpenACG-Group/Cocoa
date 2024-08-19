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

#include "Gallium/bindings/renderer/Rect.h"
#include "Gallium/ffi/ArrayBuffer.h"
#include "Gallium/ffi/Interface.h"
#include "Gallium/ffi/Context.h"
#include "Gallium/bindings/renderer/Module.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

ffi::Ret<SkRect> UnwrapJSRect(v8::Isolate *isolate, v8::Local<v8::Value> value)
{
    v8::HandleScope handle_scope(isolate);
    if (!value->IsObject())
        return ffi::Fail(ffi::kTypeErr, "cannot interpret the value as a Rect");

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    auto key = v8::String::NewFromUtf8Literal(isolate, "__mem__");

    v8::Local<v8::Value> mem_prop;

    v8::TryCatch try_catch(isolate);
    if (!value.As<v8::Object>()->Get(ctx, key).ToLocal(&mem_prop))
        return ffi::Fail(try_catch, "failed to read `__mem__` property: ");

    ffi::Ret<ffi::Mem<float>> mem = ffi::Cast<ffi::Mem<float>>::From(isolate, mem_prop);
    if (mem.HasError())
    {
        return ffi::Fail(ffi::kTypeErr,
            fmt::format("Rect `__mem__` property: {}", mem.GetError().message));
    }

    if (mem.Extract().Size() != 4)
        return ffi::Fail(ffi::kTypeErr, "Rect `__mem__` property: requires a 4-element Float32Array");

    float *ltrb = mem.Extract().Address();
    return SkRect::MakeLTRB(ltrb[0], ltrb[1], ltrb[2], ltrb[3]);
}

v8::Local<v8::Object> CreateJSRect(v8::Isolate *isolate, const SkRect& rect)
{
    v8::EscapableHandleScope handle_scope(isolate);

    auto *storage = ffi::TypeContext::FromIsolate(isolate)->GetValueStorage();
    auto ctor = storage->Load(FFI_GVSTORE_USE_ID(ctor_Rect)).As<v8::Function>();
    CHECK(!ctor.IsEmpty());

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    v8::Local<v8::Value> args[] = {
        v8::Number::New(isolate, rect.fLeft),
        v8::Number::New(isolate, rect.fTop),
        v8::Number::New(isolate, rect.fRight),
        v8::Number::New(isolate, rect.fBottom)
    };
    return handle_scope.Escape(ctor->NewInstance(ctx, 4, args)
                               .ToLocalChecked().As<v8::Object>());
}

ffi::Ret<RectAdapter> RectAdapter::Cast(v8::Isolate *isolate, v8::Local<v8::Value> value)
{
    ffi::Ret<SkRect> res = UnwrapJSRect(isolate, value);
    if (res.HasError())
        return res.GetError();
    return RectAdapter{ .rect = res.Extract() };
}

namespace {

ffi::Ret<SkRRect> rrect_from_uniform_xy(const SkRect& rect, float *R, uint32_t size)
{
    // Swizzle radii sequence: TL, TR, BR, BL
    switch (size)
    {
    case 1:
        // Input: [TL|BL|TR|BR]
        R[1] = R[0];
        R[2] = R[0];
        R[3] = R[0];
        break;

    case 2:
        // Input: [TL|BR, TR|BL]
        R[2] = R[0];
        R[3] = R[1];
        break;

    case 3:
        // Input: [TL, TR|BL, BR]
        R[3] = R[1];
        break;

    case 4:
        // Input: [TL, TR, BR, BL]
        break;

    default:
        return ffi::Fail(ffi::kErr, "interface `RRect`: invalid size for radii array");
    }

    SkVector v[4] = {
        {R[0], R[0]}, {R[1], R[1]},
        {R[2], R[2]}, {R[3], R[3]}
    };

    SkRRect rrect = SkRRect::MakeEmpty();
    rrect.setRectRadii(rect, v);
    return rrect;
}

ffi::Ret<SkRRect> rrect_from_discrete_xy(const SkRect& rect, float *R, uint32_t size)
{
    SkVector v[4];
    switch (size)
    {
    case 2:
        v[0] = {R[0], R[1]};
        v[1] = v[0];
        v[2] = v[0];
        v[3] = v[0];
        break;

    case 4:
        v[0] = {R[0], R[1]};
        v[1] = {R[2], R[3]};
        v[2] = v[0];
        v[3] = v[1];
        break;

    case 6:
        v[0] = {R[0], R[1]};
        v[1] = {R[2], R[3]};
        v[2] = {R[4], R[5]};
        v[3] = v[1];
        break;

    case 8:
        v[0] = {R[0], R[1]};
        v[1] = {R[2], R[3]};
        v[2] = {R[4], R[5]};
        v[3] = {R[6], R[7]};
        break;

    default:
        return ffi::Fail(ffi::kErr, "interface `RRect`: invalid size for radii array");
    }

    SkRRect rrect = SkRRect::MakeEmpty();
    rrect.setRectRadii(rect, v);
    return rrect;
}

} // namespace anonymous

ffi::Ret<SkRRect> UnwrapJSRRect(v8::Isolate *isolate, v8::Local<v8::Value> value)
{
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

    auto ret_iface = ffi::Cast<ffi::IFace<RRect>>::From(isolate, value);
    if (ret_iface.HasError())
        return ret_iface.GetError();

    ffi::IFace<RRect>& iface = ret_iface.Extract();

    auto maybe_err = UnwrapJSRect(isolate, iface->rect);
    if (maybe_err.HasError())
        return maybe_err.GetError();
    SkRect rect = maybe_err.Extract();

    uint32_t length = iface->border_radii->Length();
    if (length > 8)
        return ffi::Fail(ffi::kErr, "interface `RRect`: property `borderRadii`: length is out of range");

    SkScalar radii[8];
    v8::TryCatch try_catch(isolate);
    for (uint32_t i = 0; i < length; i++)
    {
        v8::Local<v8::Value> v;
        if (!iface->border_radii->Get(ctx, i).ToLocal(&v))
            return ffi::Fail(try_catch, "interface `RRect`: property `borderRadii`: ");

        if (auto radius = ffi::Cast<SkScalar>::From(isolate, v))
            radii[i] = *radius;
        else
            return ffi::Fail(ffi::kTypeErr, "interface `RRect`: property `borderRadii`: invalid array");
    }

    return iface->uniform_radii ? rrect_from_uniform_xy(rect, radii, length)
                                : rrect_from_discrete_xy(rect, radii, length);
}

v8::Local<v8::Object> CreateJSRRect(v8::Isolate *isolate, const SkRRect& rect)
{
    auto iface = ffi::IFace<RRect>::Allocate();

    iface->rect = CreateJSRect(isolate, rect.rect());
    iface->uniform_radii = false;

    v8::Local<v8::Value> radii[] = {
        v8::Number::New(isolate, rect.radii(SkRRect::kUpperLeft_Corner).fX),
        v8::Number::New(isolate, rect.radii(SkRRect::kUpperLeft_Corner).fY),
        v8::Number::New(isolate, rect.radii(SkRRect::kUpperRight_Corner).fX),
        v8::Number::New(isolate, rect.radii(SkRRect::kUpperRight_Corner).fY),
        v8::Number::New(isolate, rect.radii(SkRRect::kLowerRight_Corner).fX),
        v8::Number::New(isolate, rect.radii(SkRRect::kLowerRight_Corner).fY),
        v8::Number::New(isolate, rect.radii(SkRRect::kLowerLeft_Corner).fX),
        v8::Number::New(isolate, rect.radii(SkRRect::kLowerLeft_Corner).fY)
    };
    iface->border_radii = v8::Array::New(isolate, radii, 8);

    return ffi::Cast<ffi::IFace<RRect>>::To(isolate, iface).Extract();
}

ffi::Ret<RRectAdapter> RRectAdapter::Cast(v8::Isolate *isolate, v8::Local<v8::Value> value)
{
    ffi::Ret<SkRRect> rrect = UnwrapJSRRect(isolate, value);
    if (rrect.HasError())
        return rrect.GetError();

    return RRectAdapter{ .rrect = rrect.Extract() };
}

GALLIUM_BINDINGS_RENDERER_NS_END
