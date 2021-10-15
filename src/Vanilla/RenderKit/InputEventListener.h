#ifndef COCOA_INPUTEVENTLISTENER_H
#define COCOA_INPUTEVENTLISTENER_H

#include "Vanilla/Base.h"
#include "Vanilla/KeySymbols.h"
#include "Vanilla/Typetraits.h"
VANILLA_NS_BEGIN

class Window;

/**
 * Input events:
 *  Window System => vanilla::Display =(Dispatch)=> vanilla::Window => Emitted signal
 *
 * Handler:
 *  Emitted signal => InputEventListener
 */
class InputEventListener
{
public:
    explicit InputEventListener(const Handle<Window>& window);
    virtual ~InputEventListener();

protected:
    void connectListenerSlots(const Handle<Window>& window);

    virtual void onMap() {}
    virtual void onUnmap() {}
    virtual void onResizeOrDrag(const SkRect& rect) {}
    virtual void onRepaint(const SkRect& boundary) {}
    virtual void onClose() {}
    virtual void onButtonPress(Button button, vec::float2 pos) {}
    virtual void onButtonRelease(Button button, vec::float2 pos) {}
    virtual void onMotion(vec::float2 pos) {}
    virtual void onTouchBegin(vec::float2 pos) {}
    virtual void onTouchUpdate(vec::float2 pos) {}
    virtual void onTouchEnd(vec::float2 pos) {}
    virtual void onKeyPress(KeySymbol key, Bitfield<KeyModifier> modifiers) {}
    virtual void onKeyRelease(KeySymbol key, Bitfield<KeyModifier> modifiers) {}

private:
    va_slot void onSlotMap(const Handle<Window>& win);
    va_slot void onSlotUnmap(const Handle<Window>& win);
    va_slot void onSlotConfigure(const Handle<Window>& win, const SkRect& rect);
    va_slot void onSlotRepaint(const Handle<Window>& win, const SkRect& rect);
    va_slot void onSlotClose(const Handle<Window>& win);
    va_slot void onSlotButtonPress(const Handle<Window>& win, Button button, vec::float2 pos);
    va_slot void onSlotButtonRelease(const Handle<Window>& win, Button button, vec::float2 pos);
    va_slot void onSlotMotion(const Handle<Window>& win, vec::float2 pos);
    va_slot void onSlotTouchBegin(const Handle<Window>& win, vec::float2 pos);
    va_slot void onSlotTouchUpdate(const Handle<Window>& win, vec::float2 pos);
    va_slot void onSlotTouchEnd(const Handle<Window>& win, vec::float2 pos);
    va_slot void onSlotKeyPress(const Handle<Window>& win, KeySymbol key,
                                Bitfield<KeyModifier> modifiers, Bitfield<KeyLed> leds);
    va_slot void onSlotKeyRelease(const Handle<Window>& win, KeySymbol key,
                                  Bitfield<KeyModifier> modifiers, Bitfield<KeyLed> leds);

    VA_SLOT_FIELDS(VA_SLOT_SIGNATURE(Window, Map)
                   VA_SLOT_SIGNATURE(Window, Unmap)
                   VA_SLOT_SIGNATURE(Window, Configure)
                   VA_SLOT_SIGNATURE(Window, Repaint)
                   VA_SLOT_SIGNATURE(Window, Close)
                   VA_SLOT_SIGNATURE(Window, ButtonPress)
                   VA_SLOT_SIGNATURE(Window, ButtonRelease)
                   VA_SLOT_SIGNATURE(Window, Motion)
                   VA_SLOT_SIGNATURE(Window, TouchBegin)
                   VA_SLOT_SIGNATURE(Window, TouchUpdate)
                   VA_SLOT_SIGNATURE(Window, TouchEnd)
                   VA_SLOT_SIGNATURE(Window, KeyPress)
                   VA_SLOT_SIGNATURE(Window, KeyRelease))
};

VANILLA_NS_END
#endif //COCOA_INPUTEVENTLISTENER_H
