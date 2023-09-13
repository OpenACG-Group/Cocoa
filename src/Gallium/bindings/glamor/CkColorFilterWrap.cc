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

#include "include/core/SkColorFilter.h"

#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/EffectDSLParser.h"
#include "Gallium/bindings/glamor/EffectDSLBuilderHelperMacros.h"
#include "Gallium/binder/TypeTraits.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

namespace {

//! FilterDecl: compose(ColorFilter outer, ColorFilter inner)
DEF_BUILDER(compose)
{
    CHECK_ARGC(2, compose)
    POP_ARGUMENT_CHECKED(inner, ColorFilter, compose)
    POP_ARGUMENT_CHECKED(outer, ColorFilter, compose)
    return SkColorFilters::Compose(*outer, *inner);
}

//! FilterDecl: blend(Color color, Int blend_mode)
DEF_BUILDER(blend)
{
    CHECK_ARGC(2, blend)
    POP_ARGUMENT_CHECKED(blend_mode_v, Integer, blend)
    POP_ARGUMENT_CHECKED(color, Color, blend)

    if (*blend_mode_v < 0 || *blend_mode_v > static_cast<int>(SkBlendMode::kLastMode))
        g_throw(RangeError, "Invalid enumeration value for argument `blend_mode`");

    auto mode = static_cast<SkBlendMode>(*blend_mode_v);
    return SkColorFilters::Blend(*color, mode);
}

//! FilterDecl: matrix(Float[20] row_major_mat)
DEF_BUILDER(matrix)
{
    CHECK_ARGC(1, matrix)

    using Op = const EffectStackOperand::Ptr&;
    auto array = st.top()->ToMonoTypeArraySafe<SkScalar>([](Op op) {
        return op->ToFloatSafe();
    });
    st.pop();
    if (!array)
        g_throw(Error, "Argument `row_major_mat` should not be null");

    if (array->size() != 20)
        g_throw(Error, "Argument `row_major_mat` must be a 5x4 matrix in row major");

    return SkColorFilters::Matrix(array->data());
}

//! FilterDecl: table(Int[256] table)
DEF_BUILDER(table)
{
    CHECK_ARGC(1, table)

    using Op = const EffectStackOperand::Ptr&;
    auto array = st.top()->ToMonoTypeArraySafe<uint8_t>([](Op op) {
        return op->ToIntegerSafe();
    });
    st.pop();
    if (!array)
        g_throw(Error, "Argument `row_major_mat` should not be null");

    if (array->size() != 256)
        g_throw(Error, "Argument `table` must be an array of 256 integers");

    return SkColorFilters::Table(array->data());
}

//! FilterDecl: table_argb(Int[256]? cA, Int[256]? cR, Int[256]? cG, Int[256]? cB)
DEF_BUILDER(table_argb)
{
    CHECK_ARGC(4, table_argb)

    std::vector<uint8_t> channels[4];
    for (auto & channel : channels)
    {
        using Op = const EffectStackOperand::Ptr&;
        auto array = st.top()->ToMonoTypeArraySafe<uint8_t>([](Op op) {
            return op->ToIntegerSafe();
        });
        st.pop();
        if (!array)
            continue;

        if (array->size() != 256)
            g_throw(Error, "Arguments must be arrays of 256 integers");

        channel = std::move(*array);
    }

#define CH_SELECT(x) channels[x].empty() ? nullptr : channels[x].data()
    return SkColorFilters::TableARGB(CH_SELECT(3), CH_SELECT(2), CH_SELECT(1), CH_SELECT(0));
#undef CH_SELECT
}

//! FilterDecl: linear_to_srgb_gamma()
DEF_BUILDER(linear_to_srgb_gamma)
{
    CHECK_ARGC(0, linear_to_srgb_gamma)
    return SkColorFilters::LinearToSRGBGamma();
}

//! FilterDecl: srgb_to_linear_gamma()
DEF_BUILDER(srgb_to_linear_gamma)
{
    CHECK_ARGC(0, srgb_gamma_to_linear)
    return SkColorFilters::SRGBToLinearGamma();
}

//! FilterDecl: lerp(Float t, ColorFilter dst, ColorFilter src)
DEF_BUILDER(lerp)
{
    CHECK_ARGC(3, lerp)
    POP_ARGUMENT_CHECKED(src, ColorFilter, lerp)
    POP_ARGUMENT_CHECKED(dst, ColorFilter, lerp)
    POP_ARGUMENT_CHECKED(t, Float, lerp)
    return SkColorFilters::Lerp(*t, AUTO_SELECT(dst), AUTO_SELECT(dst));
}

//! FilterDecl: lighting(Color mul, Color add)
DEF_BUILDER(lighting)
{
    CHECK_ARGC(2, lighting)
    POP_ARGUMENT_CHECKED(add, Color, lighting)
    POP_ARGUMENT_CHECKED(mul, Color, lighting)
    return SkColorFilters::Lighting(*mul, *add);
}

#define ENTRY(N)    { #N, builder_##N }
EffectDSLParser::EffectorBuildersMap g_color_filter_builders_map = {
    ENTRY(compose),
    ENTRY(blend),
    ENTRY(matrix),
    ENTRY(table),
    ENTRY(table_argb),
    ENTRY(linear_to_srgb_gamma),
    ENTRY(srgb_to_linear_gamma),
    ENTRY(lerp),
    ENTRY(lighting)
};
#undef ENTRY

} // namespace anonymous

v8::Local<v8::Value> CkColorFilterWrap::MakeFromDSL(v8::Local<v8::Value> dsl,
                                                    v8::Local<v8::Value> kwargs)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    if (!dsl->IsString())
        g_throw(TypeError, "Argument `dsl` must be a string");

    if (!kwargs->IsObject())
        g_throw(TypeError, "Argument `kwargs` must be an object");

    Effector effector = EffectDSLParser::Parse(isolate,
                                               v8::Local<v8::String>::Cast(dsl),
                                               v8::Local<v8::Object>::Cast(kwargs),
                                               g_color_filter_builders_map);

    return binder::NewObject<CkColorFilterWrap>(isolate,
                                                           effector.CheckColorFilter());
}

v8::Local<v8::Value> CkColorFilterWrap::serialize()
{
    sk_sp<SkData> data = GetSkObject()->serialize();
    if (!data)
        g_throw(Error, "Failed to serialize the color filter");

    void *writable_data = data->writable_data();
    auto backing_store = binder::CreateBackingStoreFromSmartPtrMemory(
            data, writable_data, data->size());

    return v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), backing_store);
}

v8::Local<v8::Value> CkColorFilterWrap::Deserialize(v8::Local<v8::Value> buffer)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto memory = binder::GetTypedArrayMemory<v8::TypedArray>(buffer);
    if (!memory)
        g_throw(TypeError, "Argument `buffer` must be an allocated TypedArray");

    auto filter = SkImageFilter::Deserialize(memory->ptr, memory->byte_size);
    if (!filter)
        g_throw(Error, "Failed to deserialize the given buffer as an color filter");

    return binder::NewObject<CkImageFilterWrap>(isolate, filter);
}

GALLIUM_BINDINGS_GLAMOR_NS_END
