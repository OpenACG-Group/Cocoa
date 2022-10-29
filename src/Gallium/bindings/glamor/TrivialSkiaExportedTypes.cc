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

} // namespace anonymous

SkRect ExtractCkRect(v8::Isolate *isolate, v8::Local<v8::Value> object)
{
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

SkRRect ExtractCkRRect(v8::Isolate *isolate, v8::Local<v8::Value> object)
{
    // TODO(sora): implement this.
}

GALLIUM_BINDINGS_GLAMOR_NS_END
