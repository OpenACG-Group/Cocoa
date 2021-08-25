#ifndef COCOA_IMGUIWINDOW_H
#define COCOA_IMGUIWINDOW_H

#include "include/core/SkPaint.h"

#include "Vanilla/Display.h"
#include "Vanilla/Window.h"
#include "Vanilla/DrawContext.h"

struct ImGuiContext;
VANILLA_NS_BEGIN

class ImGuiWindow
{
public:
    explicit ImGuiWindow(Handle<DrawContext> drawCtx);
    ~ImGuiWindow();

    VA_SIG_GETTER(Paint)

private:
    void mouseMoveEvent(vec::float2 pos);
    void mouseButtonEvent(int idx, bool state);
    void mouseWheelEvent(float delta);

    va_slot void onRepaint(const Handle<Window>& win, const SkRect& rect);
    va_slot void onClose(const Handle<Window>& win);
    va_slot void onButtonPress(const Handle<Window>& win, Button button, vec::float2 pos);
    va_slot void onButtonRelease(const Handle<Window>& win, Button button, vec::float2 pos);
    va_slot void onMotion(const Handle<Window>& win, vec::float2 pos);
    va_slot void onKeyPress(const Handle<Window>& win, KeySymbol key,
                            Bitfield<KeyModifier> modifiers, Bitfield<KeyLed> leds);
    va_slot void onKeyRelease(const Handle<Window>& win, KeySymbol key,
                              Bitfield<KeyModifier> modifiers, Bitfield<KeyLed> leds);

    VA_SLOT_FIELDS(VA_SLOT_SIGNATURE(Window, Repaint)
                   VA_SLOT_SIGNATURE(Window, Close)
                   VA_SLOT_SIGNATURE(Window, ButtonPress)
                   VA_SLOT_SIGNATURE(Window, ButtonRelease)
                   VA_SLOT_SIGNATURE(Window, Motion)
                   VA_SLOT_SIGNATURE(Window, KeyPress)
                   VA_SLOT_SIGNATURE(Window, KeyRelease))

    VA_SIG_FIELDS(VA_SIG_SIGNATURE(void(), Paint))

    Handle<DrawContext>     fDrawCtx;
    Handle<Window>          fWindow;
    SkPaint                 fFontPaint;
    ImGuiContext           *fContext;
};

VANILLA_NS_END
#endif //COCOA_IMGUIWINDOW_H
