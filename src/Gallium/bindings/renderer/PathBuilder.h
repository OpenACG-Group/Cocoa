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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_PATHBUILDER_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_PATHBUILDER_H

#include "include/core/SkPathBuilder.h"

#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Class.h"
#include "Gallium/bindings/renderer/Types.h"
#include "Gallium/bindings/renderer/Path.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

//! TSDecl: @class PathBuilder
class PathBuilder : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    //! TSDecl: @constructor()
    PathBuilder() = default;

    explicit PathBuilder(const SkPathBuilder& builder) : builder_(builder) {}
    ~PathBuilder() override = default;

    g_nodiscard SkPathBuilder& GetSkPathBuilder() {
        return builder_;
    }

    //! TSDecl: @method clone(): PathBuilder
    ffi::RetLocal<v8::Value> clone();

    //! TSDecl: @method computeBounds(): Rect
    ffi::RetLocal<v8::Value> computeBounds();

    //! TSDecl: @method snapshot(): Path
    ffi::RetLocal<v8::Value> snapshot();

    //! TSDecl: @method detach(): Path
    ffi::RetLocal<v8::Value> detach();

    //! TSDecl: @method setFillType(ft: PathFillType): PathBuilder
    ffi::RetLocal<v8::Value> setFillType(const ffi::Enum<SkPathFillType>& ft);

    //! TSDecl: @method setIsVolatile(value: boolean): PathBuilder
    ffi::RetLocal<v8::Value> setIsVolatile(bool value);

    //! TSDecl: @method reset(): PathBuilder
    ffi::RetLocal<v8::Value> reset();

    //! TSDecl: @method moveTo(x: f32, y: f32): PathBuilder
    ffi::RetLocal<v8::Value> moveTo(float x, float y);

    //! TSDecl: @method lineTo(x: f32, y: f32): PathBuilder
    ffi::RetLocal<v8::Value> lineTo(float x, float y);

    //! TSDecl: @method quadTo(x1: f32, y1: f32, x2: f32, y2: f32): PathBuilder
    ffi::RetLocal<v8::Value> quadTo(float x1, float y1, float x2, float y2);

    //! TSDecl: @method conicTo(x1: f32, y1: f32, x2: f32, y2: f32, w: f32): PathBuilder
    ffi::RetLocal<v8::Value> conicTo(float x1, float y1, float x2, float y2, float w);

    //! TSDecl: @method cubicTo(x1: f32, y1: f32, x2: f32, y2: f32, x3: f32, y3: f32): PathBuilder
    ffi::RetLocal<v8::Value> cubicTo(float x1, float y1, float x2, float y2, float x3, float y3);

    //! TSDecl: @method close(): PathBuilder
    ffi::RetLocal<v8::Value> close();

    //! TSDecl: @method rLineTo(x: f32, y: f32): PathBuilder
    ffi::RetLocal<v8::Value> rLineTo(float x, float y);

    //! TSDecl: @method rQuadTo(x1: f32, y1: f32, x2: f32, y2: f32): PathBuilder
    ffi::RetLocal<v8::Value> rQuadTo(float x1, float y1, float x2, float y2);

    //! TSDecl: @method rConicTo(x1: f32, y1: f32, x2: f32, y2: f32, w: f32): PathBuilder
    ffi::RetLocal<v8::Value> rConicTo(float x1, float y1, float x2, float y2, float w);

    //! TSDecl: @method rCubicTo(x1: f32, y1: f32, x2: f32, y2: f32, x3: f32, y3: f32): PathBuilder
    ffi::RetLocal<v8::Value> rCubicTo(float x1, float y1, float x2, float y2, float x3, float y3);

    //! TSDecl: @method ovalArcTo(oval: Rect, startAngleDeg: f32, sweepAngleDeg: f32,
    //! TSDecl:                   forceMoveTo: boolean): PathBuilder
    ffi::RetLocal<v8::Value> ovalArcTo(RectAdapter oval, float start_angle_deg, float sweep_angle_deg,
                                       bool force_move_to);

    //! TSDecl: @method tangentArcTo(x1: f32, y1: f32, x2: f32, y2: f32, radius: f32): PathBuilder
    ffi::RetLocal<v8::Value> tangentArcTo(float x1, float y1, float x2, float y2, float radius);

    //! TSDecl: @method rotateOvalArcTo(rx: f32, ry: f32, rotationDeg: f32, largeArc: boolean,
    //! TSDecl:                         sweep: PathDirection, x1: f32, y1: f32): PathBuilder
    ffi::RetLocal<v8::Value> rotateOvalArcTo(float rx, float ry, float rotation_deg, bool large_arc,
                                             const ffi::Enum<SkPathDirection>& sweep, float x1, float y1);

    //! TSDecl: @method addArc(oval: Rect, startAngleDeg: f32, sweepAngleDeg: f32): PathBuilder
    ffi::RetLocal<v8::Value> addArc(RectAdapter oval, float start_angle_deg, float sweep_angle_deg);

    //! TSDecl: @method addRect(rect: Rect, dir: PathDirection, startIndex: u32): PathBuilder
    ffi::RetLocal<v8::Value> addRect(RectAdapter rect, const ffi::Enum<SkPathDirection>& dir,
                                     uint32_t start_index);

    //! TSDecl: @method addOval(oval: Rect, dir: PathDirection, startIndex: u32): PathBuilder
    ffi::RetLocal<v8::Value> addOval(RectAdapter oval, const ffi::Enum<SkPathDirection>& dir,
                                     uint32_t start_index);

    //! TSDecl: @method addRRect(rrect: RRect, dir: PathDirection, startIndex: u32): PathBuilder
    ffi::RetLocal<v8::Value> addRRect(RRectAdapter rrect, const ffi::Enum<SkPathDirection>& dir,
                                      uint32_t start_index);

    //! TSDecl: @method addCircle(cx: f32, cy: f32, radius: f32, dir: PathDirection): PathBuilder
    ffi::RetLocal<v8::Value> addCircle(float cx, float cy, float radius,
                                       const ffi::Enum<SkPathDirection>& dir);

    //! TSDecl: @method addPolygon(pts: @mem(f32), isClosed: boolean): PathBuilder
    ffi::RetLocal<v8::Value> addPolygon(const ffi::Mem<float>& pts, bool is_closed);

    //! TSDecl: @method addPath(path: Path): PathBuilder
    ffi::RetLocal<v8::Value> addPath(const ffi::Class<Path>& path);

    //! TSDecl: @method incReserve(extraPtCount: i32, extraVerbCount: i32): PathBuilder
    ffi::RetLocal<v8::Value> incReserve(int32_t extra_pt_count, int32_t extra_verb_count);

    //! TSDecl: @method offset(dx: f32, dy: f32): PathBuilder
    ffi::RetLocal<v8::Value> offset(float dx, float dy);

    //! TSDecl: @method toggleInverseFillType(): PathBuilder
    ffi::RetLocal<v8::Value> toggleInverseFillType();

private:
    SkPathBuilder builder_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_PATHBUILDER_H
