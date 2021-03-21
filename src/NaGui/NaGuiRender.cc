#include "NaGui/NaGui.h"
NAGUI_NS_BEGIN

void NaWindow::TextLabel(const std::string& text)
{
    fCanvas->setSource(0, 0, 0);
    fCanvas->moveTo(100, 100);
    fCanvas->drawText(text.c_str());
}

NAGUI_NS_END
