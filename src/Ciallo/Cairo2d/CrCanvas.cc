#include "Ciallo/Cairo2d/CrCanvas.h"
CIALLO_BEGIN_NS

CrCanvas::CrCanvas(const std::shared_ptr<CrSurface>& surface)
    : fSurface(surface),
      fCairo(nullptr),
      fReferenced{ nullptr }
{
    fCairo = cairo_create(fSurface->nativeHandle());
}

CrCanvas::~CrCanvas()
{
    fReferenced.sourceSurface = nullptr;
    if (fCairo)
        cairo_destroy(fCairo);
}

void CrCanvas::save()
{
    cairo_save(fCairo);
}

void CrCanvas::restore()
{
    cairo_restore(fCairo);
}

void CrCanvas::pushGroup()
{
    cairo_push_group(fCairo);
}

void CrCanvas::pushGroup(cairo_content_t content)
{
    cairo_push_group_with_content(fCairo, content);
}

void CrCanvas::popGroup()
{
    cairo_pop_group(fCairo);
}

void CrCanvas::popGroupToSource()
{
    cairo_pop_group_to_source(fCairo);
}

void CrCanvas::setOperator(cairo_operator_t op)
{
    cairo_set_operator(fCairo, op);
}

void CrCanvas::setSource(CrScalar r, CrScalar g, CrScalar b)
{
    fReferenced.sourceSurface = nullptr;
    cairo_set_source_rgb(fCairo, r, g, b);
}

void CrCanvas::setSource(CrScalar r, CrScalar g, CrScalar b, CrScalar a)
{
    fReferenced.sourceSurface = nullptr;
    cairo_set_source_rgba(fCairo, r, g, b, a);
}

void CrCanvas::setSource(const std::shared_ptr<CrSurface>& surface, CrScalar x, CrScalar y)
{
    if (surface == nullptr || surface->nativeHandle() == nullptr)
        return;
    fReferenced.sourceSurface = surface;
    cairo_set_source_surface(fCairo, surface->nativeHandle(), x, y);
}

void CrCanvas::setTolerance(CrScalar tolerance)
{
    cairo_set_tolerance(fCairo, tolerance);
}

void CrCanvas::setAntialias(cairo_antialias_t antialias)
{
    cairo_set_antialias(fCairo, antialias);
}

void CrCanvas::setFillRule(cairo_fill_rule_t rule)
{
    cairo_set_fill_rule(fCairo, rule);
}

void CrCanvas::setLineWidth(CrScalar width)
{
    cairo_set_line_width(fCairo, width);
}

void CrCanvas::setLineCap(cairo_line_cap_t cap)
{
    cairo_set_line_cap(fCairo, cap);
}

void CrCanvas::setLineJoin(cairo_line_join_t join)
{
    cairo_set_line_join(fCairo, join);
}

void CrCanvas::setDash(const CrScalar *dashes, int num, CrScalar offset)
{
    cairo_set_dash(fCairo, dashes, num, offset);
}

void CrCanvas::setMiterLimit(CrScalar limit)
{
    cairo_set_miter_limit(fCairo, limit);
}

void CrCanvas::setMatrix(const cairo_matrix_t *matrix)
{
    cairo_set_matrix(fCairo, matrix);
}

void CrCanvas::translate(CrScalar tx, CrScalar ty)
{
    cairo_translate(fCairo, tx, ty);
}

void CrCanvas::scale(CrScalar sx, CrScalar sy)
{
    cairo_scale(fCairo, sx, sy);
}

void CrCanvas::rotate(CrScalar angle)
{
    cairo_rotate(fCairo, angle);
}

void CrCanvas::transform(const cairo_matrix_t *matrix)
{
    cairo_transform(fCairo, matrix);
}

void CrCanvas::identityMatrix()
{
    cairo_identity_matrix(fCairo);
}

void CrCanvas::moveTo(CrScalar x, CrScalar y)
{
    cairo_move_to(fCairo, x, y);
}

void CrCanvas::lineTo(CrScalar x, CrScalar y)
{
    cairo_line_to(fCairo, x, y);
}

void CrCanvas::curveTo(CrScalar x1, CrScalar y1, CrScalar x2, CrScalar y2, CrScalar x3, CrScalar y3)
{
    cairo_curve_to(fCairo, x1, y1, x2, y2, x3, y3);
}

void CrCanvas::drawArc(CrScalar cx, CrScalar cy, CrScalar radius, CrScalar angle1, CrScalar angle2)
{
    cairo_arc(fCairo, cx, cy, radius, angle1, angle2);
}

void CrCanvas::drawRect(CrScalar x, CrScalar y, CrScalar width, CrScalar height)
{
    cairo_rectangle(fCairo, x, y, width, height);
}

void CrCanvas::closePath()
{
    cairo_close_path(fCairo);
}

void CrCanvas::drawPaint()
{
    cairo_paint(fCairo);
}

void CrCanvas::drawPaint(double alpha)
{
    cairo_paint_with_alpha(fCairo, alpha);
}

void CrCanvas::drawStroke()
{
    cairo_stroke(fCairo);
}

void CrCanvas::drawFill()
{
    cairo_fill(fCairo);
}

void CrCanvas::clip()
{
    cairo_clip(fCairo);
}

void CrCanvas::resetClip()
{
    cairo_reset_clip(fCairo);
}

void CrCanvas::selectFontFace(const std::string& family, cairo_font_slant_t slant, cairo_font_weight_t weight)
{
    cairo_select_font_face(fCairo, family.c_str(), slant, weight);
}

void CrCanvas::setFontSize(double size)
{
    cairo_set_font_size(fCairo, size);
}

void CrCanvas::setFontMatrix(const cairo_matrix_t *matrix)
{
    cairo_set_font_matrix(fCairo, matrix);
}

void CrCanvas::drawText(const char *utf8)
{
    cairo_show_text(fCairo, utf8);
}

cairo_text_extents_t CrCanvas::textExtents(const char *utf8)
{
    cairo_text_extents_t extents;
    cairo_text_extents(fCairo, utf8, &extents);
    return extents;
}

void CrCanvas::setFontOptions(cairo_font_options_t *options)
{
    cairo_set_font_options(fCairo, options);
}

void CrCanvas::newPath()
{
    cairo_new_path(fCairo);
}

void CrCanvas::newSubPath()
{
    cairo_new_sub_path(fCairo);
}

CIALLO_END_NS
