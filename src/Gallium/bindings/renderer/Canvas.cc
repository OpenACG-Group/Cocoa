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

#include "Core/TraceEvent.h"

#include "Gallium/bindings/renderer/Canvas.h"
#include "Gallium/bindings/renderer/Picture.h"
#include "Gallium/bindings/renderer/Shader.h"
#include "Gallium/bindings/renderer/Font.h"
#include "Gallium/bindings/renderer/Vertices.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

ffi::Ret<SaveLayerRecAdapter>
SaveLayerRecAdapter::Cast(v8::Isolate *isolate, v8::Local<v8::Value> value)
{
    auto iface = ffi::Cast<ffi::IFace<SaveLayerRec>>::From(isolate, value);
    if (iface.HasError())
        return iface.GetError();

    SaveLayerRec *rec = iface.Extract().Get();
    SaveLayerRecAdapter result;
    if (rec->bounds)
    {
        ffi::Ret<SkRect> bounds = UnwrapJSRect(isolate, *rec->bounds);
        if (bounds.HasError())
            return bounds.GetError();
        result.bounds = bounds.Extract();
        result.rec.fBounds = &result.bounds;
    }

    if (rec->paint)
        result.rec.fPaint = &(*rec->paint)->GetSkPaint();

    if (rec->flags)
        result.rec.fSaveLayerFlags = *rec->flags;

    if (rec->backdrop)
    {
        result.backdrop = (*rec->backdrop)->GetSkImageFilter();
        result.rec.fBackdrop = result.backdrop.get();
    }

    if (rec->filters)
    {
        v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
        v8::Local<v8::Array> arr = *rec->filters;
        uint32_t size = arr->Length();
        result.filters.reserve(size);
        for (uint32_t i = 0; i < size; i++)
        {
            v8::Local<v8::Value> element;
            if (!arr->Get(ctx, i).ToLocal(&element))
                return ffi::FreePropagate();
            auto ret = ffi::Cast<ffi::Class<ImageFilter>>::From(isolate, element);
            if (ret.HasError())
                return ret.GetError();
            result.filters.push_back(ret.Extract()->GetSkImageFilter());
        }

        result.rec.fFilters = result.filters;
    }

    return result;
}

Canvas::Canvas(SkCanvas *canvas, v8::Local<v8::Object> parent)
    : canvas_(canvas)
    , parent_(v8::Isolate::GetCurrent(), parent)
{
    EmitterDefineEvent("release", [this]() -> uint64_t {
        rel_event_emit_ = EmitterWrapAsCallable("release");
        return 0;
    }, [this]([[maybe_unused]] uint64_t id) {
        rel_event_emit_ = {};
    });
}

void Canvas::OnParentDispose()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (rel_event_emit_)
        rel_event_emit_({ GetThisHandle(isolate) });

    rel_event_emit_ = {};
    canvas_ = nullptr;
    parent_.Reset();
    NotifyDisposeState(DisposeState::kDisposed);
}

ffi::Ret<int32_t> Canvas::save()
{
    TRACE_EVENT("renderer", "Canvas::save");
    return canvas_->save();
}

ffi::Ret<int32_t> Canvas::saveLayer(ffi::Opt<RectAdapter> bounds,
                                    ffi::Opt<ffi::Class<Paint>> paint)
{
    TRACE_EVENT("renderer", "Canvas::saveLayer");
    return canvas_->saveLayer(bounds ? &(**bounds) : nullptr,
                              paint ? &(*paint)->GetSkPaint() : nullptr);
}

ffi::Ret<int32_t> Canvas::saveLayerAlphaf(ffi::Opt<RectAdapter> bounds, float alpha)
{
    TRACE_EVENT("renderer", "Canvas::saveLayerAlphaf");
    return canvas_->saveLayerAlphaf(bounds ? &(**bounds) : nullptr, alpha);
}

ffi::Ret<int32_t> Canvas::saveLayerRec(const SaveLayerRecAdapter& rec)
{
    TRACE_EVENT("renderer", "Canvas::saveLayerRec");
    return canvas_->saveLayer(*rec);
}

ffi::Ret<void> Canvas::restore()
{
    TRACE_EVENT("renderer", "Canvas::restore");
    canvas_->restore();
    return {};
}

ffi::Ret<int32_t> Canvas::getSaveCount()
{
    return canvas_->getSaveCount();
}

ffi::Ret<void> Canvas::restoreToCount(int32_t save_count)
{
    TRACE_EVENT("renderer", "Canvas::restoreToCount");
    canvas_->restoreToCount(save_count);
    return {};
}

ffi::Ret<void> Canvas::translate(float dx, float dy)
{
    canvas_->translate(dx, dy);
    return {};
}

ffi::Ret<void> Canvas::scale(float sx, float sy)
{
    canvas_->scale(sx, sy);
    return {};
}

ffi::Ret<void> Canvas::rotate(float degrees)
{
    canvas_->rotate(degrees);
    return {};
}

ffi::Ret<void> Canvas::rotatePivot(float degrees, float px, float py)
{
    canvas_->rotate(degrees, px, py);
    return {};
}

ffi::Ret<void> Canvas::skew(float sx, float sy)
{
    canvas_->skew(sx, sy);
    return {};
}

ffi::Ret<void> Canvas::concat33(const Mat3x3Adapter& mat)
{
    canvas_->concat(*mat);
    return {};
}

ffi::Ret<void> Canvas::concat44(const Mat4x4Adapter& mat)
{
    canvas_->concat(*mat);
    return {};
}

ffi::Ret<void> Canvas::setMatrix(const Mat4x4Adapter& mat)
{
    canvas_->setMatrix(*mat);
    return {};
}

ffi::Ret<void> Canvas::resetMatrix()
{
    canvas_->resetMatrix();
    return {};
}

ffi::Ret<void> Canvas::clipRect(RectAdapter rect, ffi::Enum<SkClipOp> op, bool do_anti_alias)
{
    canvas_->clipRect(*rect, *op, do_anti_alias);
    return {};
}

ffi::Ret<void> Canvas::clipRRect(RRectAdapter rrect, ffi::Enum<SkClipOp> op, bool do_anti_alias)
{
    canvas_->clipRRect(*rrect, *op, do_anti_alias);
    return {};
}

ffi::Ret<void> Canvas::clipPath(ffi::Class<Path> path, ffi::Enum<SkClipOp> op, bool do_anti_alias)
{
    canvas_->clipPath((*path)->GetSkPath(), *op, do_anti_alias);
    return {};
}

ffi::Ret<void> Canvas::clipShader(ffi::Class<Shader> shader, ffi::Enum<SkClipOp> op)
{
    canvas_->clipShader(shader->GetSkShader(), *op);
    return {};
}

ffi::Ret<bool> Canvas::quickRejectRect(RectAdapter rect)
{
    return canvas_->quickReject(*rect);
}

ffi::Ret<bool> Canvas::quickRejectPath(ffi::Class<Path> path)
{
    return canvas_->quickReject((*path)->GetSkPath());
}

ffi::RetLocal<v8::Value> Canvas::getLocalClipBounds()
{
    return CreateJSRect(v8::Isolate::GetCurrent(), canvas_->getLocalClipBounds());
}

ffi::RetLocal<v8::Value> Canvas::getDeviceClipBounds()
{
    return CreateJSRect(v8::Isolate::GetCurrent(), SkRect::Make(canvas_->getDeviceClipBounds()));
}

ffi::Ret<void> Canvas::drawColor(uint32_t color, ffi::Enum<SkBlendMode> mode)
{
    TRACE_EVENT("renderer", "Canvas::drawColor");
    canvas_->drawColor(color, *mode);
    return {};
}

ffi::Ret<void> Canvas::drawColor4f(Color4fQuadruple color, ffi::Enum<SkBlendMode> mode)
{
    TRACE_EVENT("renderer", "Canvas::drawColor4f");
    auto [R, G, B, A] = color;
    canvas_->drawColor(SkColor4f{R, G, B, A}, *mode);
    return {};
}

ffi::Ret<void> Canvas::clear(Color4fQuadruple color)
{
    TRACE_EVENT("renderer", "Canvas::clear");
    auto [R, G, B, A] = color;
    canvas_->clear(SkColor4f{R, G, B, A});
    return {};
}

ffi::Ret<void> Canvas::discard()
{
    canvas_->discard();
    return {};
}

ffi::Ret<void> Canvas::drawPaint(ffi::Class<Paint> paint)
{
    TRACE_EVENT("renderer", "Canvas::drawPaint");
    canvas_->drawPaint(paint->GetSkPaint());
    return {};
}

ffi::Ret<void> Canvas::drawPoints(ffi::Enum<SkCanvas::PointMode> mode,
                                  const ffi::Mem<float>& pts_memory,
                                  ffi::Class<Paint> paint)
{
    TRACE_EVENT("renderer", "Canvas::drawPoints");
    if (pts_memory.Size() & 1)
        return ffi::Fail(ffi::kErr, "length of flattened points array is invalid");

    canvas_->drawPoints(*mode, pts_memory.Size() >> 1,
                        reinterpret_cast<SkPoint*>(pts_memory.Address()),
                        paint->GetSkPaint());
    return {};
}

ffi::Ret<void> Canvas::drawPoint(float x, float y, ffi::Class<Paint> paint)
{
    TRACE_EVENT("renderer", "Canvas::drawPoint");
    canvas_->drawPoint(x, y, paint->GetSkPaint());
    return {};
}

ffi::Ret<void> Canvas::drawLine(float x0, float y0, float x1, float y1, ffi::Class<Paint> paint)
{
    TRACE_EVENT("renderer", "Canvas::drawLine");
    canvas_->drawLine(x0, y0, x1, y1, paint->GetSkPaint());
    return {};
}

ffi::Ret<void> Canvas::drawRect(RectAdapter rect, ffi::Class<Paint> paint)
{
    TRACE_EVENT("renderer", "Canvas::drawRect");
    canvas_->drawRect(*rect, paint->GetSkPaint());
    return {};
}

ffi::Ret<void> Canvas::drawOval(RectAdapter oval, ffi::Class<Paint> paint)
{
    TRACE_EVENT("renderer", "Canvas::drawOval");
    canvas_->drawOval(*oval, paint->GetSkPaint());
    return {};
}

ffi::Ret<void> Canvas::drawRRect(RRectAdapter rrect, ffi::Class<Paint> paint)
{
    TRACE_EVENT("renderer", "Canvas::drawRRect");
    canvas_->drawRRect(*rrect, paint->GetSkPaint());
    return {};
}

ffi::Ret<void> Canvas::drawDRRect(RRectAdapter outer, RRectAdapter inner, ffi::Class<Paint> paint)
{
    TRACE_EVENT("renderer", "Canvas::drawDRRect");
    canvas_->drawDRRect(*outer, *inner, paint->GetSkPaint());
    return {};
}

ffi::Ret<void> Canvas::drawCircle(float cx, float cy, float radius, ffi::Class<Paint> paint)
{
    TRACE_EVENT("renderer", "Canvas::drawCircle");
    canvas_->drawCircle(cx, cy, radius, paint->GetSkPaint());
    return {};
}

ffi::Ret<void> Canvas::drawArc(RectAdapter oval, float start_angle_deg,
                               float sweep_angle_deg, bool use_center, ffi::Class<Paint> paint)
{
    TRACE_EVENT("renderer", "Canvas::drawArc");
    canvas_->drawArc(*oval, start_angle_deg, sweep_angle_deg, use_center, paint->GetSkPaint());
    return {};
}

ffi::Ret<void> Canvas::drawRoundRect(RectAdapter rect, float rx, float ry, ffi::Class<Paint> paint)
{
    TRACE_EVENT("renderer", "Canvas::drawRoundRect");
    canvas_->drawRoundRect(*rect, rx, ry, paint->GetSkPaint());
    return {};
}

ffi::Ret<void> Canvas::drawPath(ffi::Class<Path> path, ffi::Class<Paint> paint)
{
    TRACE_EVENT("renderer", "Canvas::drawPath");
    canvas_->drawPath((*path)->GetSkPath(), (*paint)->GetSkPaint());
    return {};
}

#define OPT_PAINT(p) ((p) ? &(*(p))->GetSkPaint() : nullptr)
#define OPT_MATRIX(m) ((m) ? &(**m) : nullptr)

ffi::Ret<void> Canvas::drawImage(ffi::Class<Image> image, float left, float top,
                                 SamplingOptionsAdapter sampling, ffi::Opt<ffi::Class<Paint>> paint)
{
    TRACE_EVENT("renderer", "Canvas::drawImage");
    canvas_->drawImage(image->GetSkImage(), left, top, *sampling, OPT_PAINT(paint));
    return {};
}

ffi::Ret<void> Canvas::drawImageRect(ffi::Class<Image> image, RectAdapter dst,
                                     SamplingOptionsAdapter sampling, ffi::Opt<ffi::Class<Paint>> paint)
{
    TRACE_EVENT("renderer", "Canvas::drawImageRect");
    canvas_->drawImageRect(image->GetSkImage(), *dst, *sampling, OPT_PAINT(paint));
    return {};
}

ffi::Ret<void> Canvas::drawImageRectToRect(ffi::Class<Image> image, RectAdapter src, RectAdapter dst,
                                           SamplingOptionsAdapter sampling, ffi::Opt<ffi::Class<Paint>> paint,
                                           bool strict_constraint)
{
    TRACE_EVENT("renderer", "Canvas::drawImageRectToRect");
    canvas_->drawImageRect(image->GetSkImage(), *src, *dst, *sampling, OPT_PAINT(paint),
                           strict_constraint ? SkCanvas::kStrict_SrcRectConstraint
                                             : SkCanvas::kFast_SrcRectConstraint);
    return {};
}

ffi::Ret<void> Canvas::drawPicture(const ffi::Class<Picture>& picture,
                                   const ffi::Opt<Mat3x3Adapter>& matrix,
                                   const ffi::Opt<ffi::Class<Paint>>& paint)
{
    TRACE_EVENT("renderer", "Canvas::drawPicture");
    canvas_->drawPicture(picture->GetSkPicture(), OPT_MATRIX(matrix), OPT_PAINT(paint));
    return {};
}

ffi::Ret<void> Canvas::drawString(v8::Local<v8::String> text, float x, float y,
                                  const ffi::Class<Font>& font, const ffi::Class<Paint>& paint)
{
    TRACE_EVENT("renderer", "Canvas::drawString");
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::String::Value utf16_value(isolate, text);
    if (utf16_value.length() == 0)
        return {};

    canvas_->drawSimpleText(*utf16_value, utf16_value.length() * sizeof(uint16_t),
                            SkTextEncoding::kUTF16, x, y, font->GetSkFont(), paint->GetSkPaint());
    return {};
}

ffi::Ret<void> Canvas::drawGlyphs(const ffi::Mem<uint16_t>& glyphs, const RSXformArrayAdapter& xforms,
                                  float origin_x, float origin_y,
                                  const ffi::Class<Font>& font, const ffi::Class<Paint>& paint)
{
    TRACE_EVENT("renderer", "Canvas::drawGlyphs");
    if (glyphs.Size() == 0)
        return {};
    if (xforms.count != glyphs.Size())
        return ffi::Fail(ffi::kErr, "size of RSXformArray does not match glyphs count");
    canvas_->drawGlyphs(static_cast<int>(glyphs.Size()), glyphs.Address(), xforms.address,
                        {origin_x, origin_y}, font->GetSkFont(), paint->GetSkPaint());
    return {};
}

ffi::Ret<void> Canvas::drawVertices(ffi::Class<Vertices> vertices, ffi::Enum<SkBlendMode> mode,
                                    ffi::Class<Paint> paint)
{
    TRACE_EVENT("renderer", "Canvas::drawVertices");
    canvas_->drawVertices(vertices->GetSkVertices(), *mode, paint->GetSkPaint());
    return {};
}

GALLIUM_BINDINGS_RENDERER_NS_END
