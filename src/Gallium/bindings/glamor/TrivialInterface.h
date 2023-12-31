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

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_TRIVIALSKIAEXPORTEDTYPES_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_TRIVIALSKIAEXPORTEDTYPES_H

#include <utility>

#include "include/core/SkRefCnt.h"
#include "include/core/SkRect.h"
#include "include/core/SkRRect.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkColor.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkData.h"

#include "Core/Project.h"
#include "Core/Errors.h"
#include "Gallium/bindings/ExportableObjectBase.h"
#include "Gallium/bindings/glamor/Types.h"
#include "Gallium/binder/TypeTraits.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

/**
 * TrivialInterface - Basic data types and JS wrappers of Skia objects.
 * Those data types are usually small objects and it is relatively expensive
 * to create binding classes and export them into JS. They can be represented
 * by JS native data types like arrays and simple objects (only contain datas),
 * which is more lightweight and has less overhead when they are passed
 * between C++ and JS. For example, an `SkPoint` or `SkV2` type (2d vector),
 * can be represented by a JS array with 2 elements `[x, y]`.
 *
 * Each of those data types has a `Extract*` function and a `New*` function.
 * The former converts a JS object into its corresponding C++ native object,
 * and the latter creates the JS object from a given native object.
 *
 * Some types are union types, which means a native object can be represented
 * by more than one JS data type. For example, a JS array `Array<number>`, or
 * `Float32Array`, or `interface {x, y, width, height}`, all them can represent
 * an `SkRect` object. The `Extract*` function accepts all the types of union
 * type, and the `New*` function creates a JS value in the PREFERRED type.
 * The preferred type of a union type can be found in its TSDecl declaration.
 */

enum class Sampling : uint32_t
{
    kNearest = 0,
    kLinear,
    kCubicMitchell,
    kCubicCatmullRom,

    kLast = kCubicCatmullRom
};

SkSamplingOptions SamplingToSamplingOptions(int32_t v);

//! TSDecl: type CkRect = (preferred) Array<number> [x, y, w, h]
//!                     | Float32Array [x, y, w, h]
//!                     | interface { x: number, y: number, width: number, height: number }
//!                     | interface { top: number, left: number, right: number, bottom: number }
SkRect ExtractCkRect(v8::Isolate *isolate, v8::Local<v8::Value> object);
v8::Local<v8::Value> NewCkRect(v8::Isolate *isolate, const SkRect& rect);

SkRRect ExtractCkRRect(v8::Isolate *isolate, v8::Local<v8::Value> object);

enum class ColorSpace : uint32_t
{
    kUnknown,
    kSRGB,

    kLast = kSRGB
};

sk_sp<SkColorSpace> ExtrackCkColorSpace(int32_t v);

//! TSDecl: type CkColor4f = Array<number> [R, G, B, A] where R,G,B,A∈[0, 1]
SkColor4f ExtractColor4f(v8::Isolate *isolate, v8::Local<v8::Value> color);
v8::Local<v8::Value> NewColor4f(v8::Isolate *isolate, const SkColor4f& color);

//! TSDecl: type CkPoint = Array<number> [x, y]
SkPoint ExtractCkPoint(v8::Isolate *isolate, v8::Local<v8::Value> point);
v8::Local<v8::Value> NewCkPoint(v8::Isolate *isolate, const SkPoint& p);

//! TSDecl: type CkPoint3 = Array<number> [x, y, z]
SkPoint3 ExtractCkPoint3(v8::Isolate *isolate, v8::Local<v8::Value> point3);
v8::Local<v8::Value> NewCkPoint3(v8::Isolate *isolate, const SkPoint3& point3);

SkColorType ExtractCkColorType(int32_t v);
SkAlphaType ExtractCkAlphaType(int32_t v);


//! TSDecl: class CkImageInfo
class CkImageInfo : public ExportableObjectBase
{
public:
    explicit CkImageInfo(SkImageInfo info) : info_(std::move(info)) {}
    ~CkImageInfo() = default;

    // TODO(sora): support SkColorSpace

    //! TSDecl: function MakeSRGB(w: number, h: number, colorType: Enum<ColorType>,
    //!                           alphaType: Enum<AlphaType>): CkImageInfo
    static v8::Local<v8::Value> MakeSRGB(int32_t w, int32_t h, int32_t color_type,
                                         int32_t alpha_type);

    //! TSDecl: function MakeN32(w: number, h: number,
    //!                          alphaType: Enum<AlphaType>): CkImageInfo
    static v8::Local<v8::Value> MakeN32(int32_t w, int32_t h, int32_t alpha_type);

    //! TSDecl: function MakeS32(w: number, h: number,
    //!                          alphaType: Enum<AlphaType>): CkImageInfo
    static v8::Local<v8::Value> MakeS32(int32_t w, int32_t h, int32_t alpha_type);

    //! TSDecl: function MakeN32Premul(w: number, h: number): CkImageInfo
    static v8::Local<v8::Value> MakeN32Premul(int32_t w, int32_t h);

    //! TSDecl: function MakeA8(w: number, h: number): CkImageInfo
    static v8::Local<v8::Value> MakeA8(int32_t w, int32_t h);

    //! TSDecl: function MakeUnknown(w: number, h: number): CkImageInfo
    static v8::Local<v8::Value> MakeUnknown(int32_t w, int32_t h);

    SkImageInfo& GetWrapped() {
        return info_;
    }

    //! TSDecl: readonly alphaType: Enum<AlphaType>
    g_nodiscard int32_t getAlphaType() const {
        return info_.alphaType();
    }

    //! TSDecl: readonly colorType: Enum<ColorType>
    g_nodiscard int32_t getColorType() const {
        return info_.colorType();
    }

    //! TSDecl: readonly width: number
    g_nodiscard int32_t getWidth() const {
        return info_.width();
    }

    //! TSDecl: readonly height: number
    g_nodiscard int32_t getHeight() const {
        return info_.height();
    }

    //! TSDecl: readonly isEmpty: boolean
    g_nodiscard bool getIsEmpty() const {
        return info_.isEmpty();
    }

    //! TSDecl: readonly isOpaque: boolean
    g_nodiscard bool getIsOpaque() const {
        return info_.isOpaque();
    }

    //! TSDecl: function makeWH(w: number, h: number): CkImagInfo
    v8::Local<v8::Value> makeWH(int32_t w, int32_t h);

    //! TSDecl: function makeAlphaType(alphaType: Enum<AlphaType>): CkImageInfo
    v8::Local<v8::Value> makeAlphaType(int32_t type);

    //! TSDecl: function makeColorType(colorType: Enum<ColorType>): CkImageInfo
    v8::Local<v8::Value> makeColorType(int32_t type);

    //! TSDecl: readonly bytesPerPixel: number
    g_nodiscard int32_t getBytesPerPixel() const {
        return info_.bytesPerPixel();
    }

    //! TSDecl: readonly shiftPerPixel: number
    g_nodiscard int32_t getShiftPerPixel() const {
        return info_.shiftPerPixel();
    }

    //! TSDecl: readonly minRowBytes: number
    g_nodiscard size_t getMinRowBytes() const {
        return info_.minRowBytes();
    }

    //! TSDecl: function computeOffset(x: number, y: number, rowBytes: number): number
    g_nodiscard size_t computeOffset(int32_t x, int32_t y, size_t  row_bytes) const {
        return info_.computeOffset(x, y, row_bytes);
    }

    //! TSDecl: function equalsTo(other: CkImageInfo): boolean
    bool equalsTo(v8::Local<v8::Value> other);

    //! TSDecl: function computeByteSize(rowBytes: number): number
    g_nodiscard size_t computeByteSize(size_t row_bytes) const {
        return info_.computeByteSize(row_bytes);
    }

    //! TSDecl: function computeMinByteSize(): number
    g_nodiscard size_t computeMinByteSize() const {
        return info_.computeMinByteSize();
    }

    //! TSDecl: function validRowBytes(rowBytes: number): boolean
    g_nodiscard bool validRowBytes(size_t rowBytes) const {
        return info_.validRowBytes(rowBytes);
    }

private:
    SkImageInfo info_;
};

SkImageInfo ExtractCkImageInfo(v8::Isolate *isolate, v8::Local<v8::Value> object);
v8::Local<v8::Value> NewCkImageInfo(v8::Isolate *isolate, const SkImageInfo& info);

//! TSDecl:
//! interface CkRSXform {
//!   ssin: number;
//!   scos: number;
//!   tx: number;
//!   ty: number;
//! }
SkRSXform ExtractCkRSXform(v8::Isolate *isolate, v8::Local<v8::Value> object);
v8::Local<v8::Object> NewCkRSXform(v8::Isolate *isolate, const SkRSXform& from);


//! TSDecl: type CkMat3x3 = Float32Array [ <column-major-matrix> ]
//!                       | Array<number> [ <column-major-matrix> ]
//!                       | (preferred) CkMatrix
SkMatrix ExtractCkMat3x3(v8::Isolate *isolate, v8::Local<v8::Value> mat);
v8::Local<v8::Value> NewCkMat3x3(v8::Isolate *isolate, const SkMatrix& mat);

struct TAMemoryForSkData
{
    CO_NONASSIGNABLE(TAMemoryForSkData)
    CO_NONCOPYABLE(TAMemoryForSkData)
    std::shared_ptr<v8::BackingStore> store;
};

template<typename T>
sk_sp<SkData> MakeSkDataFromTypedArrayMem(const binder::TypedArrayMemory<T>& mem)
{
    auto *ctx = new TAMemoryForSkData{ mem.memory };
    return SkData::MakeWithProc(mem.ptr, mem.byte_size, [](const void*, void *ctx) {
        CHECK(ctx);
        delete static_cast<TAMemoryForSkData*>(ctx);
    }, ctx);
}

template<typename T>
class SkiaObjectWrapper
{
public:
    using ValueType = sk_sp<T>;
    explicit SkiaObjectWrapper(ValueType value)
            : wrapped_value_(std::move(value)) {}
    ~SkiaObjectWrapper() = default;

    g_nodiscard g_inline const ValueType& GetSkObject() const {
        return wrapped_value_;
    }

private:
    ValueType   wrapped_value_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_TRIVIALSKIAEXPORTEDTYPES_H
