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

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_CKRUNTIMEEFFECT_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_CKRUNTIMEEFFECT_H

#include "include/effects/SkRuntimeEffect.h"
#include "include/v8.h"

#include "Gallium/bindings/ExportableObjectBase.h"
#include "Gallium/bindings/glamor/Types.h"
#include "Gallium/bindings/glamor/TrivialInterface.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

//! TSDecl: class CkRuntimeEffect
class CkRuntimeEffect : public ExportableObjectBase,
                        public SkiaObjectWrapper<SkRuntimeEffect>
{
public:
    using SkiaObjectWrapper::SkiaObjectWrapper;

    //! TSDecl:
    //! interface RTEffectUniform {
    //!   name: string;
    //!   offset: number;
    //!   type: Enum<RuntimeEffectUniformType>;
    //!   count: number;
    //!   flags: Bitfield<RuntimeEffectUniformFlag>;
    //!   sizeInBytes: number;
    //! }

    //! TSDecl:
    //! interface RTEffectChild {
    //!   name: string;
    //!   type: Enum<RuntimeEffectChildType>;
    //!   index: number;
    //! }

    //! TSDecl: function MakeForColorFilter(sksl: string, forceUnoptimized: boolean,
    //!                                     callback: (error: string) => void): CkRuntimeEffect | null
    static v8::Local<v8::Value> MakeForColorFilter(const std::string& sksl, bool forceUnopt,
                                                   v8::Local<v8::Value> callback);

    //! TSDecl: function MakeForShader(sksl: string, forceUnoptimized: boolean,
    //!                                callback: (error: string) => void): CkRuntimeEffect | null
    static v8::Local<v8::Value> MakeForShader(const std::string& sksl, bool forceUnopt,
                                              v8::Local<v8::Value> callback);

    //! TSDecl: function MakeForBlender(sksl: string, forceUnoptimized: boolean,
    //!                                     callback: (error: string) => void): CkRuntimeEffect | null
    static v8::Local<v8::Value> MakeForBlender(const std::string& sksl, bool forceUnopt,
                                               v8::Local<v8::Value> callback);


    //! TSDecl: function uniforms(): Array<RTEffectUniform>
    v8::Local<v8::Value> uniforms();

    //! TSDecl: function children(): Array<RTEffectChild>
    v8::Local<v8::Value> children();

    //! TSDecl: function findUniform(name: string): RTEffectUniform | null
    v8::Local<v8::Value> findUniform(const std::string& name);

    //! TSDecl: function findChild(name: string): RTEffectChild | null
    v8::Local<v8::Value> findChild(const std::string& name);

    //! TSDecl:
    //! interface RTEffectChildSpecifier {
    //!   shader?: CkShader;
    //!   blender?: CkBlender;
    //!   colorFilter?: CkColorFilter;
    //! }

    //! TSDecl: function makeShader(uniforms: Array<number>,
    //!                             children: Array<RTEffectChildSpecifier>,
    //!                             local_matrix: CkMat3x3 | null): CkShader | null
    v8::Local<v8::Value> makeShader(v8::Local<v8::Value> uniforms, v8::Local<v8::Value> children,
                                    v8::Local<v8::Value> local_matrix);

    //! TSDecl: function makeColorFilter(uniforms: Array<number>,
    //!                                  children: Array<RTEffectChildSpecifier>): CkColorFilter | null
    v8::Local<v8::Value> makeColorFilter(v8::Local<v8::Value> uniforms, v8::Local<v8::Value> children);

    //! TSDecl: function makeBlender(uniforms: Array<number>,
    //!                              children: Array<RTEffectChildSpecifier>): CkBlender | null
    v8::Local<v8::Value> makeBlender(v8::Local<v8::Value> uniforms, v8::Local<v8::Value> children);
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_CKRUNTIMEEFFECT_H
