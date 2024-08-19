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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_FLATTENABLE_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_FLATTENABLE_H

#include "include/core/SkFlattenable.h"

#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Interface.h"
#include "Gallium/ffi/ArrayBuffer.h"
#include "Gallium/bindings/renderer/Types.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

//! TSDecl: @interface Serializers
struct Serializers
{
    //! TSDecl: @property @optional onImage: @fn(@union(null, @mem(u8)), image: Image)
    ffi::OptLocal<v8::Function> on_image;

    //! TSDecl: @property @optional onPicture: @fn(@union(null, @mem(u8)), picture: Picture)
    ffi::OptLocal<v8::Function> on_picture;

    //! TSDecl: @property @optional onTypeface: @fn(@union(null, @mem(u8)), typeface: Typeface)
    ffi::OptLocal<v8::Function> on_typeface;
};
//! TSDecl: @end

//! TSDecl: @interface Deserializers
struct Deserializers
{
    //! TSDecl: @property @optional onImage: @fn(@union(null, Image), buffer: @mem(u8))
    ffi::OptLocal<v8::Function> on_image;

    //! TSDecl: @property @optional onPicture: @fn(@union(null, Picture), buffer: @mem(u8))
    ffi::OptLocal<v8::Function> on_picture;

    //! TSDecl: @property @optional onTypeface: @fn(@union(null, Typeface), buffer: @mem(u8))
    ffi::OptLocal<v8::Function> on_typeface;

    //! TSDecl: @property @optional allowSkSL: boolean
    ffi::Opt<bool> allow_sksl;
};
//! TSDecl: @end

//! TSDecl: @class @protconstructible Flattenable
class Flattenable : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    enum class Type
    {
        kColorFilter,
        kBlender,
        kDrawable,
        kImageFilter,
        kMaskFilter,
        kPathEffect,
        kShader,
        kPicture
    };

    explicit Flattenable() = default;
    ~Flattenable() override = default;

    // To declare the return type properly, add the corresponding TSDecl comment
    // in the subclass. For example,
    // @method @static Deserialize(memory: @mem(u8),
    //      deserializers: @union(Deserializers, null)): @union(null, Shader)

#define FLATTENABLE_IMPL_DESERIALIZE(type)                                                          \
    static ffi::RetLocal<v8::Value> Deserialize(const ffi::Mem<uint8_t>& memory,                    \
                                                ffi::Opt<ffi::IFace<Deserializers>> deserials) {    \
        return DeserializeImpl(Flattenable::Type::k##type, memory, std::move(deserials));    \
    }

    //! TSDecl: @method serialize(serializers: @union(null, Serializers)): @mem(u8)
    ffi::RetLocal<v8::Value> serialize(ffi::Opt<ffi::IFace<Serializers>> serials);

protected:
    virtual sk_sp<SkData> OnSerializeImpl(SkSerialProcs *procs) = 0;

    static ffi::RetLocal<v8::Value> DeserializeImpl(Type type,
                                                    const ffi::Mem<uint8_t>& memory,
                                                    ffi::Opt<ffi::IFace<Deserializers>> deserials);
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_FLATTENABLE_H
