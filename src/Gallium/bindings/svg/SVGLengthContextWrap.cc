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

#include "Gallium/bindings/svg/Exports.h"
#include "Gallium/bindings/svg/TrivialInterfaces.h"
#include "Gallium/bindings/glamor/TrivialInterface.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_SVG_NS_BEGIN

v8::Local<v8::Value> SVGLengthContextWrap::Make(v8::Local<v8::Value> vp, SkScalar dpi)
{
    if (dpi <= 0)
        g_throw(RangeError, "Invalid DPI");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return binder::Class<SVGLengthContextWrap>::create_object(
            isolate, ISize::Extract(isolate, vp), dpi);
}

v8::Local<v8::Value> SVGLengthContextWrap::viewport()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ISize::New(isolate, ctx_.viewPort());
}

void SVGLengthContextWrap::setViewport(v8::Local<v8::Value> vp)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    ctx_.setViewPort(ISize::Extract(isolate, vp));
}

SkScalar SVGLengthContextWrap::resolve(v8::Local<v8::Value> length, int32_t type)
{
    if (type < 0 || type > static_cast<int32_t>(SkSVGLengthContext::LengthType::kOther))
        g_throw(RangeError, "Argument `type` is an invalid enumeration value");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ctx_.resolve(ISVGLength::Extract(isolate, length),
                        static_cast<SkSVGLengthContext::LengthType>(type));
}

v8::Local<v8::Value>
SVGLengthContextWrap::resolveRect(v8::Local<v8::Value> x, v8::Local<v8::Value> y,
                                  v8::Local<v8::Value> w, v8::Local<v8::Value> h)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return glamor_wrap::NewCkRect(isolate, ctx_.resolveRect(
            ISVGLength::Extract(isolate, x),
            ISVGLength::Extract(isolate, y),
            ISVGLength::Extract(isolate, w),
            ISVGLength::Extract(isolate, h)
    ));
}

GALLIUM_BINDINGS_SVG_NS_END
