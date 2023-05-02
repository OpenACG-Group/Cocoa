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

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_CKVERTICES_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_CKVERTICES_H

#include "include/core/SkVertices.h"
#include "include/v8.h"

#include "Gallium/bindings/glamor/TrivialInterface.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

class CkVertices : public SkiaObjectWrapper<SkVertices>
{
public:
    explicit CkVertices(const sk_sp<SkVertices>& vertices);
    ~CkVertices();

    //! TSDecl: function MakeCopy(mode: Enum<VerticesVertexMode>,
    //!                           positions: Float32Array,
    //!                           texCoords: Float32Array | null,
    //!                           colors: Uint32Array | null,
    //!                           indices: Uint16Array | null): CkVertices
    static v8::Local<v8::Value> MakeCopy(int32_t mode, v8::Local<v8::Value> positions,
                                         v8::Local<v8::Value> texCoords,
                                         v8::Local<v8::Value> colors,
                                         v8::Local<v8::Value> indices);

    //! TSDecl: readonly uniqueID: number
    g_inline uint32_t getUniqueID() {
        return GetSkObject()->uniqueID();
    }

    //! TSDecl: readonly bounds: CkRect
    v8::Local<v8::Value> getBounds();

private:
    ssize_t approximate_size_bytes_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_CKVERTICES_H
