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

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_CKPATHEFFECTWRAP_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_CKPATHEFFECTWRAP_H

#include "include/core/SkPathEffect.h"
#include "include/v8.h"

#include "Gallium/bindings/glamor/TrivialSkiaExportedTypes.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

class CkPathEffect : public SkiaObjectWrapper<SkPathEffect>
{
public:
    using SkiaObjectWrapper::SkiaObjectWrapper;

    //! TSDecl: function MakeFromDSL(dsl: string, kwargs: object): CkPathEffect
    static v8::Local<v8::Value> MakeFromDSL(v8::Local<v8::Value> dsl,
                                            v8::Local<v8::Value> kwargs);
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_CKPATHEFFECTWRAP_H
