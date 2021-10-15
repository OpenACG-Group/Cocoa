#include "Vanilla/RenderKit/InputEventListener.h"
#include "Vanilla/Window.h"
VANILLA_NS_BEGIN

InputEventListener::InputEventListener(const Handle<Window>& win)
{
    if (win)
    {
        connectListenerSlots(win);
    }
}

InputEventListener::~InputEventListener()
{
    va_slot_disconnect(Window, Map);
    va_slot_disconnect(Window, Unmap);
    va_slot_disconnect(Window, Configure);
    va_slot_disconnect(Window, Repaint);
    va_slot_disconnect(Window, Close);
    va_slot_disconnect(Window, ButtonPress);
    va_slot_disconnect(Window, ButtonRelease);
    va_slot_disconnect(Window, Motion);
    va_slot_disconnect(Window, TouchBegin);
    va_slot_disconnect(Window, TouchUpdate);
    va_slot_disconnect(Window, TouchEnd);
    va_slot_disconnect(Window, KeyPress);
    va_slot_disconnect(Window, KeyRelease);
}

void InputEventListener::connectListenerSlots(const Handle<Window>& win)
{
#define M(f)    sigc::mem_fun(*this, &InputEventListener::f)

    va_slot_connect(Window, Map, win, M(onSlotMap));
    va_slot_connect(Window, Unmap, win, M(onSlotUnmap));
    va_slot_connect(Window, Configure, win, M(onSlotConfigure));
    va_slot_connect(Window, Repaint, win, M(onSlotRepaint));
    va_slot_connect(Window, Close, win, M(onSlotClose));
    va_slot_connect(Window, ButtonPress, win, M(onSlotButtonPress));
    va_slot_connect(Window, ButtonRelease, win, M(onSlotButtonRelease));
    va_slot_connect(Window, Motion, win, M(onSlotMotion));
    va_slot_connect(Window, TouchBegin, win, M(onSlotTouchBegin));
    va_slot_connect(Window, TouchUpdate, win, M(onSlotTouchUpdate));
    va_slot_connect(Window, TouchEnd, win, M(onSlotTouchEnd));
    va_slot_connect(Window, KeyPress, win, M(onSlotKeyPress));
    va_slot_connect(Window, KeyRelease, win, M(onSlotKeyRelease));

#undef M
}

void InputEventListener::onSlotMap(const Handle<Window>& win)
{
    onMap();
}

void InputEventListener::onSlotUnmap(const Handle<Window>& win)
{
    onUnmap();
}

void InputEventListener::onSlotConfigure(const Handle<Window>& win, const SkRect& rect)
{
    onResizeOrDrag(rect);
}

void InputEventListener::onSlotRepaint(const Handle<Window>& win, const SkRect& rect)
{
    onRepaint(rect);
}

void InputEventListener::onSlotButtonPress(const Handle<Window>& win, Button button, vec::float2 pos)
{
    onButtonPress(button, pos);
}

void InputEventListener::onSlotClose(const Handle<Window>& win)
{
    onClose();
}

void InputEventListener::onSlotButtonRelease(const Handle<Window>& win, Button button, vec::float2 pos)
{
    onButtonRelease(button, pos);
}

void InputEventListener::onSlotMotion(const Handle<Window>& win, vec::float2 pos)
{
    onMotion(pos);
}

void InputEventListener::onSlotTouchUpdate(const Handle<Window>& win, vec::float2 pos)
{
    onTouchUpdate(pos);
}

void InputEventListener::onSlotTouchBegin(const Handle<Window>& win, vec::float2 pos)
{
    onTouchBegin(pos);
}

void InputEventListener::onSlotTouchEnd(const Handle<Window>& win, vec::float2 pos)
{
    onTouchEnd(pos);
}

void InputEventListener::onSlotKeyPress(const Handle<Window>& win,
                                        KeySymbol key,
                                        Bitfield<KeyModifier> modifiers,
                                        va_maybe_unused Bitfield<KeyLed> leds)
{
    onKeyPress(key, modifiers);
}

void InputEventListener::onSlotKeyRelease(const Handle<Window>& win,
                                          KeySymbol key,
                                          Bitfield<KeyModifier> modifiers,
                                          va_maybe_unused Bitfield<KeyLed> leds)
{
    onKeyRelease(key, modifiers);
}

VANILLA_NS_END
