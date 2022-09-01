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

CursorThemeWrap::CursorThemeWrap(const std::shared_ptr<glamor::CursorTheme>& theme)
    : RenderClientObjectWrap(theme)
{
}

v8::Local<v8::Value> CursorThemeWrap::dispose()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto closure = PromiseClosure::New(isolate, nullptr);
    getObject()->Invoke(GLOP_CURSORTHEME_DISPOSE, closure, PromiseClosure::HostCallback);
    return closure->getPromise();
}

v8::Local<v8::Value> CursorThemeWrap::loadCursorFromName(const std::string& name)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    using W = CursorWrap;
    using T = glamor::Shared<glamor::Cursor>;
    auto closure = PromiseClosure::New(isolate,
                                       PromiseClosure::CreateObjectConverter<W, T>);

    getObject()->Invoke(GLOP_CURSORTHEME_LOAD_CURSOR_FROM_NAME, closure,
                        PromiseClosure::HostCallback, name);

    return closure->getPromise();
}

CursorWrap::CursorWrap(const std::shared_ptr<glamor::Cursor>& cursor)
    : RenderClientObjectWrap(cursor)
{
}

v8::Local<v8::Value> CursorWrap::dispose()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto closure = PromiseClosure::New(isolate, nullptr);
    getObject()->Invoke(GLOP_CURSOR_DISPOSE, closure, PromiseClosure::HostCallback);
    return closure->getPromise();
}

v8::Local<v8::Value> CursorWrap::getHotspotVector()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto converter = [](v8::Isolate *i, glamor::RenderHostCallbackInfo& info) -> v8::Local<v8::Value> {
        v8::EscapableHandleScope scope(i);
        auto v = info.GetReturnValue<SkIVector>();
        std::map<std::string_view, v8::Local<v8::Value>> bound_keys{
            {"x", binder::to_v8(i, v.x())},
            {"y", binder::to_v8(i, v.y())}
        };
        return scope.Escape(binder::to_v8(i, bound_keys));
    };

    auto closure = PromiseClosure::New(isolate, converter);

    getObject()->Invoke(GLOP_CURSOR_GET_HOTSPOT_VECTOR, closure,
                        PromiseClosure::HostCallback);
    return closure->getPromise();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
