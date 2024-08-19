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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_RECT_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_RECT_H

#include "include/core/SkRect.h"
#include "include/core/SkRRect.h"

#include "Gallium/ffi/ArrayBuffer.h"
#include "Gallium/bindings/renderer/Types.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

// `Rect` is defined and implemented in `./renderer.js` in JavaScript.
ffi::Ret<SkRect> UnwrapJSRect(v8::Isolate *isolate, v8::Local<v8::Value> value);
v8::Local<v8::Object> CreateJSRect(v8::Isolate *isolate, const SkRect& rect);

struct RectAdapter : public ffi::ArgAdapter
{
    static ffi::Ret<RectAdapter> Cast(v8::Isolate *isolate, v8::Local<v8::Value> value);

    const SkRect& operator*() const {
        return rect;
    }
    SkRect rect;
};

//! TSDecl: @interface RRect
struct RRect
{
    //! TSDecl: @property rect: Rect
    v8::Local<v8::Object> rect;

    //! TSDecl: @property uniformRadii: boolean
    bool uniform_radii;

    //! TSDecl: @property borderRadii: @array(f32)
    v8::Local<v8::Array> border_radii;
};
//! TSDecl: @end

ffi::Ret<SkRRect> UnwrapJSRRect(v8::Isolate *isolate, v8::Local<v8::Value> value);
v8::Local<v8::Object> CreateJSRRect(v8::Isolate *isolate, const SkRRect& rect);

struct RRectAdapter : public ffi::ArgAdapter
{
    static ffi::Ret<RRectAdapter> Cast(v8::Isolate *isolate, v8::Local<v8::Value> value);

    const SkRRect& operator*() const {
        return rrect;
    }

    SkRRect rrect;
};

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_RECT_H
