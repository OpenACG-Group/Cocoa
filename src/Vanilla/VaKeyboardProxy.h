#ifndef COCOA_VAKEYBOARDPROXY_H
#define COCOA_VAKEYBOARDPROXY_H

#include "Vanilla/Base.h"
#include "Vanilla/VaKeySymbols.h"
#include "Vanilla/Typetraits.h"

struct xkb_context;
struct xkb_keymap;
struct xkb_state;

VANILLA_NS_BEGIN

class VaKeyboardProxy
{
public:
    VaKeyboardProxy(xkb_context *pCtx, xkb_keymap *pKeymap, xkb_state *pState);
    ~VaKeyboardProxy();


    Bitfield<KeyModifier> activeMods();
    Bitfield<KeyLed> activeLeds();

private:
    xkb_context         *fXkbContext;
    xkb_keymap          *fXkbKeymap;
    xkb_state           *fXkbState;
};

VANILLA_NS_END
#endif //COCOA_VAKEYBOARDPROXY_H
