#include <iostream>

#include "Core/Project.h"
#include "Ciallo/Cairo2d/CrSurface.h"
#include "Ciallo/Cairo2d/CrCanvas.h"
#include "NaGui/Draw.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#pragma clang diagnostic ignored "-Wunused-function"
#endif
#define NK_IMPLEMENTATION
#include "NaGui/nuklear.h"
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include "NaGui/DrawContext.h"
NAGUI_NS_BEGIN

namespace {

struct FontSelector
{
    cairo_t                *pCairo;
    cairo_font_face_t      *pFace;
    double                  size;
    cairo_font_extents_t    extents;
};

//TODO: Memory monitoring for nuklear
void *nuklear_allocator_alloc(nk_handle handle, void *old, size_t size)
{
    void *ptr = nullptr;
    if (old != nullptr)
        ptr = std::realloc(old, size);
    else
        ptr = std::malloc(size);
    return ptr;
}

void nuklear_allocator_free(nk_handle handle, void *old)
{
    if (old == nullptr)
        return;
    std::free(old);
}

nk_allocator g_nuklear_allocator{
    .userdata = { nullptr },
    .alloc = nuklear_allocator_alloc,
    .free = nuklear_allocator_free
};

float nuklear_text_width(nk_handle handle, float height, const char *text, int len)
{
    auto *font = reinterpret_cast<FontSelector*>(handle.ptr);
    cairo_set_font_face(font->pCairo, font->pFace);
    cairo_set_font_size(font->pCairo, font->size);
    cairo_text_extents_t extents;
    cairo_text_extents(font->pCairo, text, &extents);
    return static_cast<float>(extents.width);
}

} // namespace anonymous

Draw::Draw(ciallo::CrSurface::Ptr surface)
    : fSurface(std::move(surface)),
      fCanvas(fSurface),
      fpCtx(new nk_context)
{
    fSurface = ciallo::CrSurface::MakeRecording(CAIRO_CONTENT_COLOR_ALPHA);
    int32_t tf = addTypeface("Consolas", CAIRO_FONT_SLANT_NORMAL,
                             CAIRO_FONT_WEIGHT_NORMAL, 13);
    nk_bool result = nk_init(fpCtx, &g_nuklear_allocator, fTypefaces[tf]);
    if (result == nk_false)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Failed to create a Nuklear context")
                .make<RuntimeException>();
    }
}

Draw::~Draw()
{
    nk_free(fpCtx);
    for (nk_user_font *pFont : fTypefaces)
    {
        delete reinterpret_cast<FontSelector *>(pFont->userdata.ptr);
        delete pFont;
    }
    delete fpCtx;
}

int32_t Draw::addTypeface(const std::string& family, cairo_font_slant_t slant,
                          cairo_font_weight_t weight, double size)
{
    auto userFont = new nk_user_font;
    auto fontSelector = new FontSelector;

    fCanvas.selectFontFace(family, slant, weight);
    fCanvas.setFontSize(size);
    fontSelector->pCairo = fCanvas.nativeHandle();
    fontSelector->pFace = cairo_get_font_face(fCanvas.nativeHandle());
    fontSelector->size = size;
    cairo_font_extents(fCanvas.nativeHandle(), &fontSelector->extents);

    userFont->userdata = nk_handle_ptr(fontSelector);
    userFont->width = nuklear_text_width;
    userFont->height = static_cast<float>(fontSelector->extents.height);

    fTypefaces.push_back(userFont);
    return fTypefaces.size() - 1;
}

void Draw::test()
{
    if (nk_begin(fpCtx, "Demo", nk_rect(50, 50, 200, 200),
                 NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
                 NK_WINDOW_CLOSABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
    {
        enum {EASY, HARD};
        static int op = EASY;
        static int property = 20;

        nk_layout_row_static(fpCtx, 30, 80, 1);
        if (nk_button_label(fpCtx, "button"))
            fprintf(stdout, "button pressed\n");
        nk_layout_row_dynamic(fpCtx, 30, 2);
        if (nk_option_label(fpCtx, "easy", op == EASY)) op = EASY;
        if (nk_option_label(fpCtx, "hard", op == HARD)) op = HARD;
        nk_layout_row_dynamic(fpCtx, 25, 1);
        nk_property_int(fpCtx, "Compression:", 0, &property, 100, 10, 1);
    }
    nk_end(fpCtx);
}

void Draw::_privBeginInput()
{
    nk_input_begin(fpCtx);
}

void Draw::_privEndInput()
{
    nk_input_end(fpCtx);
}

void Draw::_privMouseLeftButton(bool down, const Vec2i& pos)
{
    nk_input_button(fpCtx, NK_BUTTON_LEFT, pos.s0, pos.s1, down);
}

void Draw::_privMouseRightButton(bool down, const Vec2i& pos)
{
    nk_input_button(fpCtx, NK_BUTTON_RIGHT, pos.s0, pos.s1, down);
}

void Draw::_privMouseMiddleButton(bool down, const Vec2i& pos)
{
    nk_input_button(fpCtx, NK_BUTTON_MIDDLE, pos.s0, pos.s1, down);
}

void Draw::_privMouseMotion(const Vec2i& pos)
{
    nk_input_motion(fpCtx, pos.s0, pos.s1);
}

#define EXTRA_CMD(type)     auto *type = reinterpret_cast<const nk_command_##type*>(cmd);
#define FLT_COLOR(c)        (c.r) / 255.0, (c.g) / 255.0, (c.b) / 255.0, (c.a) / 255.0

namespace {

void drawArcPathXYWH(ciallo::CrCanvas& canvas,
                     double x, double y,
                     double w, double h,
                     double angle0, double angle1)
{
    /* See also: Python code in script/cairo-draw-arc.py */
    cairo_matrix_t savedMatrix;
    cairo_get_matrix(canvas.nativeHandle(), &savedMatrix);
    double ox = x + w / 2.0;
    double oy = y + h / 2.0;
    canvas.translate(ox, oy);
    canvas.scale(w / h, 1);
    canvas.translate(-ox, -oy);
    canvas.drawArc(ox, oy, h / 2, angle0, angle1);
    canvas.setMatrix(&savedMatrix);
}

void drawTrianglePath(ciallo::CrCanvas& canvas, const Vec2d *vert)
{
    canvas.moveTo(vert[0].s0, vert[0].s1);
    canvas.lineTo(vert[1].s0, vert[1].s1);
    canvas.lineTo(vert[2].s0, vert[2].s1);
    canvas.closePath();
}

void drawPolygonPath(ciallo::CrCanvas& canvas, int32_t vCount, const struct nk_vec2i *v)
{
    canvas.moveTo(v[0].x, v[0].y);
    for (int32_t i = 1; i < vCount; i++)
        canvas.lineTo(v[i].x, v[i].y);
    canvas.closePath();
}

void drawPolylinePath(ciallo::CrCanvas& canvas, int32_t vCount, const struct nk_vec2i *v)
{
    canvas.moveTo(v[0].x, v[0].y);
    for (int32_t i = 1; i < vCount; i++)
        canvas.lineTo(v[i].x, v[i].y);
}

} // namespace anonymous
void Draw::render()
{
    fCanvas.save();
    BeforeLeaveScope leave([this]() -> void {
        fCanvas.restore();
    });

    /**
     * The Ciallo 2D engine (Cairo2d backend) always send drawing
     * operations to the display server like X11. The problem
     * is that we don't know when the display server will update
     * screen and we don't have a double-buffered architecture.
     * That might cause awful flickering on the window.
     * To avoid this, we push all of the drawing operations to
     * a group, and pop them to a pattern after the drawing is finished.
     * Then this pattern will be painted at once as source.
     * Finally, we force the display server refresh its
     * queue (CrSurface::flush()) and perform all the drawing operations.
     *
     * See also: https://www.cairographics.org/Xlib/
     */
    fCanvas.pushGroup();

    fCanvas.setSource(1, 1, 1);
    fCanvas.drawPaint();
    fCanvas.setAntialias(CAIRO_ANTIALIAS_BEST);

    for (const nk_command *cmd = nk__begin(fpCtx);
         cmd != nullptr;
         cmd = nk__next(fpCtx, cmd))
    {
        switch (cmd->type)
        {
        case NK_COMMAND_NOP:
            break;

        case NK_COMMAND_SCISSOR:
        {
            EXTRA_CMD(scissor)
            fCanvas.resetClip();
            fCanvas.drawRect(scissor->x, scissor->y,
                             scissor->w, scissor->h);
            fCanvas.clip();
        }
        break;

        case NK_COMMAND_LINE:
        {
            EXTRA_CMD(line)
            fCanvas.setLineWidth(line->line_thickness);
            fCanvas.setSource(FLT_COLOR(line->color));
            fCanvas.moveTo(line->begin.x, line->begin.y);
            fCanvas.lineTo(line->end.x, line->end.y);
            fCanvas.drawStroke();
        }
        break;

        case NK_COMMAND_CURVE:
        {
            EXTRA_CMD(curve)
            fCanvas.setLineWidth(curve->line_thickness);
            fCanvas.setSource(FLT_COLOR(curve->color));
            fCanvas.moveTo(curve->begin.x, curve->begin.y);
            fCanvas.curveTo(curve->ctrl[0].x, curve->ctrl[0].y,
                            curve->ctrl[1].x, curve->ctrl[1].y,
                            curve->end.x, curve->end.y);
            fCanvas.drawStroke();
        }
        break;

        case NK_COMMAND_RECT:
        {
            EXTRA_CMD(rect)

            constexpr double deg = M_PI / 180.0;
            double R = rect->rounding;
            double x = rect->x, y = rect->y, w = rect->w, h = rect->h;
            fCanvas.newSubPath();
            fCanvas.drawArc(x + w - R, y + R, R, -90 * deg, 0);
            fCanvas.drawArc(x + w - R, y + h - R, R, 0, 90 * deg);
            fCanvas.drawArc(x + R, y + h - R, R, 90 * deg, 180 * deg);
            fCanvas.drawArc(x + R, y + R, R, 180 * deg, 270 * deg);
            fCanvas.closePath();

            fCanvas.setSource(FLT_COLOR(rect->color));
            fCanvas.setLineWidth(rect->line_thickness);
            fCanvas.drawStroke();
        }
        break;

        case NK_COMMAND_RECT_FILLED:
        {
            EXTRA_CMD(rect_filled)
            constexpr double deg = M_PI / 180.0;
            double R = rect_filled->rounding;
            double x = rect_filled->x, y = rect_filled->y;
            double w = rect_filled->w, h = rect_filled->h;
            fCanvas.newSubPath();
            fCanvas.drawArc(x + w - R, y + R, R, -90 * deg, 0);
            fCanvas.drawArc(x + w - R, y + h - R, R, 0, 90 * deg);
            fCanvas.drawArc(x + R, y + h - R, R, 90 * deg, 180 * deg);
            fCanvas.drawArc(x + R, y + R, R, 180 * deg, 270 * deg);
            fCanvas.closePath();

            fCanvas.setSource(FLT_COLOR(rect_filled->color));
            fCanvas.drawFill();
        }
        break;

        case NK_COMMAND_RECT_MULTI_COLOR:
            std::cout << "DrawCmd: rect_multiple_color" << std::endl;
            break;

        case NK_COMMAND_CIRCLE:
        {
            EXTRA_CMD(circle)

            drawArcPathXYWH(fCanvas, circle->x, circle->y, circle->w, circle->h, 0, M_PI * 2);
            fCanvas.setSource(FLT_COLOR(circle->color));
            fCanvas.setLineWidth(circle->line_thickness);
            fCanvas.drawStroke();
        }
        break;

        case NK_COMMAND_CIRCLE_FILLED:
        {
            EXTRA_CMD(circle_filled)

            drawArcPathXYWH(fCanvas, circle_filled->x, circle_filled->y,
                            circle_filled->w, circle_filled->h, 0, M_PI * 2);
            fCanvas.setSource(FLT_COLOR(circle_filled->color));
            fCanvas.drawFill();
        }
        break;

        case NK_COMMAND_ARC:
        {
            EXTRA_CMD(arc)
            fCanvas.drawArc(arc->cx, arc->cy, arc->r, arc->a[0], arc->a[1]);
            fCanvas.setSource(FLT_COLOR(arc->color));
            fCanvas.setLineWidth(arc->line_thickness);
            fCanvas.drawStroke();
        }
        break;

        case NK_COMMAND_ARC_FILLED:
        {
            EXTRA_CMD(arc_filled)
            fCanvas.drawArc(arc_filled->cx, arc_filled->cy,
                            arc_filled->r, arc_filled->a[0],
                            arc_filled->a[1]);
            fCanvas.setSource(FLT_COLOR(arc_filled->color));
            fCanvas.drawFill();
        }
        break;

        case NK_COMMAND_TRIANGLE:
        {
            EXTRA_CMD(triangle)
            Vec2d vert[3] = {
                    Vec2d(triangle->a.x, triangle->a.y),
                    Vec2d(triangle->b.x, triangle->b.y),
                    Vec2d(triangle->c.x, triangle->c.y)
            };
            drawTrianglePath(fCanvas, vert);
            fCanvas.setSource(FLT_COLOR(triangle->color));
            fCanvas.setLineWidth(triangle->line_thickness);
            fCanvas.drawStroke();
        }
        break;

        case NK_COMMAND_TRIANGLE_FILLED:
        {
            EXTRA_CMD(triangle_filled)
            Vec2d vert[3] = {
                    Vec2d(triangle_filled->a.x, triangle_filled->a.y),
                    Vec2d(triangle_filled->b.x, triangle_filled->b.y),
                    Vec2d(triangle_filled->c.x, triangle_filled->c.y)
            };
            drawTrianglePath(fCanvas, vert);
            fCanvas.setSource(FLT_COLOR(triangle_filled->color));
            fCanvas.drawFill();
        }
        break;

        case NK_COMMAND_POLYGON:
        {
            EXTRA_CMD(polygon)
            drawPolygonPath(fCanvas, polygon->point_count, polygon->points);
            fCanvas.setSource(FLT_COLOR(polygon->color));
            fCanvas.setLineWidth(polygon->line_thickness);
            fCanvas.drawStroke();
        }
        break;

        case NK_COMMAND_POLYGON_FILLED:
        {
            EXTRA_CMD(polygon_filled)
            drawPolygonPath(fCanvas, polygon_filled->point_count, polygon_filled->points);
            fCanvas.setSource(FLT_COLOR(polygon_filled->color));
            fCanvas.drawFill();
        }
        break;

        case NK_COMMAND_POLYLINE:
        {
            EXTRA_CMD(polyline)
            drawPolylinePath(fCanvas, polyline->point_count, polyline->points);
            fCanvas.setSource(FLT_COLOR(polyline->color));
            fCanvas.setLineWidth(polyline->line_thickness);
            fCanvas.drawStroke();
        }
        break;

        case NK_COMMAND_TEXT: {
            EXTRA_CMD(text)
            fCanvas.drawRect(text->x, text->y, text->w, text->h);
            fCanvas.setSource(FLT_COLOR(text->background));
            fCanvas.drawFill();

            auto ft = reinterpret_cast<FontSelector*>(text->font->userdata.ptr);
            cairo_set_font_face(ft->pCairo, ft->pFace);
            fCanvas.setFontSize(ft->size);
            fCanvas.moveTo(text->x, text->y + ft->extents.ascent);
            fCanvas.setSource(FLT_COLOR(text->foreground));
            fCanvas.drawText(text->string);
        }
        break;

        case NK_COMMAND_IMAGE:
        {
            EXTRA_CMD(image)
        }
        break;

        case NK_COMMAND_CUSTOM:
            break;
        }
    }
    nk_clear(fpCtx);
    fCanvas.popGroupToSource();
    fCanvas.drawPaint();
    fSurface->flush();
}
#undef EXTRA_CMD
#undef FLT_COLOR

NAGUI_NS_END
