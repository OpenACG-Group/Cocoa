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

#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/CkPaintWrap.h"
#include "Gallium/bindings/glamor/CkPathEffectWrap.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

#define CHECK_ENUM_RANGE(v, last) \
    if (v < 0 || v > static_cast<int32_t>(last)) { \
        g_throw(RangeError, "Invalid enumeration value for arguemnt `" #v "`"); \
    }

void CkPaint::reset()
{
    paint_.reset();
}

void CkPaint::setAntiAlias(bool AA)
{
    paint_.setAntiAlias(AA);
}

void CkPaint::setDither(bool dither)
{
    paint_.setDither(dither);
}

void CkPaint::setStyle(int32_t style)
{
    CHECK_ENUM_RANGE(style, SkPaint::kStyleCount - 1)
    paint_.setStyle(static_cast<SkPaint::Style>(style));
}

void CkPaint::setColor(uint32_t color)
{
    paint_.setColor(color);
}

void CkPaint::setColor4f(v8::Local<v8::Value> color)
{
    paint_.setColor4f(ExtractColor4f(v8::Isolate::GetCurrent(), color));
}

void CkPaint::setAlphaf(float alpha)
{
    paint_.setAlphaf(alpha);
}

void CkPaint::setAlpha(uint32_t alpha)
{
    paint_.setAlpha(std::max(0xffU, alpha));
}

void CkPaint::setStrokeWidth(SkScalar width)
{
    paint_.setStrokeWidth(width);
}

void CkPaint::setStrokeMiter(SkScalar miter)
{
    paint_.setStrokeMiter(miter);
}

void CkPaint::setStrokeCap(int32_t cap)
{
    CHECK_ENUM_RANGE(cap, SkPaint::kLast_Cap)
    paint_.setStrokeCap(static_cast<SkPaint::Cap>(cap));
}

void CkPaint::setStrokeJoin(int32_t join)
{
    CHECK_ENUM_RANGE(join, SkPaint::kLast_Join)
    paint_.setStrokeJoin(static_cast<SkPaint::Join>(join));
}

void CkPaint::setShader(v8::Local<v8::Value> shader)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *wrapper = binder::Class<CkShaderWrap>::unwrap_object(isolate, shader);
    if (!wrapper)
        g_throw(TypeError, "Argument `shader` must be an instance of `CkShader`");
    paint_.setShader(wrapper->getSkiaObject());
}

void CkPaint::setColorFilter(v8::Local<v8::Value> filter)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *wrapper = binder::Class<CkColorFilterWrap>::unwrap_object(isolate, filter);
    if (!wrapper)
        g_throw(TypeError, "Argument `filter` must be an instance of `CkColorFilter`");
    paint_.setColorFilter(wrapper->getSkiaObject());
}

void CkPaint::setBlendMode(int32_t mode)
{
    CHECK_ENUM_RANGE(mode, SkBlendMode::kLastMode)
    paint_.setBlendMode(static_cast<SkBlendMode>(mode));
}

void CkPaint::setBlender(v8::Local<v8::Value> blender)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *wrapper = binder::Class<CkBlenderWrap>::unwrap_object(isolate, blender);
    if (!wrapper)
        g_throw(TypeError, "Argument `blender` must be an instance of `CkBlender`");
    paint_.setBlender(wrapper->getSkiaObject());
}

void CkPaint::setPathEffect(v8::Local<v8::Value> effect)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *wrapper = binder::Class<CkPathEffect>::unwrap_object(isolate, effect);
    if (!wrapper)
        g_throw(TypeError, "Argument `effect` must be an instance of `CkPathEffect`");
    paint_.setPathEffect(wrapper->getSkiaObject());
}

void CkPaint::setImageFilter(v8::Local<v8::Value> filter)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *wrapper = binder::Class<CkImageFilterWrap>::unwrap_object(isolate, filter);
    if (!wrapper)
        g_throw(TypeError, "Argument `filter` must be an instance of `CkImageFilter`");
    paint_.setImageFilter(wrapper->getSkiaObject());
}

GALLIUM_BINDINGS_GLAMOR_NS_END
