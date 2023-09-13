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
#include "Gallium/bindings/glamor/PromiseHelper.h"
#include "Glamor/Cursor.h"
#include "Glamor/CursorTheme.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

CursorThemeWrap::CursorThemeWrap(std::shared_ptr<gl::CursorTheme> handle)
    : handle_(std::move(handle))
{
}

v8::Local<v8::Value> CursorThemeWrap::dispose()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(
            isolate, handle_, {}, GLOP_CURSORTHEME_DISPOSE);
}

v8::Local<v8::Value> CursorThemeWrap::loadCursorFromName(const std::string& name)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    using ObjCast = CreateObjCast<std::shared_ptr<gl::Cursor>, CursorWrap>;
    return PromisifiedRemoteCall::Call(
            isolate, handle_, PromisifiedRemoteCall::GenericConvert<ObjCast>,
            GLOP_CURSORTHEME_LOAD_CURSOR_FROM_NAME, name);
}

CursorWrap::CursorWrap(std::shared_ptr<gl::Cursor> handle)
    : handle_(std::move(handle))
{
}

v8::Local<v8::Value> CursorWrap::dispose()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(isolate, handle_, {}, GLOP_CURSOR_DISPOSE);
}

v8::Local<v8::Value> CursorWrap::getHotspotVector()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(
            isolate, handle_,
            [](v8::Isolate *i, gl::PresentRemoteCallReturn& info) {
                auto v = info.GetReturnValue<SkIVector>();
                using Map = std::unordered_map<std::string_view, v8::Local<v8::Value>>;
                return binder::to_v8(i, Map{
                    { "x", binder::to_v8(i, v.x()) },
                    { "y", binder::to_v8(i, v.y()) }
                });
            },
            GLOP_CURSOR_GET_HOTSPOT_VECTOR
    );
}

GALLIUM_BINDINGS_GLAMOR_NS_END
