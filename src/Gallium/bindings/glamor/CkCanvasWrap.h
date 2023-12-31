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

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_CKCANVASWRAP_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_CKCANVASWRAP_H

#include "include/core/SkCanvas.h"
#include "include/v8.h"

#include "Core/Errors.h"
#include "Gallium/bindings/ExportableObjectBase.h"
#include "Gallium/bindings/glamor/TrivialInterface.h"
#include "Gallium/binder/ThrowExcept.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

class CkCanvas : public ExportableObjectBase
{
public:
    explicit CkCanvas(SkCanvas *canvas) : canvas_(canvas) {
        CHECK(canvas != nullptr);
    }
    ~CkCanvas() = default;

    class NullSafeCanvasPtr
    {
    public:
        explicit NullSafeCanvasPtr(SkCanvas *canvas) : ptr_(canvas) {}
        ~NullSafeCanvasPtr() = default;

        g_inline void SetNull() {
            ptr_ = nullptr;
        }

        g_nodiscard g_inline SkCanvas *Get() const {
            return ptr_;
        }

        g_nodiscard g_inline SkCanvas *operator->() const {
            if (!ptr_) [[unlikely]]
                g_throw(Error, "Canvas has been disposed");
            return ptr_;
        }

    private:
        SkCanvas *ptr_;
    };

    g_nodiscard g_inline SkCanvas *GetCanvas() const {
        return canvas_.Get();
    }

    //! TSDecl: function save(): number
    int save();

    //! TSDecl: function saveLayer(bounds: null | CkRect, paint: null | CkPaint): number
    int saveLayer(v8::Local<v8::Value> bounds, v8::Local<v8::Value> paint);

    //! TSDecl: function saveLayerAlpha(bounds: null | CkRect, alpha: number): number
    int saveLayerAlpha(v8::Local<v8::Value> bounds, float alpha);

    //! TSDecl:
    //! interface SaveLayerRec {
    //!   bounds: null | CkRect;
    //!   paint: null | CkPaint;
    //!   backdrop: null | CkImageFilter;
    //!   flags: Bitfield<Enum<CanvasSaveLayer>>;
    //! }

    //! TSDecl: function saveLayerRec(rec: SaveLayerRec): number
    int saveLayerRec(v8::Local<v8::Value> rec);

    //! TSDecl: function restore(): void
    void restore();

    //! TSDecl: function restoreToCount(saveCount: number): void
    void restoreToCount(int saveCount);

    //! TSDecl: function getSaveCount(): number
    int getSaveCount();

    //! TSDecl: function translate(dx: number, dy: number): void
    void translate(SkScalar dx, SkScalar dy);

    //! TSDecl: function scale(sx: number, sy: number): void
    void scale(SkScalar sx, SkScalar sy);

    //! TSDecl: function rotate(rad: number, px: number, py: number): void
    void rotate(SkScalar rad, SkScalar px, SkScalar py);

    //! TSDecl: function skew(sx: number, sy: number)
    void skew(SkScalar sx, SkScalar sy);

    //! TSDecl: function concat(matrix: CkMat3x3): void
    void concat(v8::Local<v8::Value> matrix);

    //! TSDecl: function setMatrix(matrix: CkMat3x3): void
    void setMatrix(v8::Local<v8::Value> matrix);

    //! TSDecl: function resetMatrix(): void
    void resetMatrix();

    //! TSDecl: function getTotalMatrix(): CkMatrix
    v8::Local<v8::Value> getTotalMatrix();

    //! TSDecl: function clipRect(rect: CkRect, op: Enum<ClipOp>, AA: boolean): void
    void clipRect(v8::Local<v8::Value> rect, int32_t op, bool AA);

    //! TSDecl: function clipRRect(rrect: CkRRect, op: Enum<ClipOp>, AA: boolean): void
    void clipRRect(v8::Local<v8::Value> rrect, int32_t op, bool AA);

    //! TSDecl: function clipPath(path: CkPath, op: Enum<ClipOp>, AA: boolean): void
    void clipPath(v8::Local<v8::Value> path, int32_t op, bool AA);

    //! TSDecl: function clipShader(shader: CkShader, op: Enum<ClipOp>): void
    void clipShader(v8::Local<v8::Value> shader, int32_t op);

    //! TSDecl: function quickRejectRect(rect: CkRect): boolean
    bool quickRejectRect(v8::Local<v8::Value> rect);

    //! TSDecl: function quickRejectPath(path: CkPath): boolean
    bool quickRejectPath(v8::Local<v8::Value> path);

    //! TSDecl: function getLocalClipBounds(): CkRect
    v8::Local<v8::Value> getLocalClipBounds();

    //! TSDecl: function getDeviceClipBounds(): CkRect
    v8::Local<v8::Value> getDeviceClipBounds();

    //! TSDecl: function drawColor(color: Color4f, mode: Enum<BlendMode>): void
    void drawColor(v8::Local<v8::Value> color, int32_t mode);

    //! TSDecl: function clear(color: Color4f): void
    void clear(v8::Local<v8::Value> color);

    //! TSDecl: function drawPaint(paint: CkPaint): void
    void drawPaint(v8::Local<v8::Value> paint);

    //! TSDecl: function drawPoints(mode: Enum<PointMode>, points: Array<CkPoint>, paint: CkPaint): void
    void drawPoints(int32_t mode, v8::Local<v8::Value> points, v8::Local<v8::Value> paint);

    //! TSDecl: function drawPoint(x: number, y: number, paint: CkPaint): void
    void drawPoint(SkScalar x, SkScalar y, v8::Local<v8::Value> paint);

    //! TSDecl: function drawLine(p1: CkPoint, p2: CkPoint, paint: CkPaint): void
    void drawLine(v8::Local<v8::Value> p1, v8::Local<v8::Value> p2, v8::Local<v8::Value> paint);

    //! TSDecl: function drawRect(rect: CkRect, paint: CkPaint): void
    void drawRect(v8::Local<v8::Value> rect, v8::Local<v8::Value> paint);

    //! TSDecl: function drawOval(oval: CkRect, paint: CkPaint): void
    void drawOval(v8::Local<v8::Value> rect, v8::Local<v8::Value> paint);

    //! TSDecl: function drawRRect(rrect: CkRRect, paint: CkPaint): void
    void drawRRect(v8::Local<v8::Value> rrect, v8::Local<v8::Value> paint);

    //! TSDecl: function drawDRRect(outer: CkRRect, inner: CkRRect, paint: CkPaint): void
    void drawDRRect(v8::Local<v8::Value> outer, v8::Local<v8::Value> inner, v8::Local<v8::Value> paint);

    //! TSDecl: function drawCircle(cx: number, cy: number, r: number, paint: CkPaint): void
    void drawCircle(SkScalar cx, SkScalar cy, SkScalar r, v8::Local<v8::Value> paint);

    //! TSDecl: function drawArc(oval: CkRect, startAngle: number, sweepAngle: number,
    //!                          useCenter: boolean, paint: CkPaint): void
    void drawArc(v8::Local<v8::Value> oval, SkScalar startAngle, SkScalar sweepAngle,
                 bool useCenter, v8::Local<v8::Value> paint);

    //! TSDecl: function drawRoundRect(rect: CkRect, rx: number, ry: number, paint: CkPaint): void
    void drawRoundRect(v8::Local<v8::Value> rect, SkScalar rx, SkScalar ry, v8::Local<v8::Value> paint);

    //! TSDecl: function drawPath(path: CkPath, paint: CkPaint): void
    void drawPath(v8::Local<v8::Value> path, v8::Local<v8::Value> paint);

    //! TSDecl: function drawImage(image: CkImage, left: number, top: number,
    //!                            sampling: Enum<Sampling>, paint: null | CkPaint): void
    void drawImage(v8::Local<v8::Value> image, SkScalar left, SkScalar top,
                   int32_t sampling, v8::Local<v8::Value> paint);

    //! TSDecl: function drawImageRect(image: CkImage, src: CkRect, dst: CkRect,
    //!                                sampling: Enum<Sampling>, paint: null | CkPaint,
    //!                                constraint: Enum<CanvasSrcRectConstraint>): void
    void drawImageRect(v8::Local<v8::Value> image, v8::Local<v8::Value> src,
                       v8::Local<v8::Value> dst, int32_t sampling,
                       v8::Local<v8::Value> paint, int32_t constraint);

    //! TSDecl: function drawString(str: string, x: number, y: number, font: CkFont,
    //!                             paint: CkPaint): void
    void drawString(const std::string& str, SkScalar x, SkScalar y,
                    v8::Local<v8::Value> font, v8::Local<v8::Value> paint);

    //! TSDecl: function drawGlyphs(glyphs: Uint16Array, positions: Array<CkPoint>,
    //!                             origin: CkPoint, font: CkFont, paint: CkPaint): void
    void drawGlyphs(v8::Local<v8::Value> glyphs, v8::Local<v8::Value> positions,
                    v8::Local<v8::Value> origin, v8::Local<v8::Value> font,
                    v8::Local<v8::Value> paint);

    //! TSDecl: function drawTextBlob(blob: CkTextBlob, x: number, y: number, paint: CkPaint): void
    void drawTextBlob(v8::Local<v8::Value> blob, SkScalar x, SkScalar y, v8::Local<v8::Value> paint);

    //! TSDecl: function drawPicture(picture: CkPicture, matrix: null | CkMat3x3,
    //!                              paint: null | CkPaint): void
    void drawPicture(v8::Local<v8::Value> picture, v8::Local<v8::Value> matrix,
                     v8::Local<v8::Value> paint);

    //! TSDecl: function drawVertices(vertices: CkVertices, mode: Enum<BlendMode>, paint: CkPaint): void
    void drawVertices(v8::Local<v8::Value> vertices, int32_t mode, v8::Local<v8::Value> paint);

    //! TSDecl: function drawPatch(cubics: Array<CkPoint>, colors: Array<Color4f>,
    //!                            texCoords: Array<CkPoint>, mode: Enum<BlendMode>, paint: CkPaint): void
    void drawPatch(v8::Local<v8::Value> cubics, v8::Local<v8::Value> colors,
                   v8::Local<v8::Value> texCoords, int32_t mode, v8::Local<v8::Value> paint);

    // TODO(sora): draw annotation, draw atlas

protected:
    void InvalidateCanvasRef();

private:
    NullSafeCanvasPtr canvas_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_CKCANVASWRAP_H
