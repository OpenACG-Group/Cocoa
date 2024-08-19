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

#include "Gallium/bindings/renderer/Paint.h"
#include "Gallium/bindings/renderer/PathEffect.h"
#include "Gallium/bindings/renderer/ImageFilter.h"
#include "Gallium/bindings/renderer/ColorFilter.h"
#include "Gallium/bindings/renderer/Blender.h"
#include "Gallium/bindings/renderer/Shader.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

ffi::RetLocal<v8::Value> Paint::clone()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<Paint>(isolate, paint_);
}

ffi::Ret<bool> Paint::equalTo(ffi::Class<Paint> other)
{
    return (other->paint_ == paint_);
}

ffi::Ret<void> Paint::reset()
{
    paint_.reset();
    return {};
}

#define SET_CAST_PASS(x)   (x)
#define SET_CAST_DEREF(x) *(x)

#define ACCESSORS_IMPL_BASE(type, prop, get, set, set_cast) \
    ffi::Ret<type> Paint::get_##prop() { return paint_.get(); } \
    ffi::Ret<void> Paint::set_##prop(type v) { paint_.set(set_cast(v)); return {}; }

#define ACCESSORS_IMPL_BOOL(prop, getset) \
    ACCESSORS_IMPL_BASE(bool, prop, is##getset, set##getset, SET_CAST_PASS)

#define ACCESSORS_IMPL_ENUM(type, prop, getset) \
    ACCESSORS_IMPL_BASE(ffi::Enum<type>, prop, get##getset, set##getset, SET_CAST_DEREF)

#define ACCESSORS_IMPL_PRIMITIVE(type, prop, getset) \
    ACCESSORS_IMPL_BASE(type, prop, get##getset, set##getset, SET_CAST_PASS)

ACCESSORS_IMPL_BOOL(antiAlias, AntiAlias)
ACCESSORS_IMPL_BOOL(dither, Dither)
ACCESSORS_IMPL_ENUM(SkPaint::Style, style, Style)
ACCESSORS_IMPL_PRIMITIVE(uint32_t, color, Color)
ACCESSORS_IMPL_PRIMITIVE(float, alphaf, Alphaf)
ACCESSORS_IMPL_PRIMITIVE(uint8_t, alpha, Alpha)
ACCESSORS_IMPL_PRIMITIVE(float, strokeWidth, StrokeWidth)
ACCESSORS_IMPL_PRIMITIVE(float, strokeMiter, StrokeMiter)
ACCESSORS_IMPL_ENUM(SkPaint::Cap, strokeCap, StrokeCap)
ACCESSORS_IMPL_ENUM(SkPaint::Join, strokeJoin, StrokeJoin)

ffi::RetLocal<v8::Value> Paint::get_blendMode()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    std::optional<SkBlendMode> mode = paint_.asBlendMode();
    if (!mode)
        return v8::Null(isolate);
    return ffi::Cast<ffi::Enum<SkBlendMode>>::To(isolate, ffi::Enum<SkBlendMode>(*mode));
}

ffi::Ret<void> Paint::set_blendMode(v8::Local<v8::Value> value)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto maybe = ffi::Cast<ffi::Enum<SkBlendMode>>::From(isolate, value);
    if (maybe.HasError())
        return maybe.GetError();
    paint_.setBlendMode(*maybe.Extract());
    return {};
}

#define ACCESSORS_IMPL_CACHABLE(propname, name, cachefield)             \
    ffi::RetLocal<v8::Value> Paint::get_##propname() {                  \
        v8::Isolate *isolate = v8::Isolate::GetCurrent();               \
        if (!cachefield.IsEmpty()) { return cachefield.Get(isolate); }  \
        auto value = paint_.ref##name();                                \
        if (!value) { return v8::Null(isolate); }                       \
        auto object = ffi::JSObject::New<name>(isolate, value);         \
        cachefield.Reset(isolate, object);                              \
        return object;                                                  \
    }                                                                   \
    ffi::Ret<void> Paint::set_##propname(v8::Local<v8::Value> v) {      \
        v8::Isolate *isolate = v8::Isolate::GetCurrent();               \
        if (v->IsNullOrUndefined()) {                                   \
            paint_.set##name(nullptr);                                  \
            cachefield.Reset();                                         \
            return {};                                                  \
        }                                                               \
        auto maybe = ffi::Cast<ffi::Class<name>>::From(isolate, v);     \
        if (maybe.HasError()) { return maybe.GetError(); }              \
        paint_.set##name(maybe.Extract()->GetSk##name());               \
        cachefield.Reset(isolate, v.As<v8::Object>());                  \
        return {};                                                      \
    }

ACCESSORS_IMPL_CACHABLE(pathEffect, PathEffect, path_effect_cache_)
ACCESSORS_IMPL_CACHABLE(imageFilter, ImageFilter, image_filter_cache_)
ACCESSORS_IMPL_CACHABLE(colorFilter, ColorFilter, color_filter_cache_)
ACCESSORS_IMPL_CACHABLE(blender, Blender, blender_cache_)
ACCESSORS_IMPL_CACHABLE(shader, Shader, shader_cache_)

ffi::Ret<Color4fQuadruple> Paint::getColor4f()
{
    SkColor4f c = paint_.getColor4f();
    return std::make_tuple(c.fR, c.fG, c.fB, c.fA);
}

ffi::Ret<void> Paint::setColor4f(Color4fQuadruple value, ffi::Opt<ffi::Class<ColorSpace>> cs)
{
    auto [R, G, B, A] = value;
    paint_.setColor4f(SkColor4f{R, G, B, A}, cs ? (*cs)->GetColorSpace().get() : nullptr);
    return {};
}

ffi::Ret<bool> Paint::nothingToDraw()
{
    return paint_.nothingToDraw();
}

ffi::Ret<bool> Paint::canComputeFastBounds()
{
    return paint_.canComputeFastBounds();
}

ffi::RetLocal<v8::Value> Paint::computeFastBounds(const RectAdapter& orig)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!paint_.canComputeFastBounds())
        return ffi::Fail(ffi::kErr, "fast bounds cannot be computed for this Paint");
    return CreateJSRect(isolate, paint_.computeFastBounds(*orig, nullptr));
}

GALLIUM_BINDINGS_RENDERER_NS_END
