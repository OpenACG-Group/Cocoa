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

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_CKPATHWRAP_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_CKPATHWRAP_H

#include "include/core/SkPath.h"
#include "include/v8.h"

#include "Gallium/bindings/ExportableObjectBase.h"
#include "Gallium/bindings/glamor/Types.h"
#include "Gallium/bindings/glamor/TrivialInterface.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

class CkPath : public ExportableObjectBase
{
public:
    explicit CkPath(const SkPath& path) : path_(path) {}
    ~CkPath() = default;

    g_nodiscard g_inline SkPath& GetPath() {
        return path_;
    }

    //! TSDecl: constructor()
    CkPath() = default;

    //! TSDecl: function IsLineDegenerate(p1: CkPoint, p2: CkPoint, exact: boolean): boolean
    static bool IsLineDegenerate(v8::Local<v8::Value> p1, v8::Local<v8::Value> p2, bool exact);

    //! TSDecl: function IsQuadDegenerate(p1: CkPoint, p2: CkPoint, p3: CkPoint, exact: boolean): boolean
    static bool IsQuadDegenerate(v8::Local<v8::Value> p1, v8::Local<v8::Value> p2,
                                 v8::Local<v8::Value> p3, bool exact);

    //! TSDecl: function IsCubicDegenerate(p1: CkPoint, p2: CkPoint, p3: CkPoint,
    //!                                    p4: CkPoint, exact: boolean): boolean
    static bool IsCubicDegenerate(v8::Local<v8::Value> p1, v8::Local<v8::Value> p2,
                                  v8::Local<v8::Value> p3, v8::Local<v8::Value> p4,
                                  bool exact);

    //! TSDecl: function clone(): CkPath
    v8::Local<v8::Value> clone();

    //! TSDecl: function isInterpolatable(compare: CkPath): boolean
    bool isInterpolatable(v8::Local<v8::Value> compare);

    //! TSDecl: function interpolate(ending: CkPath, weight: number): CkPath
    v8::Local<v8::Value> interpolate(v8::Local<v8::Value> ending, SkScalar weight);

    //! TSDecl: function setFillType(ft: Enum<PathFillType>): void
    void setFillType(int32_t ft);

    //! TSDecl: function toggleInverseFillType(): void
    void toggleInverseFillType();

    //! TSDecl: function isConvex(): boolean
    bool isConvex();

    //! TSDecl: function reset(): void
    void reset();

    //! TSDecl: function rewind(): void
    void rewind();

    //! TSDecl: function isEmpty(): boolean
    bool isEmpty();

    //! TSDecl: function isLastContourClosed(): boolean
    bool isLastContourClosed();

    //! TSDecl: function isFinite(): boolean
    bool isFinite();

    //! TSDecl: function isVolatile(): boolean
    bool isVolatile();

    //! TSDecl: function setIsVolatile(volatile: boolean): void
    void setIsVolatile(bool volatile_);

    //! TSDecl: function countPoints(): number
    int32_t countPoints();

    //! TSDecl: function getPoint(index: number): CkPoint
    v8::Local<v8::Value> getPoint(int32_t index);

    //! TSDecl: function getBounds(): CkRect
    v8::Local<v8::Value> getBounds();

    //! TSDecl: function computeTightBounds(): CkRect
    v8::Local<v8::Value> computeTightBounds();

    //! TSDecl: function conservativelyContainsRect(rect: CkRect): boolean
    bool conservativelyContainsRect(v8::Local<v8::Value> rect);

    //! TSDecl: function moveTo(x: number, y: number): void
    void moveTo(SkScalar x, SkScalar y);

    //! TSDecl: function rMoveTo(dx: number, dy: number): void
    void rMoveTo(SkScalar dx, SkScalar dy);

    //! TSDecl: function lineTo(x: number, y: number): void
    void lineTo(SkScalar x, SkScalar y);

    //! TSDecl: function rLineTo(dx: number, dy: number): void
    void rLineTo(SkScalar dx, SkScalar dy);

    //! TSDecl: function quadTo(x1: number, y1: number, x2: number, y2: number): void
    void quadTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2);

    //! TSDecl: function rQuadTo(dx1: number, dy1: number, dx2: number, dy2: number): void
    void rQuadTo(SkScalar dx1, SkScalar dy1, SkScalar dx2, SkScalar dy2);

    //! TSDecl: function conicTo(x1: number, y1: number, x2: number, y2: number,
    //!                          w: number): void
    void conicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                 SkScalar w);

    //! TSDecl: function rConicTo(dx1: number, dy1: number, dx2: number, dy2: number,
    //!                           w: number): void
    void rConicTo(SkScalar dx1, SkScalar dy1, SkScalar dx2, SkScalar dy2,
                  SkScalar w);

    //! TSDecl: function cubicTo(x1: number, y1: number, x2: number, y2: number,
    //!                          x3: number, y3: number): void
    void cubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                 SkScalar x3, SkScalar y3);

    //! TSDecl: function rCubicTo(dx1: number, dy1: number, dx2: number, dy2: number,
    //!                           dx3: number, dy3: number): void
    void rCubicTo(SkScalar dx1, SkScalar dy1, SkScalar dx2, SkScalar dy2,
                  SkScalar dx3, SkScalar dy3);

    //! TSDecl: function oaaArcTo(oval: CkRect, startAngle: number, sweepAngle: number,
    //!                           forceMoveTo: boolean): void
    void oaaArcTo(v8::Local<v8::Value> oval, SkScalar startAngle, SkScalar sweepAngle,
                  bool forceMoveTo);

    //! TSDecl: function pprArcTo(p1: CkPoint, p2: CkPoint, radius: number): void
    void pprArcTo(v8::Local<v8::Value> p1, v8::Local<v8::Value> p2, SkScalar radius);

    //! TSDecl: function pspArcTo(r: CkPoint, xAxisRotate: number, arc: Enum<ArcSize>,
    //!                           sweep: Enum<PathDirection>, xy: CkPoint): void
    void pspArcTo(v8::Local<v8::Value> r, SkScalar xAxisRotate, int32_t arc,
                  int32_t sweep, v8::Local<v8::Value> xy);

    //! TSDecl: function rPspArcTo(rx: number, ry: number, xAxisRotate: number, arc: Enum<ArcSize>,
    //!                            sweep: Enum<PathDirection>, dx: number, dy: number): void
    void rPspArcTo(SkScalar rx, SkScalar ry, SkScalar xAxisRotate, int32_t arc,
                   int32_t sweep, SkScalar dx, SkScalar dy);

    //! TSDecl: function close(): void
    void close();

    //! TSDecl: function addRect(rect: CkRect, dir: Enum<PathDirection>, start: number): void
    void addRect(v8::Local<v8::Value> rect, int32_t dir, int32_t start);

    //! TSDecl: function addOval(oval: CkRect, dir: Enum<PathDirection>, start: number): void
    void addOval(v8::Local<v8::Value> oval, int32_t dir, int32_t start);

    //! TSDecl: function addCircle(x: number, y: number, r: number, dir: Enum<PathDirection>): void
    void addCircle(SkScalar x, SkScalar y, SkScalar r, int32_t dir);

    //! TSDecl: function addArc(oval: CkRect, startAngle: number, sweepAngle: number): void
    void addArc(v8::Local<v8::Value> oval, SkScalar startAngle, SkScalar sweepAngle);

    //! TSDecl: function addRRect(rrect: CkRRect, dir: Enum<PathDirection>, start: number): void
    void addRRect(v8::Local<v8::Value> rrect, int32_t dir, int32_t start);

    //! TSDecl: function addPoly(pts: Array<CkPoint>, close: boolean): void
    void addPoly(v8::Local<v8::Value> pts, bool close);

    //! TSDecl: function addPath(src: CkPath, dx: number, dy: number, mode: Enum<AddPathMode>): void
    void addPath(v8::Local<v8::Value> src, SkScalar dx, SkScalar dy, int32_t mode);

    //! TSDecl: function addPathMatrix(src: CkPath, matrix: CkMatrix, mode: Enum<AddPathMode>): void
    void addPathMatrix(v8::Local<v8::Value> src, v8::Local<v8::Value> matrix, int32_t mode);

    //! TSDecl: function reverseAddPath(src: CkPath): void
    void reverseAddPath(v8::Local<v8::Value> src);

    //! TSDecl: function offset(dx: number, dy: number): void
    void offset(SkScalar dx, SkScalar dy);

    //! TSDecl: function transform(matrix: CkMatrix, pc: Enum<ApplyPerspectiveClip>): void
    void transform(v8::Local<v8::Value> matrix, int32_t pc);

    //! TSDecl: function toString(hex: boolean): string
    std::string toString(bool hex);

private:
    SkPath path_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_CKPATHWRAP_H
