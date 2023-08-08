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

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_CKMATRIXWRAP_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_CKMATRIXWRAP_H

#include "include/core/SkMatrix.h"
#include "include/v8.h"

#include "Core/Project.h"
#include "Gallium/bindings/ExportableObjectBase.h"
#include "Gallium/bindings/glamor/Types.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

class CkMatrix : public ExportableObjectBase
{
public:
    explicit CkMatrix(const SkMatrix& matrix) : matrix_(matrix) {}
    ~CkMatrix() = default;

    g_nodiscard g_inline SkMatrix& GetMatrix() {
        return matrix_;
    }

    //! TSDecl: function Identity(): CkMatrix
    static v8::Local<v8::Value> Identity();

    //! TSDecl: function Scale(sx: number, sy: number): CkMatrix
    static v8::Local<v8::Value> Scale(SkScalar sx, SkScalar sy);

    //! TSDecl: function Translate(dx: number, dy: number): CkMatrix
    static v8::Local<v8::Value> Translate(SkScalar dx, SkScalar dy);

    //! TSDecl: function RotateRad(rad: number, pt: CkPoint): CkMatrix
    static v8::Local<v8::Value> RotateRad(SkScalar rad, v8::Local<v8::Value> pt);

    //! TSDecl: function Skew(kx: number, ky: number): CkMatrix
    static v8::Local<v8::Value> Skew(SkScalar kx, SkScalar ky);

    //! TSDecl: function RectToRect(src: CkRect, dst: CkRect, mode: Enum<MatrixScaleToFit>): CkMatrix
    static v8::Local<v8::Value> RectToRect(v8::Local<v8::Value> src,
                                           v8::Local<v8::Value> dst, int32_t mode);

    //! TSDecl: function All(scaleX: number, skewX: number, transX: number,
    //!                      skewY: number, scaleY: number, transY: number,
    //!                      pers0: number, pers1: number, pers2: number): CkMatrix
    static v8::Local<v8::Value> All(SkScalar scaleX, SkScalar skewX,  SkScalar transX,
                                    SkScalar skewY,  SkScalar scaleY, SkScalar transY,
                                    SkScalar pers0, SkScalar pers1, SkScalar pers2);

    //! TSDecl: function Concat(a: CkMatrix, b: CkMatrix): CkMatrix
    static v8::Local<v8::Value> Concat(v8::Local<v8::Value> a, v8::Local<v8::Value> b);

    //! TSDecl: scaleX: number
    SkScalar getScaleX();
    void setScaleX(SkScalar v);

    //! TSDecl: scaleY: number
    SkScalar getScaleY();
    void setScaleY(SkScalar v);

    //! TSDecl: skewX: number
    SkScalar getSkewX();
    void setSkewX(SkScalar v);

    //! TSDecl: skewY: number
    SkScalar getSkewY();
    void setSkewY(SkScalar v);

    //! TSDecl: translateX: number
    SkScalar getTranslateX();
    void setTranslateX(SkScalar v);

    //! TSDecl: translateY: number
    SkScalar getTranslateY();
    void setTranslateY(SkScalar v);

    //! TSDecl: perspectiveX: number
    SkScalar getPerspectiveX();
    void setPerspectiveX(SkScalar v);

    //! TSDecl: perspectiveY: number
    SkScalar getPerspectiveY();
    void setPerspectiveY(SkScalar v);

    //! TSDecl: function clone(): CkMatrix
    v8::Local<v8::Value> clone();

    //! TSDecl: function rectStaysRect(): boolean
    bool rectStaysRect();

    //! TSDecl: function hasPerspective(): boolean
    bool hasPerspective();

    //! TSDecl: function isSimilarity(): boolean
    bool isSimilarity();

    //! TSDecl: function preservesRightAngles(): boolean
    bool preservesRightAngles();

    //! TSDecl: function preTranslate(dx: number, dy: number): void
    void preTranslate(SkScalar dx, SkScalar dy);

    //! TSDecl: function preScale(sx: number, sy: number, px: number, py: number): void
    void preScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py);

    //! TSDecl: function preRotate(rad: number, px: number, py: number): void
    void preRotate(SkScalar rad, SkScalar px, SkScalar py);

    //! TSDecl: function preSkew(kx: number, ky: number, px: number, py: number): void
    void preSkew(SkScalar kx, SkScalar ky, SkScalar px, SkScalar py);

    //! TSDecl: function preConcat(other: CkMatrix): void
    void preConcat(v8::Local<v8::Value> other);

    //! TSDecl: function postTranslate(dx: number, dy: number): void
    void postTranslate(SkScalar dx, SkScalar dy);

    //! TSDecl: function postScale(sx: number, sy: number, px: number, py: number): void
    void postScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py);

    //! TSDecl: function postRotate(rad: number, px: number, py: number): void
    void postRotate(SkScalar rad, SkScalar px, SkScalar py);

    //! TSDecl: function postSkew(kx: number, ky: number, px: number, py: number): void
    void postSkew(SkScalar kx, SkScalar ky, SkScalar px, SkScalar py);

    //! TSDecl: function postConcat(other: CkMatrix): void
    void postConcat(v8::Local<v8::Value> other);

    //! TSDecl: function invert(): null | CkMatrix
    v8::Local<v8::Value> invert();

    //! TSDecl: function normalizePerspective(): CkMatrix
    void normalizePerspective();

    //! TSDecl: function mapPoints(points: Array<CkPoint>): Array<CkPoint>
    v8::Local<v8::Value> mapPoints(v8::Local<v8::Value> points);

    //! TSDecl: function mapPoint(point: CkPoint): CkPoint
    v8::Local<v8::Value> mapPoint(v8::Local<v8::Value> point);

    //! TSDecl: function mapHomogeneousPoints(points: Array<CkPoint3>): Array<CkPoint3>
    v8::Local<v8::Value> mapHomogeneousPoints(v8::Local<v8::Value> points);

    //! TSDecl: function mapRect(src: CkRect, pc: Enum<ApplyPerspectiveClip>): CkRect
    v8::Local<v8::Value> mapRect(v8::Local<v8::Value> src, int32_t pc);

    //! TSDecl: function mapRadius(radius: number): number
    SkScalar mapRadius(SkScalar radius);

    //! TSDecl: function isFinite(): boolean
    bool isFinite();

private:
    SkMatrix matrix_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_CKMATRIXWRAP_H
