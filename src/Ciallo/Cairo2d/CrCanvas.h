#ifndef COCOA_CRCANVAS_H
#define COCOA_CRCANVAS_H

#include <cairo.h>

#include "Ciallo/Cairo2d/Cairo2d.h"
#include "Ciallo/Cairo2d/CrSurface.h"
CIALLO_BEGIN_NS

class CrCanvas
{
public:
    explicit CrCanvas(const std::shared_ptr<CrSurface>& surface);
    ~CrCanvas();

    void save();
    void restore();

    void pushGroup();
    void pushGroup(cairo_content_t content);
    void popGroup();
    void popGroupToSource();

    void setOperator(cairo_operator_t op);
    void setSource(CrScalar r, CrScalar g, CrScalar b);
    void setSource(CrScalar r, CrScalar g, CrScalar b, CrScalar a);
    void setSource(const std::shared_ptr<CrSurface>& surface, CrScalar x, CrScalar y);
    void setTolerance(CrScalar tolerance);
    void setAntialias(cairo_antialias_t antialias);
    void setFillRule(cairo_fill_rule_t rule);
    void setLineWidth(CrScalar width);
    void setLineCap(cairo_line_cap_t cap);
    void setLineJoin(cairo_line_join_t join);
    void setDash(const CrScalar *dashes, int num, CrScalar offset);
    void setMiterLimit(CrScalar limit);
    void setMatrix(const cairo_matrix_t *matrix);

    void translate(CrScalar tx, CrScalar ty);
    void scale(CrScalar sx, CrScalar sy);
    void rotate(CrScalar angle);
    void transform(const cairo_matrix_t *matrix);
    void identityMatrix();

    void moveTo(CrScalar x, CrScalar y);
    void lineTo(CrScalar x, CrScalar y);
    void curveTo(CrScalar x1, CrScalar y1,
                 CrScalar x2, CrScalar y2,
                 CrScalar x3, CrScalar y3);
    void drawArc(CrScalar cx, CrScalar cy, CrScalar radius, CrScalar angle1, CrScalar angle2);
    void drawRect(CrScalar x, CrScalar y, CrScalar width, CrScalar height);
    void closePath();

    void drawPaint();
    void drawPaint(double alpha);
    void drawStroke();
    void drawFill();

    void clip();
    void resetClip();

    void selectFontFace(const std::string& family,
                        cairo_font_slant_t slant,
                        cairo_font_weight_t weight);
    void setFontSize(double size);
    void setFontMatrix(const cairo_matrix_t *matrix);
    void setFontOptions(cairo_font_options_t *options);
    void drawText(const char *utf8);
    void newPath();
    void newSubPath();
    cairo_text_extents_t textExtents(const char *utf8);
    inline cairo_t *nativeHandle()
    {
        return fCairo;
    }

private:
    struct References
    {
        std::shared_ptr<CrSurface>  sourceSurface;
    };

    std::shared_ptr<CrSurface>  fSurface;
    cairo_t                    *fCairo;
    References                  fReferenced;
};

CIALLO_END_NS
#endif //COCOA_CRCANVAS_H
