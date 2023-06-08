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

#include "modules/svg/include/SkSVGSVG.h"

#include "Gallium/bindings/svg/Exports.h"
#include "Gallium/bindings/svg/TrivialInterfaces.h"
#include "Gallium/bindings/glamor/CkCanvasWrap.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_SVG_NS_BEGIN

void SVGDOMWrap::setContainerSize(SkScalar width, SkScalar height)
{
    if (width <= 0 || height <= 0)
        g_throw(RangeError, "Invalid width and height");
    dom_->setContainerSize(SkSize::Make(width, height));
}

void SVGDOMWrap::render(v8::Local<v8::Value> canvas)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    glamor_wrap::CkCanvas *wrap =
            binder::UnwrapObject<glamor_wrap::CkCanvas>(isolate, canvas);
    if (!wrap)
        g_throw(TypeError, "Argument `canvas` must be an instance of `CkCanvas`");

    dom_->render(wrap->GetCanvas());
}

v8::Local<v8::Value> SVGDOMWrap::intrinsicSize(v8::Local<v8::Value> ctx)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SVGLengthContextWrap *wrap =
            binder::UnwrapObject<SVGLengthContextWrap>(isolate, ctx);

    if (!wrap)
        g_throw(TypeError, "Argument `ctx` must be an instance of `SVGLengthContext`");

    return ISize::New(isolate, dom_->getRoot()->intrinsicSize(wrap->GetContext()));
}

v8::Local<v8::Value> SVGDOMWrap::width()
{
    return ISVGLength::New(v8::Isolate::GetCurrent(), dom_->getRoot()->getWidth());
}

v8::Local<v8::Value> SVGDOMWrap::height()
{
    return ISVGLength::New(v8::Isolate::GetCurrent(), dom_->getRoot()->getHeight());
}

GALLIUM_BINDINGS_SVG_NS_END
