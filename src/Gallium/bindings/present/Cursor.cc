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

#include "Gallium/bindings/present/Cursor.h"
#include "Gallium/bindings/present/Promisify.h"
GALLIUM_BINDINGS_PRESENT_NS_BEGIN

ffi::RetLocal<v8::Value> CursorTheme::dispose()
{
    NotifyDisposeState(DisposeState::kDisposed);
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(isolate, theme_, {}, GLOP_CURSORTHEME_DISPOSE);
}

ffi::RetLocal<v8::Value> CursorTheme::loadCursor(const std::string& name)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(isolate, theme_,
        PromisifiedRemoteCall::GenericConvert<CreateObjCast<std::shared_ptr<gl::Cursor>, Cursor>>,
        GLOP_CURSORTHEME_LOAD_CURSOR_FROM_NAME, name);
}

ffi::RetLocal<v8::Value> Cursor::dispose()
{
    NotifyDisposeState(DisposeState::kDisposed);
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(isolate, cursor_, {}, GLOP_CURSOR_DISPOSE);
}

ffi::RetLocal<v8::Value> Cursor::getHotspotVector()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(
        isolate, cursor_,
        [](v8::Isolate *i, gl::PresentRemoteCallReturn& info) {
            SkIVector vec = info.GetReturnValue<SkIVector>();
            using Tuple = std::tuple<int32_t, int32_t>;
            return ffi::Cast<Tuple>::ToChecked(i, {vec.fX, vec.fY});
        },
        GLOP_CURSOR_GET_HOTSPOT_VECTOR
    );
}

GALLIUM_BINDINGS_PRESENT_NS_END
