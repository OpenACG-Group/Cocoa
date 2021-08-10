#ifndef COCOA_XCBKEYBOARD_H
#define COCOA_XCBKEYBOARD_H

#include <xkbcommon/xkbcommon.h>

#include "Vanilla/Base.h"
#include "Vanilla/KeyboardProxy.h"
VANILLA_NS_BEGIN

class XcbDisplay;
class XcbKeyboard
{
public:
    explicit XcbKeyboard(XcbDisplay *display);
    ~XcbKeyboard();

    va_nodiscard inline KeyboardProxy *proxy()
    { return fProxy.get(); }

    va_nodiscard inline uint32_t xkbResponseType() const
    { return fResponseType; }

    void handleXkbEvent(const xcb_generic_event_t *event);
    KeySymbol symbol(xcb_keycode_t code);

private:
    XcbDisplay                    *fDisplay;
    xkb_context                     *fXkbContext;
    xkb_keymap                      *fXkbKeymap;
    xkb_state                       *fXkbState;
    UniqueHandle<KeyboardProxy>    fProxy;
    uint32_t                         fResponseType;
    uint32_t                         fCoreDeviceId;
};

VANILLA_NS_END
#endif //COCOA_XCBKEYBOARD_H
