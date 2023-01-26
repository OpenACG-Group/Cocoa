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

#include "include/core/SkRefCnt.h"
#include "include/core/SkRect.h"
#include "include/core/SkRRect.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkColor.h"
#include "include/core/SkPoint3.h"

#include "Core/Project.h"
#include "Gallium/bindings/glamor/Types.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

enum class Sampling : uint32_t
{
    kNearest = 0,
    kLinear,
    kCubicMitchell,
    kCubicCatmullRom,

    kLast = kCubicCatmullRom
};

SkSamplingOptions SamplingToSamplingOptions(int32_t v);

//! TSDecl: Array<number> [x, y, w, h]
//!         or Float32Array [x, y, w, h]
//!         or interface { x: number, y: number, width: number, height: number }
//!         or interface { top: number, left: number, right: number, bottom: number }
SkRect ExtractCkRect(v8::Isolate *isolate, v8::Local<v8::Value> object);
v8::Local<v8::Value> WrapCkRect(v8::Isolate *isolate, const SkRect& rect);

SkRRect ExtractCkRRect(v8::Isolate *isolate, v8::Local<v8::Value> object);

enum class ColorSpace : uint32_t
{
    kUnknown,
    kSRGB,

    kLast = kSRGB
};

sk_sp<SkColorSpace> ExtrackCkColorSpace(int32_t v);

//! TSDecl: Array<number> [R, G, B, A] where R,G,B,A∈[0, 1]
SkColor4f ExtractColor4f(v8::Isolate *isolate, v8::Local<v8::Value> color);
v8::Local<v8::Value> WrapColor4f(v8::Isolate *isolate, const SkColor4f& color);

//! TSDecl: Array<number> [x, y]
SkPoint ExtractCkPoint(v8::Isolate *isolate, v8::Local<v8::Value> point);
v8::Local<v8::Value> WrapCkPoint(v8::Isolate *isolate, const SkPoint& p);

//! TSDecl: Array<number> [x, y, z]
SkPoint3 ExtractCkPoint3(v8::Isolate *isolate, v8::Local<v8::Value> point3);
v8::Local<v8::Value> WrapCkPoint3(v8::Isolate *isolate, const SkPoint3& point3);

SkColorType ExtractCkColorType(int32_t v);
SkAlphaType ExtractCkAlphaType(int32_t v);

//! TSDecl:
//! interface CkImageInfo {
//!   alphaType: number;
//!   colorType: number;
//!   colorSpace: number;
//!   width: number;
//!   height: number;
//! }
SkImageInfo ExtractCkImageInfo(v8::Isolate *isolate, v8::Local<v8::Value> object);
v8::Local<v8::Value> WrapCkImageInfo(v8::Isolate *isolate, const SkImageInfo& info);

template<typename T>
class SkiaObjectWrapper
{
public:
    using ValueType = sk_sp<T>;
    explicit SkiaObjectWrapper(ValueType value)
            : wrapped_value_(std::move(value)) {}
    ~SkiaObjectWrapper() = default;

    g_nodiscard g_inline const ValueType& getSkiaObject() const {
        return wrapped_value_;
    }

private:
    ValueType   wrapped_value_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_TRIVIALSKIAEXPORTEDTYPES_H
