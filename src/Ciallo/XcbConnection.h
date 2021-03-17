#ifndef COCOA_XCBCONNECTION_H
#define COCOA_XCBCONNECTION_H

#include <map>
#include <vector>

#include <xcb/xcb.h>
#include <xcb/xinput.h>

#include "Core/BaseEventProcessor.h"
#include "Ciallo/Ciallo.h"
#include "Ciallo/XcbAtom.h"
CIALLO_BEGIN_NS

class XcbScreen;
class XcbEventQueue;

class XcbWindowEventListener
{
    friend class XcbConnection;

public:
    virtual ~XcbWindowEventListener() = default;

protected:
    virtual void handleExposeEvent(const xcb_expose_event_t *) {}
    virtual void handleClientMessageEvent(const xcb_client_message_event_t *) {}
    virtual void handleConfigureNotifyEvent(const xcb_configure_notify_event_t *) {}
    virtual void handleMapNotifyEvent(const xcb_map_notify_event_t *) {}
    virtual void handleUnmapNotifyEvent(const xcb_unmap_notify_event_t *) {}
    virtual void handleDestroyNotifyEvent(const xcb_destroy_notify_event_t *) {}
    virtual void handleButtonPressEvent(const xcb_button_press_event_t *) {}
    virtual void handleButtonReleaseEvent(const xcb_button_release_event_t *) {}
    virtual void handleMotionNotifyEvent(const xcb_motion_notify_event_t *) {}
    virtual void handleEnterNotifyEvent(const xcb_enter_notify_event_t *) {}
    virtual void handleLeaveNotifyEvent(const xcb_leave_notify_event_t *) {}
    virtual void handleFocusInEvent(const xcb_focus_in_event_t *) {}
    virtual void handleFocusOutEvent(const xcb_focus_out_event_t *) {}
    virtual void handlePropertyNotifyEvent(const xcb_property_notify_event_t *) {}
    virtual void handleXIMouseEvent(xcb_ge_event_t *) {}
    virtual void handleXIEnterLeave(xcb_ge_event_t *) {}
};

class XcbConnection : public BaseEventProcessor
{
public:
    explicit XcbConnection(const char *pDisplay = nullptr);
    ~XcbConnection() override;

    const xcb_setup_t *setup() const;
    XcbScreen *screen();
    XcbEventQueue *queue();
    void disconnect();

    xcb_atom_t atom(XcbAtom::Atom atom);

    void addWindowEventListener(xcb_window_t window, XcbWindowEventListener *listener);
    void removeWindowEventListener(XcbWindowEventListener *listener);
    XcbWindowEventListener *getWindowEventListener(xcb_window_t window);

    const xcb_format_t *formatForDepth(uint8_t depth);
    xcb_connection_t *nativeHandle();

    void processEvent() override;

private:
    XcbWindowEventListener *selectListener(xcb_window_t window);
    void processExpose(xcb_expose_event_t *ev);
    void processClientMessage(xcb_client_message_event_t *ev);
    void processXcbError(xcb_generic_error_t *event);

private:
    using WindowMapper = std::map<xcb_window_t, XcbWindowEventListener*>;

    xcb_connection_t               *fConnection;
    const xcb_setup_t              *fSetup;
    XcbScreen                      *fScreen;
    XcbEventQueue                  *fEventQueue;
    XcbAtom                         fAtom;
    WindowMapper                    fWindowEventListeners;
};

CIALLO_END_NS
#endif //COCOA_XCBCONNECTION_H
