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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_PATH_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_PATH_H

#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"

#include "Gallium/bindings/renderer/Types.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/ArrayBuffer.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/Class.h"
#include "Gallium/bindings/renderer/Rect.h"
#include "Gallium/bindings/renderer/Matrix.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

class Paint;

//! TSDecl: @class Path
class Path : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kTransferable_Attr)

    //! TSDecl: @constructor()
    Path() = default;
    explicit Path(const SkPath& path) : path_(path) {}
    ~Path() override = default;

    g_nodiscard SkPath& GetSkPath() {
        return path_;
    }

    //! TSDecl: @method @static Make(points: @mem(f32), verbs: @mem(u8), weights: @mem(f32),
    //! TSDecl:                      fillType: PathFillType, isVolatile: boolean): Path
    static ffi::RetLocal<v8::Value> Make(const ffi::Mem<float>& points,
                                         const ffi::Mem<uint8_t>& verbs,
                                         const ffi::Mem<float>& weights,
                                         const ffi::Enum<SkPathFillType>& fill_type,
                                         bool is_volatile);

    //! TSDecl: @method @static Rect(rect: Rect, dir: PathDirection, startIndex: u32): Path
    static ffi::RetLocal<v8::Value> Rect(RectAdapter rect,
                                         const ffi::Enum<SkPathDirection>& dir,
                                         uint32_t start_index);

    //! TSDecl: @method @static Oval(rect: Rect, dir: PathDirection, startIndex: u32): Path
    static ffi::RetLocal<v8::Value> Oval(RectAdapter rect,
                                         const ffi::Enum<SkPathDirection>& dir,
                                         uint32_t start_index);

    //! TSDecl: @method @static Circle(centerX: f32, centerY: f32, radius: f32,
    //! TSDecl:                        dir: PathDirection): Path
    static ffi::RetLocal<v8::Value> Circle(float center_x, float center_y, float radius,
                                           const ffi::Enum<SkPathDirection>& dir);

    //! TSDecl: @method @static RRect(rrect: RRect, dir: PathDirection, startIndex: u32): Path
    static ffi::RetLocal<v8::Value> RRect(RRectAdapter rrect,
                                          const ffi::Enum<SkPathDirection>& dir,
                                          uint32_t start_index);

    //! TSDecl: @method @static Polygon(points: @mem(f32), isClosed: boolean, fillType: PathFillType,
    //! TSDecl:                         isVolatile: boolean): Path
    static ffi::RetLocal<v8::Value> Polygon(const ffi::Mem<float>& points,
                                            bool isClosed,
                                            const ffi::Enum<SkPathFillType>& fill_type,
                                            bool is_volatile);

    //! TSDecl: @method @static Line(x1: f32, y1: f32, x2: f32, y2: f32): Path
    static ffi::RetLocal<v8::Value> Line(float x1, float y1, float x2, float y2);

    //! TSDecl: @method clone(): Path
    ffi::RetLocal<v8::Value> clone();

    //! TSDecl: @method equalTo(other: Path): boolean
    ffi::Ret<bool> equalTo(const ffi::Class<Path>& other);

    //! TSDecl: @method isInterpolatable(compare: Path): boolean
    ffi::Ret<bool> isInterpolatable(const ffi::Class<Path>& compare);

    //! TSDecl: @method interpolate(ending: Path, weight: f32): @union(Path, null)
    ffi::RetLocal<v8::Value> interpolate(const ffi::Class<Path>& ending, float weight);

    //! TSDecl: @property fillType: PathFillType
    ffi::Ret<ffi::Enum<SkPathFillType>> getFillType() {
        return path_.getFillType();
    }
    ffi::Ret<void> setFillType(ffi::Enum<SkPathFillType> ft) {
        path_.setFillType(*ft);
        return {};
    }

    //! TSDecl: @method toggleInverseFillType(): void
    ffi::Ret<void> toggleInverseFillType();

    //! TSDecl: @property @readonly isConvex: boolean
    ffi::Ret<bool> getIsConvex() {
        return path_.isConvex();
    }

    //! TSDecl: @method asOval(): @union(Rect, null)
    ffi::RetLocal<v8::Value> asOval();

    //! TSDecl: @method asRRect(): @union(RRect, null)
    ffi::RetLocal<v8::Value> asRRect();

    //! TSDecl: @method reset(): void
    ffi::Ret<void> reset();

    //! TSDecl: @method rewind(): void
    ffi::Ret<void> rewind();

    //! TSDecl: @property @readonly isEmpty: boolean
    ffi::Ret<bool> getIsEmpty() {
        return path_.isEmpty();
    }

    //! TSDecl: @property @readonly isLastContourClosed: boolean
    ffi::Ret<bool> getIsLastContourClosed() {
        return path_.isLastContourClosed();
    }

    //! TSDecl: @property @readonly isFinite: boolean
    ffi::Ret<bool> getIsFinite() {
        return path_.isFinite();
    }

    //! TSDecl: @property isVolatile: boolean
    ffi::Ret<bool> getIsVolatile() {
        return path_.isVolatile();
    }
    ffi::Ret<void> setIsVolatile(bool v) {
        path_.setIsVolatile(v);
        return {};
    }

    //! TSDecl: @method countPoints(): i32
    ffi::Ret<int32_t> countPoints();

    //! TSDecl: @method getPoint(index: i32): @tuple(f32, f32)
    ffi::Ret<std::tuple<float, float>> getPoint(int32_t index);

    //! TSDecl: @method getPoints(dst: @mem(f32), maxPointNum: i32): i32
    ffi::Ret<int32_t> getPoints(const ffi::Mem<float>& dst, int32_t max);

    //! TSDecl: @method countVerbs(): i32
    ffi::Ret<int32_t> countVerbs();

    //! TSDecl: @method getVerbs(dst: @mem(u8), maxVerbNum: i32): i32
    ffi::Ret<int32_t> getVerbs(const ffi::Mem<uint8_t>& dst, int32_t max);

    //! TSDecl: @property @readonly roughBounds: Rect
    ffi::RetLocal<v8::Value> getRoughBounds();

    //! TSDecl: @method computeTightBounds(): Rect
    ffi::RetLocal<v8::Value> computeTightBounds();

    //! TSDecl: @method conservativelyContainsRect(rect: Rect): boolean
    ffi::Ret<bool> conservativelyContainsRect(RectAdapter rect);

    //! TSDecl: @method asRect(): @union(@tuple(Rect, boolean, PathDirection), null)
    ffi::RetLocal<v8::Value> asRect();

    //! TSDecl: @method addPathOffset(src: Path, dx: f32, dy: f32, mode: AddPathMode): void
    ffi::Ret<void> addPathOffset(const ffi::Class<Path>& src, float dx, float dy,
                                 const ffi::Enum<SkPath::AddPathMode>& mode);

    //! TSDecl: @method addPath(src: Path, matrix: Mat3x3, mode: AddPathMode): void
    ffi::Ret<void> addPath(const ffi::Class<Path>& src, const Mat3x3Adapter& matrix,
                           const ffi::Enum<SkPath::AddPathMode>& mode);

    //! TSDecl: @method reverseAddPath(src: Path): void
    ffi::Ret<void> reverseAddPath(const ffi::Class<Path>& src);

    //! TSDecl: @method transform(matrix: Mat3x3, perspectiveClip: boolean): void
    ffi::Ret<void> transform(const Mat3x3Adapter& matrix, bool perspective_clip);

    //! TSDecl: @method makeTransform(matrix: Mat3x3, perspectiveClip: boolean): Path
    ffi::RetLocal<v8::Value> makeTransform(const Mat3x3Adapter& matrix, bool perspective_clip);

    //! TSDecl: @property @readonly segmentMasks: u32
    ffi::Ret<uint32_t> getSegmentMasks() {
        return path_.getSegmentMasks();
    }

    // TODO(sora): implement the path iterator (SkPath::Iter)

    //! TSDecl: @method contains(x: f32, y: f32): boolean
    ffi::Ret<bool> contains(float x, float y);

    //! TSDecl: @method fillWithPaint(paint: Paint, cull: @union(Rect, null),
    //! TSDecl:                       ctm: @union(Mat3x3, null)): @union(Path, null)
    ffi::RetLocal<v8::Value> fillWithPaint(const ffi::Class<Paint>& paint,
                                           const ffi::Opt<RectAdapter>& cull,
                                           const ffi::Opt<Mat3x3Adapter>& ctm);

    //! TSDecl: @method serialize(): @mem(u8)
    ffi::Ret<ffi::Mem<uint8_t>> serialize();

    //! TSDecl: @method serializeToMemory(buffer: @mem(u8)): u64
    ffi::Ret<size_t> serializeToMemory(const ffi::Mem<uint8_t>& buffer);

    //! TSDecl: @method @static Deserialize(buffer: @mem(u8)): @tuple(@union(Path, null), u64)
    static ffi::Ret<std::tuple<v8::Local<v8::Value>, size_t>>
    Deserialize(const ffi::Mem<uint8_t>& buffer);

    //! TSDecl: @property @readonly generationID: u32
    ffi::Ret<uint32_t> getGenerationID() {
        return path_.getGenerationID();
    }

    //! TSDecl: @method isValid(): boolean
    ffi::Ret<bool> isValid();

private:
    std::unique_ptr<ffi::JSTransferData> OnObjectClone(v8::Isolate *isolate) override;

    SkPath path_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_PATH_H
