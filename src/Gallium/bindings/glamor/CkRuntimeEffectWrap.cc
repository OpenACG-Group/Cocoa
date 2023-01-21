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

#include <unordered_map>
#include <vector>

#include "include/effects/SkGradientShader.h"
#include "fmt/format.h"

#include "Gallium/bindings/glamor/CkRuntimeEffectWrap.h"
#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/CkMatrixWrap.h"
#include "Gallium/binder/ThrowExcept.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

namespace {

enum class EffectTarget
{
    kShader,
    kColorFilter,
    kBlender
};

template<EffectTarget TARGET>
v8::Local<v8::Value> make_rt_effect(const std::string& sksl, bool force_unopt,
                                    v8::Local<v8::Value> callback)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!callback->IsFunction())
        g_throw(TypeError, "Argument `callback` must be a function");

    auto func = v8::Local<v8::Function>::Cast(callback);
    SkRuntimeEffect::Result result;
    SkRuntimeEffect::Options options{};
    options.forceUnoptimized = force_unopt;

    SkString source(sksl);

    if constexpr (TARGET == EffectTarget::kShader) {
        result = SkRuntimeEffect::MakeForShader(source, options);
    } else if constexpr (TARGET == EffectTarget::kColorFilter) {
        result = SkRuntimeEffect::MakeForColorFilter(source, options);
    } else if constexpr (TARGET == EffectTarget::kBlender) {
        result = SkRuntimeEffect::MakeForBlender(source, options);
    }

    if (!result.effect)
    {
        v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
        v8::Local<v8::Value> error = binder::to_v8(isolate, result.errorText.c_str());
        (void) func->Call(ctx, ctx->Global(), 1, &error);
        return v8::Null(isolate);
    }

    return binder::Class<CkRuntimeEffect>::create_object(isolate, result.effect);
}

v8::Local<v8::Value> wrap_uniform(v8::Isolate *isolate, const SkRuntimeEffect::Uniform& uniform)
{
    using VMap = std::unordered_map<std::string_view, v8::Local<v8::Value>>;
    VMap vmap{
        { "name", binder::to_v8(isolate, uniform.name) },
        { "offset", binder::to_v8(isolate, uniform.offset) },
        { "type", binder::to_v8(isolate, static_cast<int32_t>(uniform.type)) },
        { "count", binder::to_v8(isolate, uniform.count) },
        { "flags", binder::to_v8(isolate, uniform.flags) },
        { "sizeInBytes", binder::to_v8(isolate, uniform.sizeInBytes()) }
    };
    return binder::to_v8(isolate, vmap);
}

v8::Local<v8::Value> wrap_child(v8::Isolate *isolate, const SkRuntimeEffect::Child& child)
{
    using VMap = std::unordered_map<std::string_view, v8::Local<v8::Value>>;
    VMap vmap{
        { "name", binder::to_v8(isolate, child.name) },
        { "type", binder::to_v8(isolate, static_cast<int32_t>(child.type)) },
        { "index", binder::to_v8(isolate, child.index) }
    };
    return binder::to_v8(isolate, vmap);
}

bool uniform_requires_int(SkRuntimeEffect::Uniform::Type type)
{
    using T = SkRuntimeEffect::Uniform::Type;
    return (type == T::kInt || type == T::kInt2 || type == T::kInt3 || type == T::kInt4);
}

sk_sp<const SkData>
create_flattened_uniforms_checked(v8::Local<v8::Value> input,
                                  const SkSpan<const SkRuntimeEffect::Uniform>& uniforms)
{
    if (!input->IsArray())
        g_throw(TypeError, "Argument `uniforms` must be an array of numbers");

    size_t uniform_size_bytes = 0;
    for (const auto& uniform : uniforms)
        uniform_size_bytes += uniform.sizeInBytes();

    if (uniform_size_bytes == 0)
        return nullptr;

    sk_sp<SkData> data = SkData::MakeUninitialized(uniform_size_bytes);
    auto *data_ptr = reinterpret_cast<uint8_t*>(data->writable_data());

    auto input_arr = v8::Local<v8::Array>::Cast(input);
    auto nb_inputs = static_cast<int>(input_arr->Length());
    int nb_input_remaining = nb_inputs;

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

    for (const auto& uniform : uniforms)
    {
        int nb_requires = static_cast<int>(uniform.sizeInBytes()) / 4;
        if (nb_requires > nb_input_remaining)
            g_throw(Error, "Provided uniforms array is invalid");

        bool requires_int = uniform_requires_int(uniform.type);
        for (int i = 0; i < nb_requires; i++)
        {
            int idx = nb_inputs - nb_input_remaining + i;
            if (requires_int)
            {
                int32_t v = binder::from_v8<int32_t>(isolate, input_arr->Get(ctx, idx).ToLocalChecked());
                std::memcpy(data_ptr + uniform.offset + i * sizeof(int32_t), &v, sizeof(int32_t));
            }
            else
            {
                float v = binder::from_v8<float>(isolate, input_arr->Get(ctx, idx).ToLocalChecked());
                std::memcpy(data_ptr + uniform.offset + i * sizeof(float), &v, sizeof(float));
            }
        }

        nb_input_remaining -= nb_requires;
    }

    return data;
}

std::vector<SkRuntimeEffect::ChildPtr>
extract_child_specifier_checked(v8::Local<v8::Value> input,
                                const SkSpan<const SkRuntimeEffect::Child>& children)
{
    if (!input->IsArray())
        g_throw(TypeError, "Argument `children` must be an array of numbers");

    if (children.empty())
        return {};

    auto input_arr = v8::Local<v8::Array>::Cast(input);
    if (input_arr->Length() < children.size())
        g_throw(Error, "Argument `children` cannot provide enough child effectors");

    std::vector<SkRuntimeEffect::ChildPtr> result(children.size());

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    for (int32_t i = 0; i < input_arr->Length(); i++)
    {
        auto v = input_arr->Get(ctx, i).ToLocalChecked();
        if (!v->IsObject())
            g_throw(TypeError, "Argument `children` must be an array of `RTEffectChildSpecifier`");

        auto obj = v8::Local<v8::Object>::Cast(v);
        auto maybe_shader = obj->Get(ctx, binder::to_v8(isolate, "shader"))
                                    .FromMaybe(v8::Local<v8::Value>());
        auto maybe_blender = obj->Get(ctx, binder::to_v8(isolate, "blender"))
                                    .FromMaybe(v8::Local<v8::Value>());
        auto maybe_cf = obj->Get(ctx, binder::to_v8(isolate, "colorFilter"))
                                    .FromMaybe(v8::Local<v8::Value>());

#define UNWRAP_CHECKED(var, type) \
    auto *w = binder::Class<type>::unwrap_object(isolate, var); \
    if (!w) {                     \
        g_throw(TypeError, fmt::format("Invalid child effector specifier for `{}`", children[i].name)); \
    }

        uint8_t f = 0;
        f |= !maybe_shader.IsEmpty() && maybe_shader->IsObject();
        f |= (!maybe_blender.IsEmpty() && maybe_blender->IsObject()) << 1;
        f |= (!maybe_cf.IsEmpty() && maybe_cf->IsObject()) << 2;
        if (f == 0b001)
        {
            UNWRAP_CHECKED(maybe_shader, CkShaderWrap)
            result[i] = SkRuntimeEffect::ChildPtr(w->getSkiaObject());
        }
        else if (f == 0b010)
        {
            UNWRAP_CHECKED(maybe_blender, CkBlenderWrap)
            result[i] = SkRuntimeEffect::ChildPtr(w->getSkiaObject());
        }
        else if (f == 0b100)
        {
            UNWRAP_CHECKED(maybe_cf, CkColorFilterWrap)
            result[i] = SkRuntimeEffect::ChildPtr(w->getSkiaObject());
        }
        else
        {
            g_throw(TypeError, "Children specifier is invalid or ambiguous");
        }
#undef UNWRAP_CHECKED
    }

    return result;
}

SkMatrix *extract_maybe_matrix(v8::Isolate *isolate, v8::Local<v8::Value> v, const char *argname)
{
    if (v->IsNullOrUndefined())
        return nullptr;

    auto *w = binder::Class<CkMatrix>::unwrap_object(isolate, v);
    if (!w)
        g_throw(TypeError, fmt::format("Argument `{}` must be an instance of `CkMatrix`", argname));

    return &w->GetMatrix();
}

} // namespace anonymous

v8::Local<v8::Value> CkRuntimeEffect::MakeForBlender(const std::string& sksl, bool forceUnopt,
                                                     v8::Local<v8::Value> callback)
{
    return make_rt_effect<EffectTarget::kBlender>(sksl, forceUnopt, callback);
}

v8::Local<v8::Value> CkRuntimeEffect::MakeForShader(const std::string& sksl, bool forceUnopt,
                                                    v8::Local<v8::Value> callback)
{
    return make_rt_effect<EffectTarget::kShader>(sksl, forceUnopt, callback);
}

v8::Local<v8::Value> CkRuntimeEffect::MakeForColorFilter(const std::string& sksl, bool forceUnopt,
                                                         v8::Local<v8::Value> callback)
{
    return make_rt_effect<EffectTarget::kColorFilter>(sksl, forceUnopt, callback);
}

v8::Local<v8::Value> CkRuntimeEffect::uniforms()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    SkSpan<const SkRuntimeEffect::Uniform> uniforms = getSkiaObject()->uniforms();
    std::vector<v8::Local<v8::Value>> out;
    for (const auto& uniform : uniforms)
        out.push_back(wrap_uniform(isolate, uniform));

    return binder::to_v8(isolate, out);
}

v8::Local<v8::Value> CkRuntimeEffect::children()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    SkSpan<const SkRuntimeEffect::Child> children = getSkiaObject()->children();
    std::vector<v8::Local<v8::Value>> out;
    for (const auto& child : children)
        out.push_back(wrap_child(isolate, child));

    return binder::to_v8(isolate, out);
}

v8::Local<v8::Value> CkRuntimeEffect::findUniform(const std::string& name)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    const SkRuntimeEffect::Uniform *u = getSkiaObject()->findUniform(name);
    if (!u)
        return v8::Null(isolate);
    return wrap_uniform(isolate, *u);
}

v8::Local<v8::Value> CkRuntimeEffect::findChild(const std::string& name)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    const SkRuntimeEffect::Child *c = getSkiaObject()->findChild(name);
    if (!c)
        return v8::Null(isolate);
    return wrap_child(isolate, *c);
}

v8::Local<v8::Value> CkRuntimeEffect::makeShader(v8::Local<v8::Value> uniforms,
                                                 v8::Local<v8::Value> children,
                                                 v8::Local<v8::Value> local_matrix)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto children_vec = extract_child_specifier_checked(children, getSkiaObject()->children());
    SkSpan<SkRuntimeEffect::ChildPtr> children_span(children_vec);

    sk_sp<SkShader> shader = getSkiaObject()->makeShader(
            create_flattened_uniforms_checked(uniforms, getSkiaObject()->uniforms()),
            children_span, extract_maybe_matrix(isolate, local_matrix, "local_matrix"));

    if (!shader)
        return v8::Null(isolate);
    return binder::Class<CkShaderWrap>::create_object(isolate, shader);
}

v8::Local<v8::Value> CkRuntimeEffect::makeBlender(v8::Local<v8::Value> uniforms,
                                                  v8::Local<v8::Value> children)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto children_vec = extract_child_specifier_checked(children, getSkiaObject()->children());
    SkSpan<SkRuntimeEffect::ChildPtr> children_span(children_vec);

    sk_sp<SkBlender> blender = getSkiaObject()->makeBlender(
            create_flattened_uniforms_checked(uniforms, getSkiaObject()->uniforms()),
            children_span);

    if (!blender)
        return v8::Null(isolate);
    return binder::Class<CkBlenderWrap>::create_object(isolate, blender);
}

v8::Local<v8::Value> CkRuntimeEffect::makeColorFilter(v8::Local<v8::Value> uniforms,
                                                      v8::Local<v8::Value> children)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto children_vec = extract_child_specifier_checked(children, getSkiaObject()->children());
    SkSpan<SkRuntimeEffect::ChildPtr> children_span(children_vec);

    sk_sp<SkColorFilter> cf = getSkiaObject()->makeColorFilter(
            create_flattened_uniforms_checked(uniforms, getSkiaObject()->uniforms()),
            children_span);

    if (!cf)
        return v8::Null(isolate);
    return binder::Class<CkColorFilterWrap>::create_object(isolate, cf);
}

GALLIUM_BINDINGS_GLAMOR_NS_END
