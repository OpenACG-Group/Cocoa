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

#include "include/core/SkStream.h"

#include "Gallium/binder/Class.h"
#include "Gallium/binder/ThrowExcept.h"
#include "Gallium/bindings/glamor/CkPathWrap.h"
#include "Gallium/bindings/glamor/CkMatrixWrap.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

#define EXTRACT_PATH_CHECKED(arg, result) \
    auto *result = binder::Class<CkPath>::unwrap_object(isolate, arg); \
    if (!result) {                       \
        g_throw(TypeError, "Argument `" #arg "` must be an instance of `CkPath`"); \
    }

#define CHECK_ENUM_RANGE(v, last) \
    if (v < 0 || v > static_cast<int32_t>(last)) { \
        g_throw(RangeError, "Invalid enumeration value for arguemnt `" #v "`"); \
    }

bool CkPath::IsLineDegenerate(v8::Local<v8::Value> p1, v8::Local<v8::Value> p2, bool exact)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return SkPath::IsLineDegenerate(ExtractCkPoint(isolate, p1),
                                    ExtractCkPoint(isolate, p2),
                                    exact);
}

bool CkPath::IsQuadDegenerate(v8::Local<v8::Value> p1,
                              v8::Local<v8::Value> p2,
                              v8::Local<v8::Value> p3,
                              bool exact)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return SkPath::IsQuadDegenerate(ExtractCkPoint(isolate, p1),
                                    ExtractCkPoint(isolate, p2),
                                    ExtractCkPoint(isolate, p3),
                                    exact);
}

bool CkPath::IsCubicDegenerate(v8::Local<v8::Value> p1,
                               v8::Local<v8::Value> p2,
                               v8::Local<v8::Value> p3,
                               v8::Local<v8::Value> p4,
                               bool exact)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return SkPath::IsCubicDegenerate(ExtractCkPoint(isolate, p1),
                                     ExtractCkPoint(isolate, p2),
                                     ExtractCkPoint(isolate, p3),
                                     ExtractCkPoint(isolate, p4),
                                     exact);
}

v8::Local<v8::Value> CkPath::clone()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return binder::Class<CkPath>::create_object(isolate, path_);
}

bool CkPath::isInterpolatable(v8::Local<v8::Value> compare)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_PATH_CHECKED(compare, path)
    return path_.isInterpolatable(path->path_);
}

v8::Local<v8::Value> CkPath::interpolate(v8::Local<v8::Value> ending, SkScalar weight)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_PATH_CHECKED(ending, path)

    SkPath result_path;
    if (!path_.interpolate(path->path_, weight, &result_path))
        g_throw(Error, "Path is not interpolatable");

    return binder::Class<CkPath>::create_object(isolate, result_path);
}

void CkPath::setFillType(int32_t ft)
{
    CHECK_ENUM_RANGE(ft, SkPathFillType::kInverseEvenOdd)
    path_.setFillType(static_cast<SkPathFillType>(ft));
}

void CkPath::toggleInverseFillType()
{
    path_.toggleInverseFillType();
}

bool CkPath::isConvex()
{
    return path_.isConvex();
}

void CkPath::reset()
{
    path_.reset();
}

void CkPath::rewind()
{
    path_.rewind();
}

bool CkPath::isEmpty()
{
    return path_.isEmpty();
}

bool CkPath::isLastContourClosed()
{
    return path_.isLastContourClosed();
}

bool CkPath::isFinite()
{
    return path_.isFinite();
}

bool CkPath::isVolatile()
{
    return path_.isVolatile();
}

void CkPath::setIsVolatile(bool volatile_)
{
    path_.setIsVolatile(volatile_);
}

int32_t CkPath::countPoints()
{
    return path_.countPoints();
}

v8::Local<v8::Value> CkPath::getPoint(int32_t index)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (index < 0 || index >= path_.countPoints())
        g_throw(RangeError, "Invalid point index");

    return NewCkPoint(isolate, path_.getPoint(index));
}

v8::Local<v8::Value> CkPath::getBounds()
{
    return NewCkRect(v8::Isolate::GetCurrent(), path_.getBounds());
}

v8::Local<v8::Value> CkPath::computeTightBounds()
{
    return NewCkRect(v8::Isolate::GetCurrent(), path_.computeTightBounds());
}

bool CkPath::conservativelyContainsRect(v8::Local<v8::Value> rect)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return path_.conservativelyContainsRect(ExtractCkRect(isolate, rect));
}

void CkPath::moveTo(SkScalar x, SkScalar y)
{
    path_.moveTo(x, y);
}

void CkPath::rMoveTo(SkScalar dx, SkScalar dy)
{
    path_.rMoveTo(dx, dy);
}

void CkPath::lineTo(SkScalar x, SkScalar y)
{
    path_.lineTo(x, y);
}

void CkPath::rLineTo(SkScalar dx, SkScalar dy)
{
    path_.rLineTo(dx, dy);
}

void CkPath::quadTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2)
{
    path_.quadTo(x1, y1, x2, y2);
}

void CkPath::rQuadTo(SkScalar dx1, SkScalar dy1, SkScalar dx2, SkScalar dy2)
{
    path_.rQuadTo(dx1, dy1, dx2, dy2);
}

void CkPath::conicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar w)
{
    path_.conicTo(x1, y1, x2, y2, w);
}

void CkPath::rConicTo(SkScalar dx1, SkScalar dy1, SkScalar dx2, SkScalar dy2, SkScalar w)
{
    path_.rConicTo(dx1, dy1, dx2, dy2, w);
}

void CkPath::cubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar x3, SkScalar y3)
{
    path_.cubicTo(x1, y1, x2, y2, x3, y3);
}

void CkPath::rCubicTo(SkScalar dx1, SkScalar dy1, SkScalar dx2, SkScalar dy2, SkScalar dx3, SkScalar dy3)
{
    path_.rCubicTo(dx1, dy1, dx2, dy2, dx3, dy3);
}

void CkPath::oaaArcTo(v8::Local<v8::Value> oval, SkScalar startAngle, SkScalar sweepAngle, bool forceMoveTo)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    path_.arcTo(ExtractCkRect(isolate, oval), startAngle, sweepAngle, forceMoveTo);
}

void CkPath::pprArcTo(v8::Local<v8::Value> p1, v8::Local<v8::Value> p2, SkScalar radius)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    path_.arcTo(ExtractCkPoint(isolate, p1), ExtractCkPoint(isolate, p2), radius);
}

void CkPath::pspArcTo(v8::Local<v8::Value> r, SkScalar xAxisRotate,
                      int32_t arc, int32_t sweep, v8::Local<v8::Value> xy)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    CHECK_ENUM_RANGE(arc, SkPath::ArcSize::kLarge_ArcSize)
    CHECK_ENUM_RANGE(sweep, SkPathDirection::kCCW)

    path_.arcTo(ExtractCkPoint(isolate, r), xAxisRotate,
                static_cast<SkPath::ArcSize>(arc), static_cast<SkPathDirection>(sweep),
                ExtractCkPoint(isolate, xy));
}

void CkPath::rPspArcTo(SkScalar rx, SkScalar ry, SkScalar xAxisRotate,
                       int32_t arc, int32_t sweep, SkScalar dx, SkScalar dy)
{
    CHECK_ENUM_RANGE(arc, SkPath::ArcSize::kLarge_ArcSize)
    CHECK_ENUM_RANGE(sweep, SkPathDirection::kCCW)

    path_.rArcTo(rx, ry, xAxisRotate, static_cast<SkPath::ArcSize>(arc),
                 static_cast<SkPathDirection>(sweep), dx, dy);
}

void CkPath::close()
{
    path_.close();
}

void CkPath::addRect(v8::Local<v8::Value> rect, int32_t dir, int32_t start)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_ENUM_RANGE(dir, SkPathDirection::kCCW)
    path_.addRect(ExtractCkRect(isolate, rect), static_cast<SkPathDirection>(dir), start);
}

void CkPath::addOval(v8::Local<v8::Value> oval, int32_t dir, int32_t start)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_ENUM_RANGE(dir, SkPathDirection::kCCW)
    path_.addOval(ExtractCkRect(isolate, oval), static_cast<SkPathDirection>(dir), start);
}

void CkPath::addCircle(SkScalar x, SkScalar y, SkScalar r, int32_t dir)
{
    CHECK_ENUM_RANGE(dir, SkPathDirection::kCCW)
    path_.addCircle(x, y, r, static_cast<SkPathDirection>(dir));
}

void CkPath::addArc(v8::Local<v8::Value> oval, SkScalar startAngle, SkScalar sweepAngle)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    path_.addArc(ExtractCkRect(isolate, oval), startAngle, sweepAngle);
}

void CkPath::addRRect(v8::Local<v8::Value> rrect, int32_t dir, int32_t start)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_ENUM_RANGE(dir, SkPathDirection::kCCW)
    path_.addRRect(ExtractCkRRect(isolate, rrect), static_cast<SkPathDirection>(dir), start);
}

void CkPath::addPoly(v8::Local<v8::Value> pts, bool close)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!pts->IsArray())
        g_throw(TypeError, "Argument `pts` must be an array of `CkPoint`");

    auto array = v8::Local<v8::Array>::Cast(pts);
    if (array->Length() == 0)
        return;

    auto pod = std::make_unique<SkPoint[]>(array->Length());

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    for (int32_t i = 0; i < array->Length(); i++)
    {
        v8::Local<v8::Value> v = array->Get(ctx, i).ToLocalChecked();
        pod[i] = ExtractCkPoint(isolate, v);
    }

    path_.addPoly(pod.get(), static_cast<int32_t>(array->Length()), close);
}

void CkPath::addPath(v8::Local<v8::Value> src, SkScalar dx, SkScalar dy, int32_t mode)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_ENUM_RANGE(mode, SkPath::AddPathMode::kExtend_AddPathMode)
    EXTRACT_PATH_CHECKED(src, path)
    path_.addPath(path->path_, dx, dy, static_cast<SkPath::AddPathMode>(mode));
}

void CkPath::addPathMatrix(v8::Local<v8::Value> src, v8::Local<v8::Value> matrix, int32_t mode)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_ENUM_RANGE(mode, SkPath::AddPathMode::kExtend_AddPathMode)
    EXTRACT_PATH_CHECKED(src, path)

    auto *m = binder::Class<CkMatrix>::unwrap_object(isolate, matrix);
    if (!m)
        g_throw(TypeError, "Argument `matrix` must be be an instance of `CkMatrix`");

    path_.addPath(path->path_, m->GetMatrix(), static_cast<SkPath::AddPathMode>(mode));
}

void CkPath::reverseAddPath(v8::Local<v8::Value> src)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_PATH_CHECKED(src, path)
    path_.reverseAddPath(path->path_);
}

void CkPath::offset(SkScalar dx, SkScalar dy)
{
    path_.offset(dx, dy);
}

void CkPath::transform(v8::Local<v8::Value> matrix, int32_t pc)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_ENUM_RANGE(pc, SkApplyPerspectiveClip::kYes)
    auto *m = binder::Class<CkMatrix>::unwrap_object(isolate, matrix);
    if (!m)
        g_throw(TypeError, "Argument `matrix` must be be an instance of `CkMatrix`");
    path_.transform(m->GetMatrix(), static_cast<SkApplyPerspectiveClip>(pc));
}

std::string CkPath::toString(bool hex)
{
    SkDynamicMemoryWStream stream;
    path_.dump(&stream, hex);

    sk_sp<SkData> data = stream.detachAsData();

    return {reinterpret_cast<const char*>(data->data()), data->size()};
}

GALLIUM_BINDINGS_GLAMOR_NS_END
