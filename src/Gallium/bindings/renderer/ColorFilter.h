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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_COLORFILTER_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_COLORFILTER_H

#include "include/core/SkColorFilter.h"
#include "include/effects/SkHighContrastFilter.h"

#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/bindings/renderer/Types.h"
#include "Gallium/bindings/renderer/Flattenable.h"
#include "Gallium/bindings/renderer/Color.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

//! TSDecl: @interface HighContrastConfig
struct HighContrastConfig
{
    //! TSDecl: @property grayscale: boolean
    bool grayscale;

    //! TSDecl: @property invertStyle: HighContrastInvertStyle
    ffi::Enum<SkHighContrastConfig::InvertStyle> invert_style;

    //! TSDecl: @property contrast: f32
    float contrast;
};
//! TSDecl: @end

//! TSDecl: @class @nonconstructible @extends(Flattenable) ColorFilter
class ColorFilter : public Flattenable
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit ColorFilter(sk_sp<SkColorFilter> f) : filter_(std::move(f)) {}
    ~ColorFilter() override = default;

    g_nodiscard sk_sp<SkColorFilter> GetSkColorFilter() const {
        return filter_;
    }

    //! TSDecl: @method @static Compose(outer: ColorFilter, inner: ColorFilter): ColorFilter
    static ffi::RetLocal<v8::Value> Compose(const ffi::Class<ColorFilter>& outer,
                                            const ffi::Class<ColorFilter>& inner);

    //! TSDecl: @method @static Blend(c: Color4f, cs: @union(ColorSpace, null), mode: BlendMode): ColorFilter
    static ffi::RetLocal<v8::Value> Blend(Color4fQuadruple color,
                                          const ffi::Opt<ffi::Class<ColorSpace>>& colorspace,
                                          const ffi::Enum<SkBlendMode>& mode);

    //! TSDecl: @method @static Matrix(matrix: ColorMatrix): ColorFilter
    static ffi::RetLocal<v8::Value> Matrix(const ffi::Class<ColorMatrix>& matrix);

    //! TSDecl: @method @static HSLAMatrix(matrix: ColorMatrix): ColorFilter
    static ffi::RetLocal<v8::Value> HSLAMatrix(const ffi::Class<ColorMatrix>& matrix);

    //! TSDecl: @method @static LinearToSRGBGamma(): ColorFilter
    static ffi::RetLocal<v8::Value> LinearToSRGBGamma();

    //! TSDecl: @method @static SRGBToLinearGamma(): ColorFilter
    static ffi::RetLocal<v8::Value> SRGBToLinearGamma();

    //! TSDecl: @method @static Lerp(t: f32, dst: ColorFilter, src: ColorFilter): ColorFilter
    static ffi::RetLocal<v8::Value> Lerp(float t,
                                         const ffi::Class<ColorFilter>& dst,
                                         const ffi::Class<ColorFilter>& src);

    //! TSDecl: @method @static Table(table: @mem(u8)): ColorFilter
    static ffi::RetLocal<v8::Value> Table(const ffi::Mem<uint8_t>& table);

    //! TSDecl: @method @static TableARGB(tableA: @mem(u8), tableR: @mem(u8),
    //! TSDecl:                           tableG: @mem(u8), tableB: @mem(u8)): ColorFilter
    static ffi::RetLocal<v8::Value> TableARGB(const ffi::Mem<uint8_t>& table_a,
                                              const ffi::Mem<uint8_t>& table_r,
                                              const ffi::Mem<uint8_t>& table_g,
                                              const ffi::Mem<uint8_t>& table_b);

    //! TSDecl: @method @static Lighting(mul: u32, add: u32): ColorFilter
    static ffi::RetLocal<v8::Value> Lighting(uint32_t mul, uint32_t add);

    //! TSDecl: @method @static HighContrast(config: HighContrastConfig): ColorFilter
    static ffi::RetLocal<v8::Value> HighContrast(const ffi::IFace<HighContrastConfig>& config);

    //! TSDecl: @method @static Luma(): ColorFilter
    static ffi::RetLocal<v8::Value> Luma();

    //! TSDecl: @method @static Overdraw(colors: @array(u32)): ColorFilter
    static ffi::RetLocal<v8::Value> Overdraw(const std::vector<uint32_t>& colors);


    //! TSDecl: @method @static Deserialize(memory: @mem(u8),
    //! TSDecl:                             deserializers: @union(Deserializers, null))
    //! TSDecl:                             : @union(null, ColorFilter)
    FLATTENABLE_IMPL_DESERIALIZE(ColorFilter)

    //! TSDecl: @method asAColorMode(): @union(null, @tuple(u32, BlendMode))
    ffi::RetLocal<v8::Value> asAColorMode();

    //! TSDecl: @method asAColorMatrix(matrix: @mem(f32)): boolean
    ffi::Ret<bool> asAColorMatrix(const ffi::Mem<float>& matrix);

    //! TSDecl: @method filterColor4f(src: Color4f, srcCS: ColorSpace, dstCS: ColorSpace): Color4f
    ffi::Ret<Color4fQuadruple> filterColor4f(Color4fQuadruple src,
                                             const ffi::Class<ColorSpace>& src_cs,
                                             const ffi::Class<ColorSpace>& dst_cs);

    //! TSDecl: @method makeComposed(inner: ColorFilter): ColorFilter
    ffi::RetLocal<v8::Value> makeComposed(const ffi::Class<ColorFilter>& inner);

    //! TSDecl: @method makeWithWorkingColorSpace(cs: ColorSpace): ColorFilter
    ffi::RetLocal<v8::Value> makeWithWorkingColorSpace(const ffi::Class<ColorSpace>& cs);

private:
    sk_sp<SkData> OnSerializeImpl(SkSerialProcs *procs) override;

    sk_sp<SkColorFilter> filter_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_COLORFILTER_H
