#ifndef COCOA_VAXCBDISPLAY_H
#define COCOA_VAXCBDISPLAY_H

#include <xcb/xcb.h>

#include "Vanilla/Base.h"
#include "Vanilla/VaDisplay.h"
#include "Vanilla/Xcb/VaXcbAtoms.h"
#include "Vanilla/Xcb/VaXcbEventQueue.h"
#include "Vanilla/Xcb/VaXcbKeyboard.h"
VANILLA_NS_BEGIN

class Context;
class VaXcbWindow;

class VaXcbDisplay : public VaDisplay,
                     public std::enable_shared_from_this<VaXcbDisplay>
{
public:
    VaXcbDisplay(const Handle<Context>& context,
                 xcb_connection_t *pConnection,
                 xcb_screen_t *pScreen,
                 xcb_visualtype_t *pVisual,
                 SkColorType format);
    ~VaXcbDisplay() override;

    va_nodiscard inline xcb_connection_t *connection()
    { return fConnection; }
    va_nodiscard inline xcb_screen_t *screen()
    { return fScreen; }
    va_nodiscard inline xcb_visualtype_t *visual()
    { return fVisual; }
    va_nodiscard inline xcb_atom_t atom(VaXcbAtoms::AtomType type)
    { return fAtoms.get(type); }

    va_nodiscard inline VaXcbKeyboard& keyboard()
    { return fKeyboard; }

    int32_t width() override;
    int32_t height() override;
    void flush() override;
    void dispose() override;

    void handleEvent(const xcb_generic_event_t *event);
    VaKeyboardProxy *keyboardProxy() override;

private:
    void selectXInputEventForWindow(xcb_window_t window);
    void handleXInputEvent(const xcb_ge_event_t *event);
    Handle<VaXcbWindow> matchWindow(xcb_window_t window);

    void onDispose();
    Handle<VaWindow> onCreateWindow(VaVec2f size, VaVec2f pos, Handle<VaWindow> parent) override;

    xcb_connection_t            *fConnection;
    xcb_screen_t                *fScreen;
    xcb_visualtype_t            *fVisual;
    SkColorType                  fFormat;
    VaXcbAtoms                   fAtoms;
    VaXcbEventQueue              fEventQueue;
    VaXcbKeyboard                fKeyboard;
    bool                         fDisposed;
};

VANILLA_NS_END
#endif //COCOA_VAXCBDISPLAY_H
