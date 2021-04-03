#ifndef COCOA_DRAWCONTEXT_H
#define COCOA_DRAWCONTEXT_H

#include <string>
#include <stack>

#include "NaGui/NaGuiNamespace.h"
#include "NaGui/Math.h"
NAGUI_NS_BEGIN

struct InputState
{
    Vec2i   mousePos{0, 0};
    bool    mouseLeftDown = false;
    bool    mouseMiddleDown = false;
    bool    mouseRightDown = false;
};

#define NAGUI_RESERVED_ID   0
using DrawId = uint64_t;
using String = std::string;

struct DrawContext
{
    enum Layout
    {
        kLayout_Horizontal,
        kLayout_Vertical
    };

    enum FontSlant
    {
        kSlant_Normal,
        kSlant_Bold,
        kSlant_Italic
    };

    enum FontWeight
    {
        kWeight_Normal,
        kWeight_Bold
    };

    Vec2i           viewportSize{0, 0};
    Vec2i           scrollPos{0, 0};
    Vec2i           viewSize{0, 0};

    bool            antialias = true;
    Layout          layout = kLayout_Horizontal;
    Vec2i           cursor{0, 0};
    DrawId          activeId = NAGUI_RESERVED_ID;
    DrawId          hoverId = NAGUI_RESERVED_ID;
    DrawId          keyboardFocusedId = NAGUI_RESERVED_ID;

    String          fontFamily = "Cantarell";
    FontSlant       fontSlant = kSlant_Normal;
    FontWeight      fontWeight = kWeight_Normal;
    double          fontSize = 13;
    bool            fontAntialias = true;
    Vec3f           fontColor{0, 0, 0};
};

NAGUI_NS_END
#endif //COCOA_DRAWCONTEXT_H
