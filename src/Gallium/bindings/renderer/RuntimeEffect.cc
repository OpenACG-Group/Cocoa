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

#include "Gallium/ffi/Interface.h"
#include "Gallium/bindings/renderer/RuntimeEffect.h"
#include "Gallium/bindings/renderer/Shader.h"
#include "Gallium/bindings/renderer/ColorFilter.h"
#include "Gallium/bindings/renderer/Blender.h"
#include "Gallium/bindings/renderer/BufferUtils.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

RuntimeEffect::RuntimeEffect(v8::Local<v8::String> sksl_source, sk_sp<SkRuntimeEffect> effect)
    : effect_(std::move(effect))
    , sksl_source_(v8::Isolate::GetCurrent(), sksl_source)
{
}

namespace {

enum class SkSLTarget
{
    kShader,
    kColorFilter,
    kBlender
};

ffi::RetLocal<v8::Value> compile_sksl(v8::Local<v8::String> source, SkSLTarget target)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::String::Utf8Value source_utf8(isolate, source);
    if (source_utf8.length() == 0)
        return ffi::Fail(ffi::kErr, "empty or bad-encoded SkSL source code");

    SkString source_str(*source_utf8, source_utf8.length());

    SkRuntimeEffect::Result result;
    if (target == SkSLTarget::kShader)
        result = SkRuntimeEffect::MakeForShader(source_str);
    else if (target == SkSLTarget::kColorFilter)
        result = SkRuntimeEffect::MakeForColorFilter(source_str);
    else if (target == SkSLTarget::kBlender)
        result = SkRuntimeEffect::MakeForBlender(source_str);
    else
        MARK_UNREACHABLE();

    using Tuple = std::tuple<v8::Local<v8::Value>, std::string>;
    if (!result.effect)
    {
        return ffi::Cast<Tuple>::ToChecked(isolate, {
            v8::Null(isolate), std::string(result.errorText.c_str())
        });
    }

    return ffi::Cast<Tuple>::ToChecked(isolate, {
        ffi::JSObject::New<RuntimeEffect>(isolate, source, result.effect),
        ""
    });
}

} // namespace anonymous

ffi::RetLocal<v8::Value> RuntimeEffect::CompileColorFilter(v8::Local<v8::String> sksl)
{
    return compile_sksl(sksl, SkSLTarget::kColorFilter);
}

ffi::RetLocal<v8::Value> RuntimeEffect::CompileBlender(v8::Local<v8::String> sksl)
{
    return compile_sksl(sksl, SkSLTarget::kBlender);
}

ffi::RetLocal<v8::Value> RuntimeEffect::CompileShader(v8::Local<v8::String> sksl)
{
    return compile_sksl(sksl, SkSLTarget::kShader);
}

namespace {

ffi::Ret<std::vector<SkRuntimeEffect::ChildPtr>>
extract_children(v8::Isolate *isolate, const std::vector<v8::Local<v8::Object>>& children)
{
    std::vector<SkRuntimeEffect::ChildPtr> result;
    result.reserve(children.size());
    for (const v8::Local<v8::Object>& js_child : children)
    {
        if (Shader *shader = ffi::JSObject::Unwrap<Shader>(isolate, js_child))
            result.emplace_back(shader->GetSkShader());
        else if (ColorFilter *cf = ffi::JSObject::Unwrap<ColorFilter>(isolate, js_child))
            result.emplace_back(cf->GetSkColorFilter());
        else if (Blender *bl = ffi::JSObject::Unwrap<Blender>(isolate, js_child))
            result.emplace_back(bl->GetSkBlender());
        else
            return ffi::Fail(ffi::kErr, "invalid SkSL child: must be a Shader, ColorFilter, or Blender");
    }
    return result;
}

} // namespace anonymous

#define MAKE_XXX_ARGS_CONVERSION \
    v8::Isolate *isolate = v8::Isolate::GetCurrent();                                                 \
    auto children_ptrs = extract_children(isolate, children);                                         \
    if (children_ptrs.HasError())                                                                     \
        return children_ptrs.GetError();                                                              \
    if (children_ptrs.Extract().size() != effect_->children().size())                                 \
        return ffi::Fail(ffi::kErr, "number of children does not satisfy the requirement");           \
    if (!uniforms && effect_->uniformSize() > 0) {                                                    \
        return ffi::Fail(ffi::kErr, "a uniform buffer is required for this SkSL program");            \
    }                                                                                                 \
    sk_sp<SkData> uniforms_data;                                                                      \
    if (uniforms) {                                                                                   \
        if (uniforms->ByteSize() < effect_->uniformSize())                                            \
            return ffi::Fail(ffi::kErr, "invalid uniform buffer (size is insufficient)");             \
        uniforms_data = WrapArrayBufferToSkData(                                                      \
                isolate, uniforms->TypedArray(), kNoCopy_MemoryFlag | kDetachBuffer_MemoryFlag, {});  \
        if (!uniforms_data)                                                                           \
            return ffi::Fail(ffi::kErr, "invalid uniform buffer (must be allocated and detachable)"); \
    }

ffi::RetLocal<v8::Value> RuntimeEffect::makeShader(const ffi::Opt<ffi::Mem<uint8_t>>& uniforms,
                                                   const std::vector<v8::Local<v8::Object>>& children,
                                                   const ffi::Opt<Mat3x3Adapter>& local_matrix)
{
    if (!effect_->allowShader())
        return ffi::Fail(ffi::kErr, "this SkSL program cannot be used as a Shader");

    MAKE_XXX_ARGS_CONVERSION

    sk_sp<SkShader> shader = effect_->makeShader(
            uniforms_data, children_ptrs.Extract(), local_matrix ? &(**local_matrix) : nullptr);
    if (!shader)
        return ffi::Fail(ffi::kErr, "failed to make a runtime Shader");
    return ffi::JSObject::New<Shader>(isolate, shader);
}

ffi::RetLocal<v8::Value> RuntimeEffect::makeColorFilter(const ffi::Opt<ffi::Mem<uint8_t>>& uniforms,
                                                        const std::vector<v8::Local<v8::Object>>& children)
{
    if (!effect_->allowColorFilter())
        return ffi::Fail(ffi::kErr, "this SkSL program cannot be used as a ColorFilter");

    MAKE_XXX_ARGS_CONVERSION

    sk_sp<SkColorFilter> cf = effect_->makeColorFilter(uniforms_data, children_ptrs.Extract());
    if (!cf)
        return ffi::Fail(ffi::kErr, "failed to make a runtime ColorFilter");
    return ffi::JSObject::New<ColorFilter>(isolate, cf);
}

ffi::RetLocal<v8::Value> RuntimeEffect::makeBlender(const ffi::Opt<ffi::Mem<uint8_t>>& uniforms,
                                                    const std::vector<v8::Local<v8::Object>>& children)
{
    if (!effect_->allowBlender())
        return ffi::Fail(ffi::kErr, "this SkSL program cannot be used as a Blender");

    MAKE_XXX_ARGS_CONVERSION

    sk_sp<SkBlender> blender = effect_->makeBlender(uniforms_data, children_ptrs.Extract());
    if (!blender)
        return ffi::Fail(ffi::kErr, "failed to make a runtime Blender");
    return ffi::JSObject::New<Blender>(isolate, blender);
}

namespace {

v8::Local<v8::Value> fill_uniform_spec(v8::Isolate *isolate,
                                       const SkRuntimeEffect::Uniform& uniform)
{
    auto iface = ffi::IFace<SkSLReflectUniform>::Construct();
    iface->name = uniform.name;
    iface->offset = uniform.offset;
    iface->size_in_bytes = uniform.sizeInBytes();
    iface->type = uniform.type;
    iface->count = uniform.count;
    iface->flags = uniform.flags;
    return ffi::Cast<decltype(iface)>::To(isolate, iface).Extract();
}

v8::Local<v8::Value> fill_child_spec(v8::Isolate *isolate,
                                     const SkRuntimeEffect::Child& child)
{
    auto iface = ffi::IFace<SkSLReflectChild>::Construct();
    iface->name = child.name;
    iface->type = child.type;
    iface->index = child.index;
    return ffi::Cast<decltype(iface)>::To(isolate, iface).Extract();
}

} // namespace anonymous

ffi::RetLocal<v8::Value> RuntimeEffect::getUniforms()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!uniforms_array_.IsEmpty())
        return uniforms_array_.Get(isolate);

    std::vector<v8::Local<v8::Value>> js_uniforms;
    js_uniforms.reserve(effect_->uniforms().size());

    for (const SkRuntimeEffect::Uniform& uniform : effect_->uniforms())
        js_uniforms.emplace_back(fill_uniform_spec(isolate, uniform));

    auto js_array = v8::Array::New(isolate, js_uniforms.data(), js_uniforms.size());
    uniforms_array_.Reset(isolate, js_array);
    return js_array;
}

ffi::RetLocal<v8::Value> RuntimeEffect::getChildren()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!children_array_.IsEmpty())
        return children_array_.Get(isolate);

    std::vector<v8::Local<v8::Value>> js_children;
    js_children.reserve(effect_->children().size());

    for (const SkRuntimeEffect::Child& child : effect_->children())
        js_children.emplace_back(fill_child_spec(isolate, child));

    auto js_array = v8::Array::New(isolate, js_children.data(), js_children.size());
    children_array_.Reset(isolate, js_array);
    return js_array;
}

ffi::RetLocal<v8::Value> RuntimeEffect::findUniform(const std::string& name)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto found_itr = uniforms_map_.find(name);
    if (found_itr != uniforms_map_.end())
        return found_itr->second.Get(isolate);

    const SkRuntimeEffect::Uniform *uniform = effect_->findUniform(name);
    if (!uniform)
        return v8::Null(isolate);

    v8::Local<v8::Value> js_uniform = fill_uniform_spec(isolate, *uniform);
    uniforms_map_[name].Reset(isolate, js_uniform);
    return js_uniform;
}

ffi::RetLocal<v8::Value> RuntimeEffect::findChild(const std::string& name)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto found_itr = children_map_.find(name);
    if (found_itr != children_map_.end())
        return found_itr->second.Get(isolate);

    const SkRuntimeEffect::Child *child = effect_->findChild(name);
    if (!child)
        return v8::Null(isolate);

    v8::Local<v8::Value> js_child = fill_child_spec(isolate, *child);
    children_map_[name].Reset(isolate, js_child);
    return js_child;
}

GALLIUM_BINDINGS_RENDERER_NS_END
