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

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_CKTEXTBLOB_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_CKTEXTBLOB_H

#include "include/core/SkTextBlob.h"
#include "include/v8.h"

#include "Gallium/bindings/ExportableObjectBase.h"
#include "Gallium/bindings/glamor/Types.h"
#include "Gallium/bindings/glamor/TrivialInterface.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

//! TSDecl: class CkTextBlob
class CkTextBlob : public ExportableObjectBase,
                   public SkiaObjectWrapper<SkTextBlob>
{
public:
    using SkiaObjectWrapper::SkiaObjectWrapper;

    //! TSDecl: function MakeFromText(text: Uint8Array, font: CkFont,
    //!                               encoding: Enum<TextEncoding>): CkTextBlob
    static v8::Local<v8::Value> MakeFromText(v8::Local<v8::Value> text,
                                             v8::Local<v8::Value> font,
                                             int32_t encoding);

    //! TSDecl: function MakeFromPosTextH(text: Uint8Array, xpos: Float32Array,
    //!                                   constY: number, font: CkFont,
    //!                                   encoding: Enum<TextEncoding>): CkTextBlob
    static v8::Local<v8::Value> MakeFromPosTextH(v8::Local<v8::Value> text,
                                                 v8::Local<v8::Value> xpos,
                                                 SkScalar constY,
                                                 v8::Local<v8::Value> font,
                                                 int32_t encoding);

    //! TSDecl: function MakeFromPosText(text: Uint8Array, pos: Array<CkPoint>,
    //!                                  font: CkFont, encoding: Enum<TextEncoding>): CkTextBlob
    static v8::Local<v8::Value> MakeFromPosText(v8::Local<v8::Value> text,
                                                v8::Local<v8::Value> pos,
                                                v8::Local<v8::Value> font,
                                                int32_t encoding);


    //! TSDecl: function MakeFromRSXformText(text: Uint8Array, forms: Array<CkRSXform>,
    //!                                      font: CkFont, encoding: Enum<TextEncoding>): CkTextBlob
    static v8::Local<v8::Value> MakeFromRSXformText(v8::Local<v8::Value> text,
                                                    v8::Local<v8::Value> forms,
                                                    v8::Local<v8::Value> font,
                                                    int32_t encoding);

    //! TSDecl: readonly bounds: CkRect
    v8::Local<v8::Value> getBounds();

    //! TSDecl: readonly uniqueID: number
    uint32_t getUniqueID();

    //! TSDecl: function getIntercepts(upperBound: number, lowerBound: number,
    //!                                paint: null | CkPaint): Float32Array
    v8::Local<v8::Value> getIntercepts(SkScalar upperBound, SkScalar lowerBound,
                                       v8::Local<v8::Value> paint);
};

//! TSDecl: class CkTextBlobBuilder
class CkTextBlobBuilder
{
public:
    //! TSDecl: constructor()
    CkTextBlobBuilder() = default;
    ~CkTextBlobBuilder() = default;

    // TODO(sora): complete this

private:
    SkTextBlobBuilder builder_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_CKTEXTBLOB_H
