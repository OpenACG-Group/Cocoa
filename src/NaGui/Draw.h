#ifndef COCOA_DRAW_H
#define COCOA_DRAW_H

#include <string>

#include "Ciallo/Cairo2d/CrSurface.h"
#include "Ciallo/Cairo2d/CrCanvas.h"
#include "NaGui/Base.h"
#include "NaGui/Math.h"

struct nk_user_font;
struct nk_context;

NAGUI_NS_BEGIN

struct DrawContext;
struct InputState;

class Draw
{
public:
    explicit Draw(ciallo::CrSurface::Ptr surface);
    ~Draw();

    int32_t addTypeface(const std::string& family, cairo_font_slant_t slant,
                        cairo_font_weight_t weight, double size);

    void test();

    void render();

    void _privBeginInput();
    void _privEndInput();
    void _privMouseLeftButton(bool down, const Vec2i& pos);
    void _privMouseRightButton(bool down, const Vec2i& pos);
    void _privMouseMiddleButton(bool down, const Vec2i& pos);
    void _privMouseMotion(const Vec2i& pos);

private:
    ciallo::CrSurface::Ptr       fSurface;
    ciallo::CrCanvas             fCanvas;
    nk_context                  *fpCtx;
    std::vector<nk_user_font*>   fTypefaces;
};

NAGUI_NS_END
#endif //COCOA_DRAW_H
