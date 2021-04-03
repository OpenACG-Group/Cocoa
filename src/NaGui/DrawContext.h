#ifndef COCOA_DRAWCONTEXT_H
#define COCOA_DRAWCONTEXT_H

#include <string>
#include <stack>

#include "NaGui/Base.h"
#include "NaGui/Math.h"

NAGUI_NS_BEGIN

struct InputState
{
    Vec2i   mousePos{0, 0};
    bool    mouseLeftDown = false;
    bool    mouseMiddleDown = false;
    bool    mouseRightDown = false;
};
using String = std::string;

struct DrawContext
{
    int32_t         width;
    int32_t         height;
};

NAGUI_NS_END
#endif //COCOA_DRAWCONTEXT_H
