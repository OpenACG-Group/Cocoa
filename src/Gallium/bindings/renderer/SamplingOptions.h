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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_SAMPLINGOPTIONS_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_SAMPLINGOPTIONS_H

#include "include/core/SkSamplingOptions.h"

#include "Gallium/bindings/renderer/Types.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Interface.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

//! TSDecl: @interface SamplingOptions
struct SamplingOptions
{
    //! TSDecl: @property @readonly @optional maxAniso: i32
    ffi::Opt<int> max_aniso;

    //! TSDecl: @property @readonly @optional useCubic: boolean
    ffi::Opt<bool> use_cubic;

    //! TSDecl: @property @readonly @optional cubicB: f32
    ffi::Opt<float> cubic_B;

    //! TSDecl: @property @readonly @optional cubicC: f32
    ffi::Opt<float> cubic_C;

    //! TSDecl: @property @readonly @optional filter: FilterMode
    ffi::Opt<ffi::Enum<SkFilterMode>> filter;

    //! TSDecl: @property @readonly @optional mipmap: MipmapMode
    ffi::Opt<ffi::Enum<SkMipmapMode>> mipmap;
};
//! TSDecl: @end

class SamplingOptionsAdapter : public ffi::ArgAdapter
{
public:
    static ffi::Ret<SamplingOptionsAdapter>
        Cast(v8::Isolate *isolate, v8::Local<v8::Value> value);

    const SkSamplingOptions& operator*() const {
        return options;
    }

    SkSamplingOptions options;
};

//! TSDecl: @class @nonconstructible CubicSamplers
class CubicSamplers : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    //! TSDecl: @method @static Mitchell(): SamplingOptions
    static ffi::Ret<ffi::IFace<SamplingOptions>> Mictchell();

    //! TSDecl: @method @static CatmullRom(): SamplingOptions
    static ffi::Ret<ffi::IFace<SamplingOptions>> CatmullRom();
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_SAMPLINGOPTIONS_H
