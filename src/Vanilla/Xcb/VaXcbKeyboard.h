#ifndef COCOA_VAXCBKEYBOARD_H
#define COCOA_VAXCBKEYBOARD_H

#include <xkbcommon/xkbcommon.h>

#include "Vanilla/Base.h"
#include "Vanilla/VaKeyboardProxy.h"
VANILLA_NS_BEGIN

class VaXcbDisplay;
class VaXcbKeyboard
{
public:
    explicit VaXcbKeyboard(VaXcbDisplay *display);
    ~VaXcbKeyboard();

    va_nodiscard inline VaKeyboardProxy *proxy()
    { return fProxy.get(); }

    va_nodiscard inline uint32_t xkbResponseType() const
    { return fResponseType; }

    void handleXkbEvent(const xcb_generic_event_t *event);
    KeySymbol symbol(xcb_keycode_t code);

private:
    VaXcbDisplay                    *fDisplay;
    xkb_context                     *fXkbContext;
    xkb_keymap                      *fXkbKeymap;
    xkb_state                       *fXkbState;
    UniqueHandle<VaKeyboardProxy>    fProxy;
    uint32_t                         fResponseType;
    uint32_t                         fCoreDeviceId;
};

VANILLA_NS_END
#endif //COCOA_VAXCBKEYBOARD_H
