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

#include "include/v8.h"
#include "include/core/SkRect.h"
#include "include/core/SkRRect.h"

#include "Gallium/bindings/glamor/Exports.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

namespace {

SkRect extract_sk_rect_from_object(v8::Isolate *isolate, v8::Local<v8::Object> object)
{
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    v8::Local<v8::Object> r = object;

    float w[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    float *pw = &w[0];

    bool s[8];
    memset(s, 0, sizeof(s));
    bool *ps = &s[0];

    for (const char *prop : {"left", "top", "right", "bottom",
                             "x", "y", "width", "height"})
    {
        auto jsName = binder::to_v8(isolate, prop);
        if ((*(ps++) = r->HasOwnProperty(ctx, jsName).FromMaybe(false)))
            *(pw++) = binder::from_v8<float>(isolate, r->Get(ctx, jsName).ToLocalChecked());
    }

    if (ps[0] && ps[1] && ps[2] && ps[3])
        return SkRect::MakeLTRB(pw[0], pw[1], pw[2], pw[3]);
    else if (ps[4] && ps[5] && ps[6] && ps[7])
        return SkRect::MakeXYWH(pw[4], pw[5], pw[6], pw[7]);
    else
        g_throw(TypeError, "Invalid `CkRect` object");
}

SkRect extract_sk_rect_from_array(v8::Isolate *isolate, v8::Local<v8::Array> array)
{
    if (array->Length() != 4)
        g_throw(Error, "CkRect array expects 4 elements [x, y, w, h]");

    float xywh[4] = {0, 0, 0, 0};
    float *pw = &xywh[0];

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    for (int i = 0; i < array->Length(); i++)
    {
        v8::Local<v8::Value> value = array->Get(ctx, i).ToLocalChecked();
        if (!value->IsNumber())
            g_throw(TypeError, "Elements in CkRect array must be numbers");
        *(pw++) = binder::from_v8<float>(isolate, value);
    }

    return SkRect::MakeXYWH(xywh[0], xywh[1], xywh[2], xywh[3]);
}

SkRect extract_sk_rect_from_typed_array(v8::Isolate *isolate,
                                        v8::Local<v8::Float32Array> typed_array)
{
    if (typed_array->Length() != 4)
        g_throw(Error, "CkRect array expects 4 elements [x, y, w, h]");

    auto xywh = static_cast<float*>(typed_array->Buffer()->Data());
    return SkRect::MakeXYWH(xywh[0], xywh[1], xywh[2], xywh[3]);
}

uint32_t extract_array_or_f32_array_fixed(v8::Isolate *isolate, v8::Local<v8::Value> array,
                                      float *out, size_t max_size)
{
    if (array->IsFloat32Array())
    {
        auto f32_array = v8::Local<v8::Float32Array>::Cast(array);
        CHECK(!f32_array.IsEmpty());
        if (f32_array->Length() == 0 || f32_array->Length() > max_size)
            g_throw(RangeError, "A wrong size of Float32Array");

        float *start_ptr = static_cast<float*>(f32_array->Buffer()->Data());
        std::memcpy(out, start_ptr, sizeof(float) * f32_array->Length());

        return f32_array->Length();
    }

    if (!array->IsArray())
        g_throw(TypeError, "Invalid type of array");

    auto arr = v8::Local<v8::Array>::Cast(array);
    CHECK(!arr.IsEmpty());

    if (arr->Length() == 0 || arr->Length() > max_size)
        g_throw(RangeError, "A wrong size of Array");

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    for (int i = 0; i < arr->Length(); i++)
    {
        out[i] = binder::from_v8<float>(isolate, arr->Get(context, i).ToLocalChecked());
    }

    return arr->Length();
}

SkRRect rrect_from_uniform_xy(const SkRect& rect, float *R, uint32_t size)
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
        g_throw(Error, "Invalid size of radii array");
    }

    SkVector v[4] = {
        {R[0], R[0]}, {R[1], R[1]},
        {R[2], R[2]}, {R[3], R[3]}
    };

    SkRRect rrect = SkRRect::MakeEmpty();
    rrect.setRectRadii(rect, v);
    return rrect;
}

SkRRect rrect_from_discrete_xy(const SkRect& rect, float *R, uint32_t size)
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
        g_throw(Error, "Invalid size of radii array");
    }

    SkRRect rrect = SkRRect::MakeEmpty();
    rrect.setRectRadii(rect, v);
    return rrect;
}

} // namespace anonymous

SkRect ExtractCkRect(v8::Isolate *isolate, v8::Local<v8::Value> object)
{
    v8::HandleScope scope(isolate);

    if (object->IsFloat32Array())
    {
        return extract_sk_rect_from_typed_array(isolate,
                                                v8::Local<v8::Float32Array>::Cast(object));
    }

    if (object->IsArray())
    {
        return extract_sk_rect_from_array(isolate,
                                          v8::Local<v8::Array>::Cast(object));
    }

    if (object->IsObject())
    {
        return extract_sk_rect_from_object(isolate,
                                           v8::Local<v8::Object>::Cast(object));
    }

    g_throw(TypeError, "Invalid CkRect object");
    MARK_UNREACHABLE();
}

SkRRect ExtractCkRRect(v8::Isolate *isolate, v8::Local<v8::Value> value)
{
    v8::HandleScope scope(isolate);

    if (!value->IsObject())
    {
        g_throw(TypeError, "CkRRect must be an object");
    }

    v8::Local<v8::Object> object = v8::Local<v8::Object>::Cast(value);
    CHECK(!object.IsEmpty());

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    for (const char *name : {"rect", "borderRadii", "uniformRadii"})
    {
        if (!object->HasOwnProperty(context, binder::to_v8(isolate, name)).FromMaybe(false))
        {
            g_throw(TypeError, "CkRRect objects must have a property named `rect`");
        }
    }

    SkRect bounds_rect = ExtractCkRect(isolate,
        object->Get(context, binder::to_v8(isolate, "rect")).ToLocalChecked());
    if (bounds_rect.isEmpty())
        return SkRRect::MakeEmpty();

    v8::Local<v8::Value> uniform_radii_v =
            object->Get(context, binder::to_v8(isolate, "uniformRadii")).ToLocalChecked();
    if (!uniform_radii_v->IsBoolean())
    {
        g_throw(TypeError, "`CkRRect.uniformRadii` must be a boolean value");
    }
    bool uniform_radii = uniform_radii_v->BooleanValue(isolate);

    v8::Local<v8::Value> border_radius =
            object->Get(context, binder::to_v8(isolate, "borderRadii")).ToLocalChecked();

    float radii[8];
    uint32_t radii_size = extract_array_or_f32_array_fixed(
            isolate, border_radius, radii, 8);

    return uniform_radii ? rrect_from_uniform_xy(bounds_rect, radii, radii_size)
                         : rrect_from_discrete_xy(bounds_rect, radii, radii_size);
}

GALLIUM_BINDINGS_GLAMOR_NS_END
