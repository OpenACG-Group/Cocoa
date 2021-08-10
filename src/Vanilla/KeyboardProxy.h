#ifndef COCOA_KEYBOARDPROXY_H
#define COCOA_KEYBOARDPROXY_H

#include "Vanilla/Base.h"
#include "Vanilla/KeySymbols.h"
#include "Vanilla/Typetraits.h"

struct xkb_context;
struct xkb_keymap;
struct xkb_state;

VANILLA_NS_BEGIN

class KeyboardProxy
{
public:
    KeyboardProxy(xkb_context *pCtx, xkb_keymap *pKeymap, xkb_state *pState);
    ~KeyboardProxy();


    Bitfield<KeyModifier> activeMods();
    Bitfield<KeyLed> activeLeds();

private:
    xkb_context         *fXkbContext;
    va_maybe_unused
    xkb_keymap          *fXkbKeymap;
    xkb_state           *fXkbState;
};

VANILLA_NS_END
#endif //COCOA_KEYBOARDPROXY_H
