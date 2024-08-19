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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_RUNTIMEEFFECT_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_RUNTIMEEFFECT_H

#include "include/effects/SkRuntimeEffect.h"

#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/ArrayBuffer.h"
#include "Gallium/bindings/renderer/Types.h"
#include "Gallium/bindings/renderer/Matrix.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

//! TSDecl: @interface SkSLReflectUniform
struct SkSLReflectUniform
{
    //! TSDecl: @property @readonly name: string
    std::string name;

    //! TSDecl: @property @readonly offset: u32
    uint32_t offset;

    //! TSDecl: @property @readonly sizeInBytes: u32
    uint32_t size_in_bytes;

    //! TSDecl: @property @readonly type: SkSLUniformType
    ffi::Enum<SkRuntimeEffect::Uniform::Type> type;

    //! TSDecl: @property @readonly count: i32
    int32_t count;

    //! TSDecl: @property @readonly flags: u32
    uint32_t flags;
};
//! TSDecl: @end

//! TSDecl: @interface SkSLReflectChild
struct SkSLReflectChild
{
    //! TSDecl: @property @readonly name: string
    std::string name;

    //! TSDecl: @property @readonly type: SkSLChildType
    ffi::Enum<SkRuntimeEffect::ChildType> type;

    //! TSDecl: @property @readonly index: i32
    int32_t index;
};
//! TSDecl: @end

//! TSDecl: @typedef SkSLChild = @union(Shader, ColorFilter, Blender)

//! TSDecl: @class @nonconstructible RuntimeEffect
class RuntimeEffect : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    RuntimeEffect(v8::Local<v8::String> sksl_source, sk_sp<SkRuntimeEffect> effect);
    ~RuntimeEffect() override = default;

    //! TSDecl: @method @static CompileColorFilter(sksl: string): @tuple(RuntimeEffect, string)
    static ffi::RetLocal<v8::Value> CompileColorFilter(v8::Local<v8::String> sksl);

    //! TSDecl: @method @static CompileShader(sksl: string): @tuple(RuntimeEffect, string)
    static ffi::RetLocal<v8::Value> CompileShader(v8::Local<v8::String> sksl);

    //! TSDecl: @method @static CompileBlender(sksl: string): @tuple(RuntimeEffect, string)
    static ffi::RetLocal<v8::Value> CompileBlender(v8::Local<v8::String> sksl);

    //! TSDecl: @method makeShader(uniforms: @union(null, @mem(u8)), children: @array(SkSLChild),
    //! TSDecl:                    localMatrix: @union(Mat3x3, null)): Shader
    ffi::RetLocal<v8::Value> makeShader(const ffi::Opt<ffi::Mem<uint8_t>>& uniforms,
                                        const std::vector<v8::Local<v8::Object>>& children,
                                        const ffi::Opt<Mat3x3Adapter>& local_matrix);

    //! TSDecl: @method makeColorFilter(uniforms: @union(null, @mem(u8)),
    //! TSDecl:                         children: @array(SkSLChild)): ColorFilter
    ffi::RetLocal<v8::Value> makeColorFilter(const ffi::Opt<ffi::Mem<uint8_t>>& uniforms,
                                             const std::vector<v8::Local<v8::Object>>& children);

    //! TSDecl: @method makeBlender(uniforms: @union(null, @mem(u8)),
    //! TSDecl:                     children: @array(SkSLChild)): Blender
    ffi::RetLocal<v8::Value> makeBlender(const ffi::Opt<ffi::Mem<uint8_t>>& uniforms,
                                        const std::vector<v8::Local<v8::Object>>& children);

    //! TSDecl: @property @readonly source: string
    ffi::RetLocal<v8::Value> getSource() {
        return sksl_source_.Get(v8::Isolate::GetCurrent());
    }

    //! TSDecl: @property @readonly uniformsByteSize: u32
    ffi::Ret<uint32_t> getUniformsByteSize() {
        return effect_->uniformSize();
    }

    //! TSDecl: @property @readonly uniforms: @array(SkSLReflectUniform)
    ffi::RetLocal<v8::Value> getUniforms();

    //! TSDecl: @property @readonly children: @array(SkSLReflectChild)
    ffi::RetLocal<v8::Value> getChildren();

    //! TSDecl: @method findUniform(name: string): @union(SkSLReflectUniform, null)
    ffi::RetLocal<v8::Value> findUniform(const std::string& name);

    //! TSDecl: @method findChild(name: string): @union(SkSLReflectChild, null)
    ffi::RetLocal<v8::Value> findChild(const std::string& name);

    //! TSDecl: @property @readonly allowShader: boolean
    ffi::Ret<bool> getAllowShader() {
        return effect_->allowShader();
    }

    //! TSDecl: @property @readonly allowColorFilter: boolean
    ffi::Ret<bool> getAllowColorFilter() {
        return effect_->allowColorFilter();
    }

    //! TSDecl: @property @readonly allowBlender: boolean
    ffi::Ret<bool> getAllowBlender() {
        return effect_->allowBlender();
    }

private:
    sk_sp<SkRuntimeEffect> effect_;
    v8::Global<v8::String> sksl_source_;

    // Property caches:
    std::unordered_map<std::string, v8::Global<v8::Value>> uniforms_map_;
    std::unordered_map<std::string, v8::Global<v8::Value>> children_map_;
    v8::Global<v8::Array> uniforms_array_;
    v8::Global<v8::Array> children_array_;
};
//! TSDecl: @end


// These helper classes are defined and implemented in `renderer.js` file:
// SkSLShaderBuilder, SkSLColorFilterBuilder, SkSLBlenderBuilder

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_RUNTIMEEFFECT_H
