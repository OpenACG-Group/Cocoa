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

#ifndef COCOA_GALLIUM_BINDINGS_PRESENT_CURSOR_H
#define COCOA_GALLIUM_BINDINGS_PRESENT_CURSOR_H

#include "Glamor/Cursor.h"
#include "Glamor/CursorTheme.h"

#include "Gallium/ffi/JSObject.h"
#include "Gallium/bindings/present/Types.h"
GALLIUM_BINDINGS_PRESENT_NS_BEGIN

//! TSDecl: @class @nonconstructible CursorTheme
class CursorTheme : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit CursorTheme(std::shared_ptr<gl::CursorTheme> theme) : theme_(std::move(theme)) {}
    ~CursorTheme() override = default;

    //! TSDecl: @method dispose(): @promise(void)
    ffi::RetLocal<v8::Value> dispose();

    //! TSDecl: @method loadCursor(name: string): @promise(Cursor)
    ffi::RetLocal<v8::Value> loadCursor(const std::string& name);

private:
    std::shared_ptr<gl::CursorTheme> theme_;
};
//! TSDecl: @end

//! TSDecl: @class @nonconstructible Cursor
class Cursor : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit Cursor(std::shared_ptr<gl::Cursor> cursor) : cursor_(std::move(cursor)) {}
    ~Cursor() override = default;

    g_nodiscard std::shared_ptr<gl::Cursor> GetGLCursor() const {
        return cursor_;
    }

    //! TSDecl: @method dispose(): @promise(void)
    ffi::RetLocal<v8::Value> dispose();

    //! TSDecl: @method getHotspotVector(): @promise(@tuple(i32, i32))
    ffi::RetLocal<v8::Value> getHotspotVector();

private:
    std::shared_ptr<gl::Cursor> cursor_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_PRESENT_NS_END
#endif //COCOA_GALLIUM_BINDINGS_PRESENT_CURSOR_H
