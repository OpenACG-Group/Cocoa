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

#include <unordered_map>

#include "Gallium/bindings/glamor/CkPathMeasureWrap.h"
#include "Gallium/bindings/glamor/CkMatrixWrap.h"
#include "Gallium/bindings/glamor/CkPathWrap.h"
#include "Gallium/bindings/glamor/TrivialInterface.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

CkPathMeasureWrap::CkPathMeasureWrap(const SkPath& path, bool forceClosed, SkScalar resScale)
    : measure_(path, forceClosed, resScale)
{
}

v8::Local<v8::Value> CkPathMeasureWrap::Make(v8::Local<v8::Value> path, bool forceClosed, SkScalar resScale)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    CkPath *path_wrap = binder::Class<CkPath>::unwrap_object(isolate, path);
    if (!path_wrap)
        g_throw(TypeError, "Argument `path` must be an instance of `CkPath`");

    return binder::Class<CkPathMeasureWrap>::create_object(
            isolate, path_wrap->GetPath(), forceClosed, resScale);
}

void CkPathMeasureWrap::setPath(v8::Local<v8::Value> path, bool forceClosed)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    if (path->IsNullOrUndefined())
    {
        measure_.setPath(nullptr, forceClosed);
        return;
    }

    CkPath *path_wrap = binder::Class<CkPath>::unwrap_object(isolate, path);
    if (!path_wrap)
        g_throw(TypeError, "Argument `path` must be an instance of `CkPath`");

    measure_.setPath(&path_wrap->GetPath(), forceClosed);
}

SkScalar CkPathMeasureWrap::getLength()
{
    return measure_.getLength();
}

bool CkPathMeasureWrap::isClosed()
{
    return measure_.isClosed();
}

bool CkPathMeasureWrap::nextContour()
{
    return measure_.nextContour();
}

v8::Local<v8::Value> CkPathMeasureWrap::getPositionTangent(SkScalar distance)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    SkPoint vpos = SkPoint::Make(0, 0);
    SkPoint vtan = SkPoint::Make(0, 0);

    if (!measure_.getPosTan(distance, &vpos, &vtan))
        return v8::Null(isolate);

    using ObjectProtoMap = std::unordered_map<std::string_view, v8::Local<v8::Value>>;

    return binder::to_v8(isolate, ObjectProtoMap{
        { "position", NewCkPoint(isolate, vpos) },
        { "tangent",  NewCkPoint(isolate, vtan) }
    });
}

v8::Local<v8::Value> CkPathMeasureWrap::getMatrix(SkScalar distance, uint32_t flags)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    SkMatrix mat = SkMatrix::I();
    if (!measure_.getMatrix(distance, &mat, static_cast<SkPathMeasure::MatrixFlags>(flags)))
        return v8::Null(isolate);

    return binder::Class<CkMatrix>::create_object(isolate, mat);
}

v8::Local<v8::Value> CkPathMeasureWrap::getSegment(SkScalar startD, SkScalar stopD, bool startWithMoveTo)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    SkPath dst;

    if (!measure_.getSegment(startD, stopD, &dst, startWithMoveTo))
        return v8::Null(isolate);

    return binder::Class<CkPath>::create_object(isolate, dst);
}

GALLIUM_BINDINGS_GLAMOR_NS_END
