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

#include "Gallium/bindings/renderer/PathBuilder.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

#define I v8::Isolate::GetCurrent()

ffi::RetLocal<v8::Value> PathBuilder::clone()
{
    return ffi::JSObject::New<PathBuilder>(I, builder_);
}

ffi::RetLocal<v8::Value> PathBuilder::computeBounds()
{
    return CreateJSRect(I, builder_.computeBounds());
}

ffi::RetLocal<v8::Value> PathBuilder::snapshot()
{
    return ffi::JSObject::New<Path>(I, builder_.snapshot());
}

ffi::RetLocal<v8::Value> PathBuilder::detach()
{
    return ffi::JSObject::New<Path>(I, builder_.detach());
}

ffi::RetLocal<v8::Value> PathBuilder::setFillType(const ffi::Enum<SkPathFillType>& ft)
{
    builder_.setFillType(*ft);
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value> PathBuilder::setIsVolatile(bool value)
{
    builder_.setIsVolatile(value);
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value> PathBuilder::reset()
{
    builder_.reset();
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value> PathBuilder::moveTo(float x, float y)
{
    builder_.moveTo(x, y);
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value> PathBuilder::lineTo(float x, float y)
{
    builder_.lineTo(x, y);
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value> PathBuilder::quadTo(float x1, float y1, float x2, float y2)
{
    builder_.quadTo(x1, y1, x2, y2);
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value> PathBuilder::conicTo(float x1, float y1, float x2, float y2, float w)
{
    builder_.conicTo(x1, y1, x2, y2, w);
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value> PathBuilder::cubicTo(float x1, float y1, float x2, float y2, float x3, float y3)
{
    builder_.cubicTo(x1, y1, x2, y2, x3, y3);
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value> PathBuilder::close()
{
    builder_.close();
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value> PathBuilder::rLineTo(float x, float y)
{
    builder_.rLineTo(x, y);
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value> PathBuilder::rQuadTo(float x1, float y1, float x2, float y2)
{
    builder_.rQuadTo(x1, y1, x2, y2);
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value> PathBuilder::rConicTo(float x1, float y1, float x2, float y2, float w)
{
    builder_.rConicTo(x1, y1, x2, y2, w);
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value> PathBuilder::rCubicTo(float x1, float y1, float x2, float y2, float x3, float y3)
{
    builder_.rCubicTo(x1, y1, x2, y2, x3, y3);
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value> PathBuilder::ovalArcTo(RectAdapter oval, float start_angle_deg,
                                                float sweep_angle_deg, bool force_move_to)
{
    builder_.arcTo(*oval, start_angle_deg, sweep_angle_deg, force_move_to);
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value> PathBuilder::tangentArcTo(float x1, float y1, float x2, float y2, float radius)
{
    builder_.arcTo({x1, y1}, {x2, y2}, radius);
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value> PathBuilder::rotateOvalArcTo(float rx, float ry, float rotation_deg, bool large_arc,
                                                      const ffi::Enum<SkPathDirection>& sweep, float x1, float y1)
{
    builder_.arcTo({rx, ry}, rotation_deg,
                   large_arc ? SkPathBuilder::ArcSize::kLarge_ArcSize
                             : SkPathBuilder::ArcSize::kSmall_ArcSize,
                   *sweep, {x1, y1});
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value> PathBuilder::addArc(RectAdapter oval, float start_angle_deg, float sweep_angle_deg)
{
    builder_.addArc(*oval, start_angle_deg, sweep_angle_deg);
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value>
PathBuilder::addRect(RectAdapter rect, const ffi::Enum<SkPathDirection>& dir, uint32_t start_index)
{
    builder_.addRect(*rect, *dir, start_index);
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value>
PathBuilder::addOval(RectAdapter oval, const ffi::Enum<SkPathDirection>& dir, uint32_t start_index)
{
    builder_.addRect(*oval, *dir, start_index);
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value>
PathBuilder::addRRect(RRectAdapter rrect, const ffi::Enum<SkPathDirection>& dir, uint32_t start_index)
{
    builder_.addRRect(*rrect, *dir, start_index);
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value>
PathBuilder::addCircle(float cx, float cy, float radius, const ffi::Enum<SkPathDirection>& dir)
{
    builder_.addCircle(cx, cy, radius, *dir);
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value> PathBuilder::addPolygon(const ffi::Mem<float>& pts, bool is_closed)
{
    if (pts.Size() & 1)
        return ffi::Fail(ffi::kErr, "invalid length of array as flattened points");

    builder_.addPolygon(reinterpret_cast<SkPoint*>(pts.Address()), pts.Size() >> 1, is_closed);
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value> PathBuilder::addPath(const ffi::Class<Path>& path)
{
    builder_.addPath((*path)->GetSkPath());
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value> PathBuilder::incReserve(int32_t extra_pt_count, int32_t extra_verb_count)
{
    builder_.incReserve(extra_pt_count, extra_verb_count);
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value> PathBuilder::offset(float dx, float dy)
{
    builder_.offset(dx, dy);
    return GetThisHandle(I);
}

ffi::RetLocal<v8::Value> PathBuilder::toggleInverseFillType()
{
    builder_.toggleInverseFillType();
    return GetThisHandle(I);
}

GALLIUM_BINDINGS_RENDERER_NS_END
