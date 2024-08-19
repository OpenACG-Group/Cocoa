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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_CANVAS_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_CANVAS_H

#include "include/core/SkCanvas.h"

#include "Gallium/ffi/JSObject.h"
#include "Gallium/bindings/EventEmitter.h"
#include "Gallium/bindings/renderer/Types.h"
#include "Gallium/bindings/renderer/Rect.h"
#include "Gallium/bindings/renderer/Paint.h"
#include "Gallium/bindings/renderer/Image.h"
#include "Gallium/bindings/renderer/Matrix.h"
#include "Gallium/bindings/renderer/Path.h"
#include "Gallium/bindings/renderer/ImageFilter.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

class Picture;
class Font;
class Vertices;

//! TSDecl: @interface SaveLayerRec
struct SaveLayerRec
{
    //! TSDecl: @property @optional bounds: Rect
    ffi::OptLocal<v8::Value> bounds;

    //! TSDecl: @property @optional paint: Paint
    ffi::Opt<ffi::Class<Paint>> paint;

    //! TSDecl: @property @optional flags: SaveLayerFlags
    ffi::Opt<uint32_t> flags;

    //! TSDecl: @property @optional backdrop: ImageFilter
    ffi::Opt<ffi::Class<ImageFilter>> backdrop;

    //! TSDecl: @property @optional filters: @array(ImageFilter)
    ffi::OptLocal<v8::Array> filters;
};
//! TSDecl: @end

class SaveLayerRecAdapter : public ffi::ArgAdapter
{
public:
    static ffi::Ret<SaveLayerRecAdapter> Cast(v8::Isolate *isolate, v8::Local<v8::Value> value);

    const SkCanvas::SaveLayerRec& operator*() const {
        return rec;
    }

    SkRect bounds;
    sk_sp<SkImageFilter> backdrop;
    std::vector<sk_sp<SkImageFilter>> filters;
    SkCanvas::SaveLayerRec rec;
};

//! @tsdocbegin
//! Canvas provides an interface for drawing, and how the drawing is clipped and transformed.
//! Canvas contains a stack of Mat3x3/Mat4x4 and clip values.
//!
//! `Canvas` and `Paint` together provide the state to draw into `Surface` or other targets.
//! Each Canvas draw call transforms the geometry of the object by the concatenation of all
//! matrix values in the stack. The transformed geometry is clipped by the intersection
//! of all of clip values in the stack. The Canvas draw calls use Paint to supply drawing
//! state such as color, Typeface, text size, stroke width, Shader and so on.
//!
//! `Canvas` should be created by its parent, like `Surface`, `PictureRecorder`, etc.
//! Draw calls are sent to the parent that creates Canvas, and the parent also manages the
//! lifetime of `Canvas`. Once the parent is disposed, the Canvas will be disposed together
//! immediately.
//!
//! `Canvas` is an event emitter of the following events:
//! @event release(): when the parent of Canvas is disposed.
//! @tsdocend
//! TSDecl: @class @nonconstructible @extends(@import(event) EventEmitterBase) Canvas
class Canvas : public EventEmitterBase
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    /**
     * Since `Canvas` object just uses a bare pointer to access the underlying
     * canvas, the lifecycle of underlying canvas is beyond our control.
     * So a `parent` object referring to the actual owner of underlying canvas
     * should be provided to keep the underlying canvas alive during the lifetime
     * of `Canvas` object.
     */
    Canvas(SkCanvas *canvas, v8::Local<v8::Object> parent);
    ~Canvas() override = default;

    SkCanvas *GetSkCanvas() const {
        return canvas_;
    }

    void OnParentDispose();

    //! @tsdocbegin
    //! Parent, an object that creates the Canvas and manages its lifetime.
    //! @tsdocend
    //! TSDecl: @property @readonly parent: object
    ffi::RetLocal<v8::Value> getParent() {
        CHECK(!parent_.IsEmpty());
        return parent_.Get(v8::Isolate::GetCurrent());
    }

    //! @tsdocbegin
    //! Saves matrix and clip.
    //! Calling restore() discards changes to matrix and clip,
    //! restoring the matrix and clip to their state when save() was called.
    //!
    //! Matrix may be changed by `translate()`, `scale()`, `rotate()`, `skew()`, `concat()`,
    //! `setMatrix()`, and `resetMatrix()`. Clip may be changed by `clipRect()`, `clipRRect()`,
    //! `clipPath()`.
    //!
    //! Saved Canvas state is put on a stack; multiple calls to `save()` should be balanced
    //! by an equal number of calls to `restore()`.
    //!
    //! Call `restoreToCount()` with the result to restore this and subsequent saves.
    //!
    //! @return  depth of saved stack
    //! @tsdocend
    //! TSDecl: @method save(): i32
    ffi::Ret<int32_t> save();

    //! @tsdocbegin
    //! Saves matrix and clip, and allocates a Surface for subsequent drawing.
    //! Calling `restore()` discards changes to matrix and clip, and draws the Surface.
    //!
    //! Rect bounds suggests but does not define the Surface size. To clip drawing to
    //! a specific rectangle, use `clipRect()`.
    //!
    //! Optional paint applies alpha, ColorFilter, ImageFilter, and
    //! BlendMode when `restore()` is called.
    //!
    //! Call `restoreToCount()` with returned value to restore this and subsequent saves.
    //!
    //! @param bounds  hint to limit the size of the layer; may be `null`
    //! @param paint   graphics state for layer; may be `null`
    //! @return        depth of saved stack
    //! @tsdocend
    //! TSDecl: @method saveLayer(bounds: @union(Rect, null), paint: @union(null, Paint)): i32
    ffi::Ret<int32_t> saveLayer(ffi::Opt<RectAdapter> bounds, ffi::Opt<ffi::Class<Paint>> paint);

    //! @tsdocbegin
    //! Variant of `saveLayer`, accepts `alpha` instead of `Paint`.
    //! @tsdocend
    //! TSDecl: @method saveLayerAlphaf(bounds: @union(Rect, null), alpha: f32): i32
    ffi::Ret<int32_t> saveLayerAlphaf(ffi::Opt<RectAdapter> bounds, float alpha);

    //! @tsdocbegin
    //! Variant of `saveLayer`, accepts `SaveLayerRec` that contains the state used to create
    //! the layer.
    //! @tsdocend
    //! TSDecl: @method saveLayerRec(rec: SaveLayerRec): i32
    ffi::Ret<int32_t> saveLayerRec(const SaveLayerRecAdapter& rec);

    //! @tsdocbegin
    //! Removes changes to matrix and clip since Canvas state was last saved.
    //! The state is removed from the stack.
    //!
    //! Does nothing if the stack is empty.
    //! @tsdocend
    //! TSDecl: @method restore(): void
    ffi::Ret<void> restore();

    //! @tsdocbegin
    //! Returns the number of saved states, each containing: matrix and clip.
    //! Equals the number of save() calls less the number of `restore()` calls plus one.
    //! The save count of a new canvas is one.
    //!
    //! @return  depth of save state stack
    //! @tsdocend
    //! TSDecl: @method getSaveCount(): i32
    ffi::Ret<int32_t> getSaveCount();

    //! @tsdocbegin
    //! Restores state to matrix and clip values when `save()`, `saveLayer()`,
    //! `saveLayerAlphaf()`, or `saveLayerRec()` returned `saveCount`.
    //!
    //! Does nothing if saveCount is greater than state stack count.
    //! Restores state to initial values if saveCount is less than or equal to one.
    //!
    //! @param saveCount  depth of state stack to restore
    //! @tsdocend
    //! TSDecl: @method restoreToCount(saveCount: i32): void
    ffi::Ret<void> restoreToCount(int32_t save_count);

    //! @tsdocbegin
    //! Translates matrix by dx along the x-axis and dy along the y-axis.
    //!
    //! Mathematically, replaces local CTM with a translation matrix
    //! premultiplied with the current local CTM.
    //!
    //! This has the effect of moving the drawing by (dx, dy) before transforming
    //! the result with the previous local CTM.
    //!
    //! @param dx  distance to translate on x-axis
    //! @param dy  distance to translate on y-axis
    //! @tsdocend
    //! TSDecl: @method translate(dx: f32, dy: f32): void
    ffi::Ret<void> translate(float dx, float dy);

    //! @tsdocbegin
    //! Scales matrix by sx on the x-axis and sy on the y-axis.
    //!
    //! Mathematically, replaces local CTM with a scale matrix
    //! premultiplied with the current local CTM.
    //!
    //! This has the effect of scaling the drawing by (sx, sy) before transforming
    //! the result with the original local CTM.
    //!
    //! @param sx  amount to scale on x-axis
    //! @param sy  amount to scale on y-axis
    //! @tsdocend
    //! TSDecl: @method scale(sx: f32, sy: f32): void
    ffi::Ret<void> scale(float sx, float sy);

    //! @tsdocbegin
    //! Rotates matrix by degrees. Positive degrees rotates clockwise.
    //!
    //! Mathematically, replaces local CTM with a rotation matrix
    //! premultiplied with the current local CTM.
    //!
    //! This has the effect of rotating the drawing by degrees before transforming
    //! the result with the original local CTM.
    //!
    //! @param degrees  amount to rotate, in degrees
    //! @tsdocend
    //! TSDecl: @method rotate(degrees: f32): void
    ffi::Ret<void> rotate(float degrees);

    //! @tsdocbegin
    //! Variant of `rotate()`, accepts a point (px, py) to rotate about.
    //! @tsdocend
    //! TSDecl: @method rotatePivot(degrees: f32, px: f32, py: f32): void
    ffi::Ret<void> rotatePivot(float degrees, float px, float py);

    //! @tsdocbegin
    //! Skews matrix by sx on the x-axis and sy on the y-axis. A positive value of sx
    //! skews the drawing right as y-axis values increase; a positive value of sy skews
    //! the drawing down as x-axis values increase.
    //!
    //! Mathematically, replaces local CTM with a skew matrix premultiplied with the
    //! current local CTM.
    //!
    //! This has the effect of skewing the drawing by (sx, sy) before transforming
    //! the result with the original local CTM.
    //!
    //! @param sx  amount to skew on x-axis
    //! @param sy  amount to skew on y-axis
    //! @tsdocend
    //! TSDecl: @method skew(sx: f32, sy: f32): void
    ffi::Ret<void> skew(float sx, float sy);

    //! @tsdocbegin
    //! Replaces matrix with matrix premultiplied with the existing one.
    //! The internal matrix storage is always `Mat4x4`, `concat33()` will expand
    //! the 3x3 matrix to 4x4 before multiplying.
    //!
    //! @param mat  matrix to premultiply with existing SkMatrix
    //! @tsdocend
    //! TSDecl: @method concat33(mat: Mat3x3): void
    ffi::Ret<void> concat33(const Mat3x3Adapter& mat);

    //! @tsdocbegin
    //! A variant of `concat33()`, accepts a `Mat4x4` matrix.
    //! @tsdocend
    //! TSDecl: @method concat44(mat: Mat4x4): void
    ffi::Ret<void> concat44(const Mat4x4Adapter& mat);

    //! TSDecl: @method setMatrix(mat: Mat4x4): void
    ffi::Ret<void> setMatrix(const Mat4x4Adapter& mat);

    //! @tsdocbegin
    //! Sets matrix to the identity matrix.
    //! Any prior matrix state is overwritten.
    //! @tsdocend
    //! TSDecl: @method resetMatrix(): void
    ffi::Ret<void> resetMatrix();

    //! @tsdocbegin
    //! Replaces clip with the intersection or difference of clip and rect,
    //! with an aliased or anti-aliased clip edge. rect is transformed by SkMatrix
    //! before it is combined with clip.
    //!
    //! @param rect         `Rect` to combine with clip
    //! @param op           `ClipOp` to apply to clip
    //! @param doAntiAlias  true if clip is to be anti-aliased
    //! @tsdocend
    //! TSDecl: @method clipRect(rect: Rect, op: ClipOp, doAntiAlias: boolean): void
    ffi::Ret<void> clipRect(RectAdapter rect, ffi::Enum<SkClipOp> op, bool do_anti_alias);

    //! @tsdocbegin
    //! Replaces clip with the intersection or difference of clip and rrect,
    //! with an aliased or anti-aliased clip edge.
    //! rrect is transformed by matrix before it is combined with clip.
    //!
    //! @param rrect        `RRect` to combine with clip
    //! @param op           `ClipOp` to apply to clip
    //! @param doAntiAlias  true if clip is to be anti-aliased
    //! @tsdocend
    //! TSDecl: @method clipRRect(rrect: RRect, op: ClipOp, doAntiAlias: boolean): void
    ffi::Ret<void> clipRRect(RRectAdapter rrect, ffi::Enum<SkClipOp> op, bool do_anti_alias);

    //! @tsdocbegin
    //! Replaces clip with the intersection or difference of clip and path,
    //! with an aliased or anti-aliased clip edge. `Path.fillType` determines if path
    //! describes the area inside or outside its contours; and if path contour overlaps
    //! itself or another path contour, whether the overlaps form part of the area.
    //! path is transformed by matrix before it is combined with clip.
    //!
    //! @param path         `Path` to combine with clip
    //! @param op           `ClipOp` to apply to clip
    //! @param doAntiAlias  true if clip is to be anti-aliased
    //! @tsdocend
    //! TSDecl: @method clipPath(path: Path, op: ClipOp, doAntiAlias: boolean): void
    ffi::Ret<void> clipPath(ffi::Class<Path> path, ffi::Enum<SkClipOp> op, bool do_anti_alias);

    //! @tsdocbegin
    //! Replaces clip with the intersection or difference of clip and shader.
    //! The result of shader will be converted into grayscale first,
    //! and multiplies the pixel value with the grayscale to generate the final pixel.
    //! In some situations, this may also be called "Shader Mask".
    //!
    //! @param shader       `Shader` to combine with clip.
    //! @param op           `ClipOp` to apply to clip.
    //! @tsdocend
    //! TSDecl: @method clipShader(shader: Shader, op: ClipOp): void
    ffi::Ret<void> clipShader(ffi::Class<Shader> shader, ffi::Enum<SkClipOp> op);

    // TODO(sora): clipRegion?

    //! @tsdocbegin
    //! Returns true if `rect`, transformed by matrix, can be quickly determined to be
    //! outside of clip. May return false even though `rect` is outside of clip.
    //!
    //! Use to check if an area to be drawn is clipped out, to skip subsequent draw calls.
    //!
    //! @param rect  `Rect` area to test
    //! @return      true if rect, transformed by matrix, does not intersect clip
    //! @tsdocend
    //! TSDecl: @method quickRejectRect(rect: Rect): boolean
    ffi::Ret<bool> quickRejectRect(RectAdapter rect);

    //! @tsdocbegin
    //! A variant of `quickRejectRect()`, accepts a `Path` instead of `Rect`.
    //! @tsdocend
    //! TSDecl: @method quickRejectPath(path: Path): boolean
    ffi::Ret<bool> quickRejectPath(ffi::Class<Path> path);

    //! @tsdocbegin
    //! Bounds of clip in local coordinates, transformed by inverse of matrix.
    //! If clip is empty, return `Rect.MakeEmpty()`, where all `Rect` sides equal zero.
    //!
    //! `Rect` returned is outset by one to account for partial pixel coverage if clip
    //! is anti-aliased.
    //! @tsdocend
    //! TSDecl: @property @readonly localClipBounds: Rect
    ffi::RetLocal<v8::Value> getLocalClipBounds();

    //! @tsdocbegin
    //! bounds of clip in base device coordinates, unaffected by matrix.
    //! If clip is empty, return `Rect.MakeEmpty()`, where all `Rect` sides equal zero.
    //!
    //! Unlike `Canvas.localClipBounds`, returned `Rect` is not outset.
    //! @tsdocend
    //! TSDecl: @property @readonly deviceClipBounds: Rect
    ffi::RetLocal<v8::Value> getDeviceClipBounds();

    //! @tsdocbegin
    //! Fills clip with color color.
    //! `mode` determines how ARGB is combined with destination.
    //!
    //! @param color  unpremultiplied ARGB
    //! @param mode   `BlendMode` used to combine source color and destination
    //! @tsdocend
    //! TSDecl: @method drawColor(color: u32, mode: BlendMode): void
    ffi::Ret<void> drawColor(uint32_t color, ffi::Enum<SkBlendMode> mode);

    //! @tsdocbegin
    //! A variant of `drawColor()`, accepts a Color4F quadruple.
    //! @tsdocend
    //! TSDecl: @method drawColor4f(color: Color4f, mode: BlendMode): void
    ffi::Ret<void> drawColor4f(Color4fQuadruple color, ffi::Enum<SkBlendMode> mode);

    //! @tsdocbegin
    //! A variant of `drawColor4f()`, with `mode == BlendMode.Src`.
    //! Useful for erasing the whole clip with a particular color.
    //! @tsdocend
    //! TSDecl: @method clear(color: Color4f): void
    ffi::Ret<void> clear(Color4fQuadruple color);

    //! @tsdocbegin
    //! Makes Canvas contents undefined. Subsequent calls that read Canvas pixels,
    //! such as drawing with `BlendMode`, return undefined results. `discard()` does
    //! not change clip or matrix.
    //!
    //! `discard()` may do nothing, depending on the implementation that created Canvas.
    //!
    //! `discard()` allows optimized performance on subsequent draws by removing
    //! cached data associated with the underlying implementation.
    //! It is not necessary to call `discard()` once done with Canvas;
    //! any cached data is deleted when owning parent of Canvas is deleted.
    //! @tsdocend
    //! TSDecl: @method discard(): void
    ffi::Ret<void> discard();

    //! TSDecl: @method drawPaint(paint: Paint): void
    ffi::Ret<void> drawPaint(ffi::Class<Paint> paint);

    //! TSDecl: @method drawPoints(mode: PointMode, pts: @mem(f32), paint: Paint): void
    ffi::Ret<void> drawPoints(ffi::Enum<SkCanvas::PointMode> mode,
                              const ffi::Mem<float>& pts_memory,
                              ffi::Class<Paint> paint);

    //! TSDecl: @method drawPoint(x: f32, y: f32, paint: Paint): void
    ffi::Ret<void> drawPoint(float x, float y, ffi::Class<Paint> paint);

    //! TSDecl: @method drawLine(x0: f32, y0: f32, x1: f32, y1: f32, paint: Paint): void
    ffi::Ret<void> drawLine(float x0, float y0, float x1, float y1, ffi::Class<Paint> paint);

    //! TSDecl: @method drawRect(rect: Rect, paint: Paint): void
    ffi::Ret<void> drawRect(RectAdapter rect, ffi::Class<Paint> paint);

    // TODO(sora): drawRegion?

    //! TSDecl: @method drawOval(oval: Rect, paint: Paint): void
    ffi::Ret<void> drawOval(RectAdapter oval, ffi::Class<Paint> paint);

    //! TSDecl: @method drawRRect(rrect: RRect, paint: Paint): void
    ffi::Ret<void> drawRRect(RRectAdapter rrect, ffi::Class<Paint> paint);

    //! TSDecl: @method drawDRRect(outer: RRect, inner: RRect, paint: Paint): void
    ffi::Ret<void> drawDRRect(RRectAdapter outer, RRectAdapter inner, ffi::Class<Paint> paint);

    //! TSDecl: @method drawCircle(cx: f32, cy: f32, radius: f32, paint: Paint): void
    ffi::Ret<void> drawCircle(float cx, float cy, float radius, ffi::Class<Paint> paint);

    //! TSDecl: @method drawArc(oval: Rect, startAngleDeg: f32, sweepAngleDeg: f32,
    //! TSDecl:                 useCenter: boolean, paint: Paint): void
    ffi::Ret<void> drawArc(RectAdapter oval, float start_angle_deg, float sweep_angle_deg,
                           bool use_center, ffi::Class<Paint> paint);

    //! TSDecl: @method drawRoundRect(rect: Rect, rx: f32, ry: f32, paint: Paint): void
    ffi::Ret<void> drawRoundRect(RectAdapter rect, float rx, float ry, ffi::Class<Paint> paint);

    //! TSDecl: @method drawPath(path: Path, paint: Paint): void
    ffi::Ret<void> drawPath(ffi::Class<Path> path, ffi::Class<Paint> paint);

    //! TSDecl: @method drawImage(image: Image, left: f32, top: f32, sampling: SamplingOptions,
    //! TSDecl:                   paint: @union(Paint, null)): void
    ffi::Ret<void> drawImage(ffi::Class<Image> image, float left, float top,
                             SamplingOptionsAdapter sampling, ffi::Opt<ffi::Class<Paint>> paint);

    //! TSDecl: @method drawImageRect(image: Image, dst: Rect, sampling: SamplingOptions,
    //! TSDecl:                       paint: @union(Paint, null)): void
    ffi::Ret<void> drawImageRect(ffi::Class<Image> image, RectAdapter dst, SamplingOptionsAdapter sampling,
                                 ffi::Opt<ffi::Class<Paint>> paint);

    //! TSDecl: @method drawImageRectToRect(image: Image, src: Rect, dst: Rect, sampling: SamplingOptions,
    //! TSDecl:                             paint: @union(Paint, null), strictConstraint: boolean): void
    ffi::Ret<void> drawImageRectToRect(ffi::Class<Image> image, RectAdapter src, RectAdapter dst,
                                       SamplingOptionsAdapter sampling, ffi::Opt<ffi::Class<Paint>> paint,
                                       bool strict_constraint);

    //! TSDecl: @method drawPicture(picture: Picture, matrix: @union(Mat3x3, null),
    //! TSDecl:                     paint: @union(Paint, null)): void
    ffi::Ret<void> drawPicture(const ffi::Class<Picture>& picture,
                               const ffi::Opt<Mat3x3Adapter>& matrix,
                               const ffi::Opt<ffi::Class<Paint>>& paint);

    //! TSDecl: @method drawString(text: string, x: f32, y: f32, font: Font, paint: Paint): void
    ffi::Ret<void> drawString(v8::Local<v8::String> text, float x, float y, const ffi::Class<Font>& font,
                              const ffi::Class<Paint>& paint);

    //! TSDecl: @method drawGlyphs(glyphs: @mem(u16), xforms: RSXformArray, originX: f32, originY: f32,
    //! TSDecl:                    font: Font, paint: Paint): void
    ffi::Ret<void> drawGlyphs(const ffi::Mem<uint16_t>& glyphs, const RSXformArrayAdapter& xforms,
                              float origin_x, float origin_y,
                              const ffi::Class<Font>& font, const ffi::Class<Paint>& paint);

    //! TSDecl: @method drawVertices(vertices: Vertices, mode: BlendMode, paint: Paint): void
    ffi::Ret<void> drawVertices(ffi::Class<Vertices> vertices, ffi::Enum<SkBlendMode> mode,
                                ffi::Class<Paint> paint);

    // TODO(sora): complete this...

private:
    SkCanvas                *canvas_;
    v8::Global<v8::Object>   parent_;
    EventEmitFuncT           rel_event_emit_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_CANVAS_H
