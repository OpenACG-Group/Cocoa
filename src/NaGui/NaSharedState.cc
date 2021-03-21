#include "NaGui/NaSharedState.h"
NAGUI_NS_BEGIN

NaSharedState::NaSharedState()
    : windowTitle("NaGui::Window"),
      windowResizable(true),
      fontFamily("Cantarell"),
      fontSlant(FontSlant::kSlant_Normal),
      fontWeight(FontWeight::kWeight_Normal),
      fontSize(13),
      fontAntialias(true),
      mouseX(-1),
      mouseY(-1),
      mouseLeftButton(false),
      mouseRightButton(false),
      mouseMiddleButton(false)
{
}

NAGUI_NS_END
