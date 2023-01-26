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

#include <memory>

#include "fmt/format.h"

#include "Gallium/bindings/glamor/CkCanvasWrap.h"
#include "Gallium/bindings/glamor/TrivialInterface.h"
#include "Gallium/bindings/glamor/CkPaintWrap.h"
#include "Gallium/bindings/glamor/CkPathWrap.h"
#include "Gallium/bindings/glamor/CkMatrixWrap.h"
#include "Gallium/bindings/glamor/CkFontWrap.h"
#include "Gallium/bindings/glamor/CkTextBlobWrap.h"
#include "Gallium/bindings/glamor/CkVerticesWrap.h"
#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

namespace {

#define EXTRACT_MAT_CHECKED(arg, result) \
    auto *result = binder::Class<CkMatrix>::unwrap_object(isolate, arg); \
    if (!result) {                       \
        g_throw(TypeError, "Argument `" #arg "` must be an instance of `CkMatrix`"); \
    }

#define EXTRACT_PATH_CHECKED(arg, result) \
    auto *result = binder::Class<CkPath>::unwrap_object(isolate, arg); \
    if (!result) {                       \
        g_throw(TypeError, "Argument `" #arg "` must be an instance of `CkPath`"); \
    }

#define EXTRACT_PAINT_CHECKED(arg, result) \
    auto *result = binder::Class<CkPaint>::unwrap_object(isolate, arg); \
    if (!result) {                       \
        g_throw(TypeError, "Argument `" #arg "` must be an instance of `CkPaint`"); \
    }

#define EXTRACT_IMAGE_CHECKED(arg, result) \
    auto *result = binder::Class<CkImageWrap>::unwrap_object(isolate, arg); \
    if (!result) {                       \
        g_throw(TypeError, "Argument `" #arg "` must be an instance of `CkImage`"); \
    }

#define EXTRACT_FONT_CHECKED(arg, result) \
    auto *result = binder::Class<CkFont>::unwrap_object(isolate, arg); \
    if (!result) {                       \
        g_throw(TypeError, "Argument `" #arg "` must be an instance of `CkFont`"); \
    }

#define CHECK_ENUM_RANGE(v, last) \
    if (v < 0 || v > static_cast<int32_t>(last)) { \
        g_throw(RangeError, "Invalid enumeration value for arguemnt `" #v "`"); \
    }

std::unique_ptr<SkRect> extract_maybe_rect(v8::Isolate *isolate,
                                           v8::Local<v8::Value> v)
{
    if (v->IsNullOrUndefined())
        return nullptr;
    return std::make_unique<SkRect>(ExtractCkRect(isolate, v));
}

SkPaint *extract_maybe_paint(v8::Isolate *isolate,
                             v8::Local<v8::Value> v,
                             const char *argname)
{
    if (v->IsNullOrUndefined())
        return nullptr;

    auto *w = binder::Class<CkPaint>::unwrap_object(isolate, v);
    if (!w)
        g_throw(TypeError, fmt::format("Argument `{}` must be an instance of `CkPaint`", argname));

    return &w->GetPaint();
}

SkMatrix *extract_maybe_matrix(v8::Isolate *isolate,
                             v8::Local<v8::Value> v,
                             const char *argname)
{
    if (v->IsNullOrUndefined())
        return nullptr;

    auto *w = binder::Class<CkMatrix>::unwrap_object(isolate, v);
    if (!w)
        g_throw(TypeError, fmt::format("Argument `{}` must be an instance of `CkMatrix`", argname));

    return &w->GetMatrix();
}

sk_sp<SkImageFilter> extract_maybe_imagefilter(v8::Isolate *isolate,
                                               v8::Local<v8::Value> v,
                                               const char *argname)
{
    if (v->IsNullOrUndefined())
        return nullptr;
    auto *w = binder::Class<CkImageFilterWrap>::unwrap_object(isolate, v);
    if (!w)
        g_throw(TypeError, fmt::format("Argument `{}` must be an instance of `CkImageFilter`", argname));
    return w->getSkiaObject();
}

} // namespace anonymous

int CkCanvas::save()
{
    return canvas_->save();
}

int CkCanvas::saveLayer(v8::Local<v8::Value> bounds, v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto maybe_bounds = extract_maybe_rect(isolate, bounds);
    SkPaint *maybe_paint = extract_maybe_paint(isolate, paint, "paint");
    return canvas_->saveLayer(maybe_bounds.get(), maybe_paint);
}

int CkCanvas::saveLayerAlpha(v8::Local<v8::Value> bounds, float alpha)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto maybe_bounds = extract_maybe_rect(isolate, bounds);
    alpha = std::clamp(alpha, 0.0f, 1.0f);
    return canvas_->saveLayerAlpha(maybe_bounds.get(),
                                   static_cast<uint8_t>(std::round(alpha * 255.0f)));
}

int CkCanvas::saveLayerRec(v8::Local<v8::Value> rec)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!rec->IsObject())
        g_throw(TypeError, "Argument `rec` must be an object");

    auto obj = v8::Local<v8::Object>::Cast(rec);
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

#define GET_PROP_CHECKED(name) \
    auto name##_v = obj->Get(ctx, binder::to_v8(isolate, #name)).FromMaybe(v8::Local<v8::Value>()); \
    if (name##_v.IsEmpty()) {  \
        g_throw(TypeError, "Argument `rec` misses required property `" #name "`"); \
    }

    GET_PROP_CHECKED(bounds)
    GET_PROP_CHECKED(paint)
    GET_PROP_CHECKED(backdrop)
    GET_PROP_CHECKED(flags)

#undef GET_PROP_CHECKED

    auto maybe_bounds = extract_maybe_rect(isolate, bounds_v);
    SkPaint *maybe_paint = extract_maybe_paint(isolate, paint_v, "rec.paint");
    sk_sp<SkImageFilter> maybe_backdrop = extract_maybe_imagefilter(isolate, backdrop_v, "rec.backdrop");
    if (!flags_v->IsNumber())
        g_throw(TypeError, "Argument `rec.flags` must be a number");

    SkCanvas::SaveLayerRec recpod(maybe_bounds.get(), maybe_paint, maybe_backdrop.get(),
                                  binder::from_v8<uint32_t>(isolate, flags_v));

    return canvas_->saveLayer(recpod);
}

void CkCanvas::restore()
{
    canvas_->restore();
}

void CkCanvas::restoreToCount(int saveCount)
{
    canvas_->restoreToCount(saveCount);
}

void CkCanvas::translate(SkScalar dx, SkScalar dy)
{
    canvas_->translate(dx, dy);
}

void CkCanvas::scale(SkScalar sx, SkScalar sy)
{
    canvas_->scale(sx, sy);
}

void CkCanvas::rotate(SkScalar rad, SkScalar px, SkScalar py)
{
    canvas_->rotate(SkRadiansToDegrees(rad), px, py);
}

void CkCanvas::skew(SkScalar sx, SkScalar sy)
{
    canvas_->skew(sx, sy);
}

void CkCanvas::concat(v8::Local<v8::Value> matrix)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_MAT_CHECKED(matrix, m)
    canvas_->concat(m->GetMatrix());
}

void CkCanvas::setMatrix(v8::Local<v8::Value> matrix)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_MAT_CHECKED(matrix, m)
    canvas_->setMatrix(SkM44(m->GetMatrix()));
}

void CkCanvas::resetMatrix()
{
    canvas_->resetMatrix();
}

void CkCanvas::clipRect(v8::Local<v8::Value> rect, int32_t op, bool AA)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_ENUM_RANGE(op, SkClipOp::kMax_EnumValue)
    canvas_->clipRect(ExtractCkRect(isolate, rect), static_cast<SkClipOp>(op), AA);
}

void CkCanvas::clipRRect(v8::Local<v8::Value> rrect, int32_t op, bool AA)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_ENUM_RANGE(op, SkClipOp::kMax_EnumValue)
    canvas_->clipRRect(ExtractCkRRect(isolate, rrect), static_cast<SkClipOp>(op), AA);
}

void CkCanvas::clipPath(v8::Local<v8::Value> path, int32_t op, bool AA)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_PATH_CHECKED(path, p)
    CHECK_ENUM_RANGE(op, SkClipOp::kMax_EnumValue)
    canvas_->clipPath(p->GetPath(), static_cast<SkClipOp>(op), AA);
}

void CkCanvas::clipShader(v8::Local<v8::Value> shader, int32_t op)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_ENUM_RANGE(op, SkClipOp::kMax_EnumValue)
    auto *wrapper = binder::Class<CkShaderWrap>::unwrap_object(isolate, shader);
    if (!wrapper)
        g_throw(TypeError, "Argument `shader` must be an instance of `CkShader`");
    canvas_->clipShader(wrapper->getSkiaObject(), static_cast<SkClipOp>(op));
}

bool CkCanvas::quickRejectRect(v8::Local<v8::Value> rect)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return canvas_->quickReject(ExtractCkRect(isolate, rect));
}

bool CkCanvas::quickRejectPath(v8::Local<v8::Value> path)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_PATH_CHECKED(path, p)
    return canvas_->quickReject(p->GetPath());
}

v8::Local<v8::Value> CkCanvas::getLocalClipBounds()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return WrapCkRect(isolate, canvas_->getLocalClipBounds());
}

v8::Local<v8::Value> CkCanvas::getDeviceClipBounds()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return WrapCkRect(isolate, SkRect::Make(canvas_->getDeviceClipBounds()));
}

void CkCanvas::drawColor(v8::Local<v8::Value> color, int32_t mode)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_ENUM_RANGE(mode, SkBlendMode::kLastMode)
    canvas_->drawColor(ExtractColor4f(isolate, color), static_cast<SkBlendMode>(mode));
}

void CkCanvas::clear(v8::Local<v8::Value> color)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    canvas_->clear(ExtractColor4f(isolate, color));
}

void CkCanvas::drawPaint(v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_PAINT_CHECKED(paint, p)
    canvas_->drawPaint(p->GetPaint());
}

void CkCanvas::drawPoint(SkScalar x, SkScalar y, v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_PAINT_CHECKED(paint, p)
    canvas_->drawPoint(x, y, p->GetPaint());
}

void CkCanvas::drawPoints(int32_t mode, v8::Local<v8::Value> points, v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_ENUM_RANGE(mode, SkBlendMode::kLastMode)

    if (!points->IsArray())
        g_throw(TypeError, "Argument `points` must be an array of `CkPoint`");

    auto arr = v8::Local<v8::Array>::Cast(points);
    auto nb_points = static_cast<int32_t>(arr->Length());
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

    std::vector<SkPoint> ptsvec(nb_points);
    for (int32_t i = 0; i < nb_points; i++)
    {
        auto v = arr->Get(ctx, i).ToLocalChecked();
        ptsvec[i] = ExtractCkPoint(isolate, v);
    }

    EXTRACT_PAINT_CHECKED(paint, p)
    canvas_->drawPoints(static_cast<SkCanvas::PointMode>(mode), nb_points,
                        ptsvec.data(), p->GetPaint());
}

void CkCanvas::drawLine(v8::Local<v8::Value> p1, v8::Local<v8::Value> p2,
                        v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_PAINT_CHECKED(paint, p)
    canvas_->drawLine(ExtractCkPoint(isolate, p1),
                      ExtractCkPoint(isolate, p2),
                      p->GetPaint());
}

void CkCanvas::drawRect(v8::Local<v8::Value> rect, v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_PAINT_CHECKED(paint, p)
    canvas_->drawRect(ExtractCkRect(isolate, rect), p->GetPaint());
}

void CkCanvas::drawOval(v8::Local<v8::Value> rect, v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_PAINT_CHECKED(paint, p)
    canvas_->drawRect(ExtractCkRect(isolate, rect), p->GetPaint());
}

void CkCanvas::drawRRect(v8::Local<v8::Value> rrect, v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_PAINT_CHECKED(paint, p)
    canvas_->drawRRect(ExtractCkRRect(isolate, rrect), p->GetPaint());
}

void CkCanvas::drawDRRect(v8::Local<v8::Value> outer, v8::Local<v8::Value> inner,
                          v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_PAINT_CHECKED(paint, p)
    canvas_->drawDRRect(ExtractCkRRect(isolate, outer), ExtractCkRRect(isolate, inner),
                        p->GetPaint());
}

void CkCanvas::drawCircle(SkScalar cx, SkScalar cy, SkScalar r, v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_PAINT_CHECKED(paint, p)
    canvas_->drawCircle(cx, cy, r, p->GetPaint());
}

void CkCanvas::drawArc(v8::Local<v8::Value> oval, SkScalar startAngle,
                       SkScalar sweepAngle, bool useCenter,
                       v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_PAINT_CHECKED(paint, p)
    canvas_->drawArc(ExtractCkRect(isolate, oval), startAngle, sweepAngle, useCenter,
                     p->GetPaint());
}

void CkCanvas::drawRoundRect(v8::Local<v8::Value> rect, SkScalar rx, SkScalar ry,
                             v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_PAINT_CHECKED(paint, p)
    canvas_->drawRoundRect(ExtractCkRect(isolate, rect), rx, ry, p->GetPaint());
}

void CkCanvas::drawPath(v8::Local<v8::Value> path, v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_PATH_CHECKED(path, wpath)
    EXTRACT_PAINT_CHECKED(paint, wpaint)
    canvas_->drawPath(wpath->GetPath(), wpaint->GetPaint());
}

void CkCanvas::drawImage(v8::Local<v8::Value> image, SkScalar left,
                         SkScalar top, int32_t sampling,
                         v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_IMAGE_CHECKED(image, i)

    canvas_->drawImage(i->getImage(), left, top, SamplingToSamplingOptions(sampling),
                       extract_maybe_paint(isolate, paint, "paint"));
}

void CkCanvas::drawImageRect(v8::Local<v8::Value> image, v8::Local<v8::Value> src,
                             v8::Local<v8::Value> dst, int32_t sampling,
                             v8::Local<v8::Value> paint, int32_t constraint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_IMAGE_CHECKED(image, i)
    CHECK_ENUM_RANGE(constraint, SkCanvas::SrcRectConstraint::kFast_SrcRectConstraint)
    canvas_->drawImageRect(i->getImage(), ExtractCkRect(isolate, src),
                           ExtractCkRect(isolate, dst), SamplingToSamplingOptions(sampling),
                           extract_maybe_paint(isolate, paint, "paint"),
                           static_cast<SkCanvas::SrcRectConstraint>(constraint));
}

void CkCanvas::drawString(const std::string& str, SkScalar x, SkScalar y,
                          v8::Local<v8::Value> font, v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_FONT_CHECKED(font, ft)
    EXTRACT_PAINT_CHECKED(paint, p)
    canvas_->drawString(str.c_str(), x, y, ft->GetFont(), p->GetPaint());
}

#define GET_TA_WRPTR_CHECKED(type, arg, result_ptr, result_len) \
    if (!arg->Is##type()) { \
        g_throw(TypeError, "Argument `" #arg "` must be a `" #type "`"); \
    }                                           \
    auto arg##_arr = v8::Local<v8::type>::Cast(arg);            \
    size_t result_len = arg##_arr->Length();                   \
    void *result_ptr = (uint8_t*)arg##_arr->Buffer()->Data() + arg##_arr->ByteOffset();

void CkCanvas::drawGlyphs(v8::Local<v8::Value> glyphs, v8::Local<v8::Value> positions,
                          v8::Local<v8::Value> origin, v8::Local<v8::Value> font,
                          v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_FONT_CHECKED(font, ft)
    EXTRACT_PAINT_CHECKED(paint, p)
    GET_TA_WRPTR_CHECKED(Uint16Array, glyphs, glyphs_ptr, nb_glyphs)

    if (!positions->IsArray())
        g_throw(TypeError, "Argument `positions` must be an array of `CkPoint`");

    auto positions_arr = v8::Local<v8::Array>::Cast(positions);
    std::vector<SkPoint> pos_vec(nb_glyphs);
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    for (size_t i = 0; i < nb_glyphs; i++)
    {
        auto v = positions_arr->Get(ctx, i).ToLocalChecked();
        pos_vec[i] = ExtractCkPoint(isolate, v);
    }

    canvas_->drawGlyphs(static_cast<int32_t>(nb_glyphs),
                        reinterpret_cast<const SkGlyphID*>(glyphs_ptr),
                        pos_vec.data(), ExtractCkPoint(isolate, origin),
                        ft->GetFont(), p->GetPaint());
}

void CkCanvas::drawTextBlob(v8::Local<v8::Value> blob, SkScalar x, SkScalar y,
                            v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *blobwrap = binder::Class<CkTextBlob>::unwrap_object(isolate, blob);
    if (!blobwrap)
        g_throw(TypeError, "Argument `blob` must be an instance of `CkTextBlob`");

    EXTRACT_PAINT_CHECKED(paint, p)
    canvas_->drawTextBlob(blobwrap->getSkiaObject(), x, y, p->GetPaint());
}

void CkCanvas::drawPicture(v8::Local<v8::Value> picture, v8::Local<v8::Value> matrix,
                           v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *pict = binder::Class<CkPictureWrap>::unwrap_object(isolate, picture);
    if (!pict)
        g_throw(TypeError, "Argument `picture` must be an instance of `CkPicture`");
    canvas_->drawPicture(pict->getPicture(),
                         extract_maybe_matrix(isolate, matrix, "matrix"),
                         extract_maybe_paint(isolate, paint, "paint"));
}

void CkCanvas::drawVertices(v8::Local<v8::Value> vertices, int32_t mode,
                            v8::Local<v8::Value> paint)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_ENUM_RANGE(mode, SkBlendMode::kLastMode)
    EXTRACT_PAINT_CHECKED(paint, p)
    auto *vw = binder::Class<CkVertices>::unwrap_object(isolate, vertices);
    if (!vw)
        g_throw(TypeError, "Argument `vertices` must be an instance of `CkVertices`");
    canvas_->drawVertices(vw->getSkiaObject(), static_cast<SkBlendMode>(mode), p->GetPaint());
}

void CkCanvas::drawPatch(v8::Local<v8::Value> cubics, v8::Local<v8::Value> colors,
                         v8::Local<v8::Value> texCoords, int32_t mode,
                         v8::Local<v8::Value> paint)
{
    // TODO(sora): implement this
}

GALLIUM_BINDINGS_GLAMOR_NS_END
