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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_MATRIX_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_MATRIX_H

#include "include/core/SkMatrix.h"
#include "include/core/SkM44.h"
#include "include/core/SkRSXform.h"

#include "Gallium/bindings/renderer/Types.h"
#include "Gallium/ffi/DataTypes.h"
#include "Gallium/ffi/ReturnValue.h"
#include "Gallium/ffi/ArrayBuffer.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

v8::Local<v8::Object> CreateJSMat3x3(v8::Isolate *isolate, const SkMatrix& from);
ffi::Ret<SkMatrix> UnwrapJSMat3x3(v8::Isolate *isolate, v8::Local<v8::Value> value);

class Mat3x3Adapter : public ffi::ArgAdapter
{
public:
    static ffi::Ret<Mat3x3Adapter> Cast(v8::Isolate *isolate,
                                        v8::Local<v8::Value> value);

    const SkMatrix& operator*() const {
        return matrix;
    }
    SkMatrix matrix;
};

v8::Local<v8::Object> CreateJSMat4x4(v8::Isolate *isolate, const SkM44& from);
ffi::Ret<SkM44> UnwrapJSMat4x4(v8::Isolate *isolate, v8::Local<v8::Value> value);

class Mat4x4Adapter : public ffi::ArgAdapter
{
public:
    static ffi::Ret<Mat4x4Adapter> Cast(v8::Isolate *isolate,
                                        v8::Local<v8::Value> value);

    const SkM44& operator*() const {
        return matrix;
    }
    SkM44 matrix;
};

class RSXformArrayAdapter : public ffi::ArgAdapter
{
    static_assert(sizeof(SkRSXform) == 4 * sizeof(float));

public:
    static ffi::Ret<RSXformArrayAdapter> Cast(v8::Isolate *isolate,
                                              v8::Local<v8::Value> value);

    ffi::Mem<float>     memory;
    size_t              count;
    SkRSXform          *address;
};

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_MATRIX_H
