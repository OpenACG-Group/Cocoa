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

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_CKFONTMGR_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_CKFONTMGR_H

#include "include/core/SkFontMgr.h"
#include "include/v8.h"

#include "Gallium/bindings/ExportableObjectBase.h"
#include "Gallium/bindings/glamor/Types.h"
#include "Gallium/bindings/glamor/TrivialInterface.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

//! TSDecl: class CkFontStyleSet
class CkFontStyleSet : public ExportableObjectBase,
                       public SkiaObjectWrapper<SkFontStyleSet>
{
public:
    using SkiaObjectWrapper::SkiaObjectWrapper;

    //! TSDecl: function count(): number
    int count();

    //! TSDecl: function getStyle(index: number): CkFontStyle
    v8::Local<v8::Value> getStyle(int index);

    //! TSDecl: function getStyleName(index: number): string
    std::string getStyleName(int index);

    //! TSDecl: function createTypeface(index: number): CkTypeface | null
    v8::Local<v8::Value> createTypeface(int index);

    //! TSDecl: function matchStyle(pattern: CkFontStyle): CkTypeface | null
    v8::Local<v8::Value> matchStyle(v8::Local<v8::Value> pattern);
};

//! TSDecl: class CkFontMgr
class CkFontMgr : public ExportableObjectBase,
                  public SkiaObjectWrapper<SkFontMgr>
{
public:
    using SkiaObjectWrapper::SkiaObjectWrapper;

    //! TSDecl: function countFamilies(): number
    int countFamilies();

    //! TSDecl: function getFamilyName(index: number): string
    std::string getFamilyName(int index);

    //! TSDecl: function createStyleSet(index: number): CkFontStyleSet
    v8::Local<v8::Value> createStyleSet(int index);

    //! TSDecl: function matchFamilyStyle(familyName: string | null, style: CkFontStyle): CkTypeface | null
    v8::Local<v8::Value> matchFamilyStyle(v8::Local<v8::Value> family_name,
                                          v8::Local<v8::Value> style);

    //! TSDecl: function makeFromData(data: data: Uint8Array, ttcIndex: number): CkTypeface
    v8::Local<v8::Value> makeFromData(v8::Local<v8::Value> data, int32_t ttc_index);

    //! TSDecl: function makeFromFile(path: string, ttcIndex: number): CkTypeface
    v8::Local<v8::Value> makeFromFile(const std::string& path, int32_t ttc_index);
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_CKFONTMGR_H
