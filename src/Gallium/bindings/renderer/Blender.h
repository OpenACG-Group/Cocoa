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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_BLENDER_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_BLENDER_H

#include "include/core/SkBlender.h"

#include "Gallium/ffi/Enum.h"
#include "Gallium/bindings/renderer/Types.h"
#include "Gallium/bindings/renderer/Flattenable.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

//! @tsdocbegin
//! Blender represents a custom blend function in the Skia pipeline. When a Blender is
//! present in a paint, the BlendMode is ignored. A blender combines a source color (the
//! result of our paint) and destination color (from the canvas) into a final color.
//! @tsdocend
//! TSDecl: @class @nonconstructible @extends(Flattenable) Blender
class Blender : public Flattenable
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit Blender(sk_sp<SkBlender> bl) : blender_(std::move(bl)) {}
    ~Blender() override = default;

    g_nodiscard sk_sp<SkBlender> GetSkBlender() const {
        return blender_;
    }

    //! @tsdocbegin
    //! Create a blender that implements the specified BlendMode.
    //! @tsdocend
    //! TSDecl: @method @static Mode(mode: BlendMode): Blender
    static ffi::RetLocal<v8::Value> Mode(const ffi::Enum<SkBlendMode>& mode);

    //! @tsdocbegin
    //! Create a blender that implements the following: k1 * src * dst + k2 * src + k3 * dst + k4.
    //!
    //! @param k1 Coefficient.
    //! @param k2 Coefficient.
    //! @param k3 Coefficient.
    //! @param k4 Coefficient.
    //! @param enforcePremul If true, the RGB channels will be clamped to the calculated alpha.
    //! @tsdocend
    //! TSDecl: @method @static Arithmetic(k1: f32, k2: f32, k3: f32, k4: f32, enforcePremul: boolean): Blender
    static ffi::RetLocal<v8::Value> Arithmetic(float k1, float k2, float k3, float k4, bool enforce_pmcolor);

    //! TSDecl: @method @static Deserialize(memory: @mem(u8),
    //! TSDecl:                             deserializers: @union(Deserializers, null))
    //! TSDecl:                             : @union(null, Blender)
    FLATTENABLE_IMPL_DESERIALIZE(Blender)

private:
    sk_sp<SkData> OnSerializeImpl(SkSerialProcs *procs) override;

    sk_sp<SkBlender> blender_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_BLENDER_H
