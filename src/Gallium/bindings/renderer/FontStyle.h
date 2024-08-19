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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_FONTSTYLE_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_FONTSTYLE_H

#include "include/core/SkFontStyle.h"

#include "Gallium/ffi/DataTypes.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/bindings/renderer/Types.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

//! TSDecl: @interface FontStyle
struct FontStyle
{
    SkFontStyle Extract() const {
        return { weight, width, *slant };
    }

    static v8::Local<v8::Object> CreateJS(const SkFontStyle& from) {
        auto iface = ffi::IFace<FontStyle>::Construct();
        iface->weight = from.weight();
        iface->width = from.width();
        iface->slant = from.slant();
        return ffi::Cast<decltype(iface)>::To(v8::Isolate::GetCurrent(), iface).Extract();
    }

    //! TSDecl: @property weight: i32
    int32_t weight;

    //! TSDecl: @property width: i32
    int32_t width;

    //! TSDecl: @property slant: FontSlant
    ffi::Enum<SkFontStyle::Slant> slant;
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_FONTSTYLE_H
