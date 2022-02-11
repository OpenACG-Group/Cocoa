#ifndef COCOA_XCBDISPLAY_H
#define COCOA_XCBDISPLAY_H

#include <xcb/xcb.h>

#include "Vanilla/Base.h"
#include "Vanilla/Display.h"
#include "Vanilla/Xcb/XcbAtoms.h"
#include "Vanilla/Xcb/XcbEventQueue.h"
#include "Vanilla/Xcb/XcbKeyboard.h"
VANILLA_NS_BEGIN

class Context;
class XcbWindow;

class XcbDisplay : public Display,
                   public std::enable_shared_from_this<XcbDisplay>
{
public:
    XcbDisplay(const Handle<Context>& context,
                 xcb_connection_t *pConnection,
                 xcb_screen_t *pScreen,
                 xcb_visualtype_t *pVisual,
                 SkColorType format);
    ~XcbDisplay() override;

    va_nodiscard inline xcb_connection_t *connection()
    { return fConnection; }
    va_nodiscard inline xcb_screen_t *screen()
    { return fScreen; }
    va_nodiscard inline xcb_visualtype_t *visual()
    { return fVisual; }
    va_nodiscard inline xcb_atom_t atom(XcbAtoms::AtomType type)
    { return fAtoms.get(type); }

    va_nodiscard inline XcbKeyboard& keyboard()
    { return fKeyboard; }

    int32_t width() override;
    int32_t height() override;
    void flush() override;
    void dispose() override;

    void handleEvent(const xcb_generic_event_t *event);
    KeyboardProxy *keyboardProxy() override;

private:
    void selectXInputEventForWindow(xcb_window_t window);
    void handleXInputEvent(const xcb_ge_event_t *event);
    Handle<XcbWindow> matchWindow(xcb_window_t window);

    void onDispose();
    Handle<Window> onCreateWindow(vec::float2 size, vec::float2 pos, Handle<Window> parent) override;

    xcb_connection_t            *fConnection;
    xcb_screen_t                *fScreen;
    xcb_visualtype_t            *fVisual;
    SkColorType                  fFormat;
    XcbAtoms                   fAtoms;
    XcbEventQueue              fEventQueue;
    XcbKeyboard                fKeyboard;
    bool                         fDisposed;
};

VANILLA_NS_END
#endif //COCOA_XCBDISPLAY_H
