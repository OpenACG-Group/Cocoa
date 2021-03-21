#ifndef COCOA_NASHAREDSTATE_H
#define COCOA_NASHAREDSTATE_H

#include "NaGui/NaGui.h"
#include "NaGui/StatefulValue.h"
NAGUI_NS_BEGIN

class NaSharedState
{
public:
    NaSharedState();

    StatefulValueT<std::string>     windowTitle;
    StatefulValueT<bool>            windowResizable;
    StatefulValueT<std::string>     fontFamily;
    StatefulValueT<FontSlant>       fontSlant;
    StatefulValueT<FontWeight>      fontWeight;
    StatefulValueT<double>          fontSize;
    StatefulValueT<bool>            fontAntialias;
    StatefulValueT<int32_t>         mouseX;
    StatefulValueT<int32_t>         mouseY;
    StatefulValueT<bool>            mouseLeftButton;
    StatefulValueT<bool>            mouseRightButton;
    StatefulValueT<bool>            mouseMiddleButton;
};

NAGUI_NS_END
#endif //COCOA_NASHAREDSTATE_H
