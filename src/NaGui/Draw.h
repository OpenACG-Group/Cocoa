#ifndef COCOA_NADRAW_H
#define COCOA_NADRAW_H

#include "NaGui/NaGuiNamespace.h"
#include "NaGui/DrawContext.h"
NAGUI_NS_BEGIN

class Draw
{
public:
    Draw(DrawContext& ctx, InputState& input);
    ~Draw();

    /**
     * @brief Playback the recorded drawing operations on a specified
     *        canvas.
     * @param canvas A Cairo canvas which will be the destination.
     */
    void playback(ciallo::CrCanvas& canvas);

private:
    DrawContext&            fCtx;
    InputState&             fInput;
    ciallo::CrSurface::Ptr  fSurface;
    ciallo::CrCanvas        fCanvas;
};

NAGUI_NS_END
#endif //COCOA_NADRAW_H
