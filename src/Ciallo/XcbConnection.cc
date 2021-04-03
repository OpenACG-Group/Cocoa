#include <xcb/xcb.h>
#include <xcb/xcb_event.h>
#include <xcb/xcb_util.h>

#include "Core/Journal.h"
#include "Ciallo/Ciallo.h"
#include "Ciallo/XcbScreen.h"
#include "Ciallo/XcbConnection.h"
#include "Ciallo/XcbEventQueue.h"
CIALLO_BEGIN_NS

XcbConnection::XcbConnection(const char *pDisplay)
    : fConnection(nullptr),
      fSetup(nullptr),
      fScreen(nullptr),
      fEventQueue(nullptr)
{
    int screenNum;
    fConnection = xcb_connect(pDisplay, &screenNum);
    if (fConnection == nullptr || xcb_connection_has_error(fConnection))
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Couldn\'t connect to X11 server")
                .make<RuntimeException>();
    }

    BeforeLeaveScope beforeLeaveScope([this]() -> void {
        if (fConnection)
            xcb_disconnect(fConnection);
    });

    requireExtensions();

    fAtom.initialize(fConnection);
    fSetup = xcb_get_setup(fConnection);

    /* TODO: Implement a complete method to find a correct screen */
    xcb_screen_iterator_t screenIterator = xcb_setup_roots_iterator(fSetup);
    for (int32_t i = screenNum; i > 0; i--)
        xcb_screen_next(&screenIterator);

    fScreen = new XcbScreen(this, screenIterator.data);
    fEventQueue = new XcbEventQueue(this);
    beforeLeaveScope.cancel();
}

XcbConnection::~XcbConnection()
{
    delete fEventQueue;
    delete fScreen;
    xcb_disconnect(fConnection);
}

void XcbConnection::requireExtensions()
{
    const xcb_query_extension_reply_t *reply = xcb_get_extension_data(fConnection,
                                                                      &xcb_input_id);
    if (!reply || !reply->present)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Require XInput extension for X11 platform")
                .make<RuntimeException>();
    }

    auto xinputVer = xcb_input_xi_query_version_reply(
            fConnection,
            xcb_input_xi_query_version(fConnection, 2, 2),
            nullptr);

    BeforeLeaveScope before([xinputVer]() -> void {
        if (xinputVer)
            std::free(xinputVer);
    });

    if (xinputVer == nullptr || xinputVer->major_version < 2)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                    .append("X server doesn't support XInput 2")
                    .make<RuntimeException>();
    }

    log_write(LOG_INFO) << "X11: XInput version " << xinputVer->major_version
                        << '.' << xinputVer->minor_version << log_endl;
}

const xcb_setup_t * XcbConnection::setup() const
{
    return fSetup;
}

XcbScreen *XcbConnection::screen()
{
    return fScreen;
}

XcbEventQueue *XcbConnection::queue()
{
    return fEventQueue;
}

void XcbConnection::disconnect()
{
    fEventQueue->sendCloseConnection();
}

xcb_atom_t XcbConnection::atom(XcbAtom::Atom atom)
{
    return fAtom.atom(atom);
}

void XcbConnection::addWindowEventListener(xcb_window_t window, XcbWindowEventListener *listener)
{
    if (window == XCB_NONE || listener == nullptr)
        return;
    fWindowEventListeners[window] = listener;
}

void XcbConnection::removeWindowEventListener(XcbWindowEventListener *listener)
{
    xcb_window_t window = XCB_NONE;
    for (auto& pair : fWindowEventListeners)
    {
        if (pair.second == listener)
        {
            window = pair.first;
            break;
        }
    }

    if (window != XCB_NONE)
        fWindowEventListeners.erase(window);
}

XcbWindowEventListener *XcbConnection::getWindowEventListener(xcb_window_t window)
{
    if (!fWindowEventListeners.contains(window))
        return nullptr;
    return fWindowEventListeners[window];
}

const xcb_format_t *XcbConnection::formatForDepth(uint8_t depth)
{
    auto itr = xcb_setup_pixmap_formats_iterator(fSetup);
    while (itr.rem)
    {
        if (itr.data->depth == depth)
            return itr.data;
        xcb_format_next(&itr);
    }

    return nullptr;
}

xcb_connection_t *XcbConnection::nativeHandle()
{
    return fConnection;
}

void XcbConnection::processXcbError(xcb_generic_error_t *err)
{
#define I(d)    static_cast<int32_t>(d)

    const char *pError = xcb_event_get_error_label(err->error_code);
    const char *pRequest = xcb_event_get_request_label(err->major_code);

    log_write(LOG_ERROR) << "XCB Error: Request " << I(err->major_code) << ":" << I(err->minor_code)
                         << " (" << (pRequest ? pRequest : "<unknown>") << "), Sequence " << err->sequence << ", "
                         << "Error " << I(err->error_code) << " (" << (pError ? pError : "<unknown>") << ")" << log_endl;
#undef I
}

XcbWindowEventListener *XcbConnection::selectListener(xcb_window_t window)
{
    if (!fWindowEventListeners.contains(window))
        return nullptr;

    return fWindowEventListeners[window];
}

#define SELECT_LISTENER_FOR(wnd)    \
    XcbWindowEventListener *listener = selectListener(wnd); \
    if (listener == nullptr)        \
        return;

#define SELECT_LISTENER     SELECT_LISTENER_FOR(ev->window)

void XcbConnection::processExpose(xcb_expose_event_t *ev)
{
    SELECT_LISTENER
    listener->handleExposeEvent(ev);
}

void XcbConnection::processClientMessage(xcb_client_message_event_t *ev)
{
    if (ev->type == fScreen->connection()->atom(XcbAtom::Atom::CA_CLOSE_CONNECTION))
    {
        log_write(LOG_INFO) << "Disconnect from X11 server." << log_endl;
        return;
    }

    SELECT_LISTENER
    listener->handleClientMessageEvent(ev);
}

void XcbConnection::processGenericEvent(xcb_ge_event_t *ev)
{
    /* Select a listener */
    SELECT_LISTENER_FOR(reinterpret_cast<xcb_input_button_press_event_t*>(ev)->event)

    switch (ev->event_type)
    {
    case XCB_INPUT_BUTTON_PRESS:
        listener->handleXInputButtonPress(ev);
        break;

    case XCB_INPUT_BUTTON_RELEASE:
        listener->handleXInputButtonRelease(ev);
        break;

    case XCB_INPUT_MOTION:
        listener->handleXInputMotion(ev);
        break;

    case XCB_INPUT_ENTER:
        listener->handleXInputEnter(ev);
        break;

    case XCB_INPUT_LEAVE:
        listener->handleXInputLeave(ev);
        break;

    default:
        break;
    }
}

void XcbConnection::processFocusIn(xcb_focus_in_event_t *ev)
{
    SELECT_LISTENER_FOR(ev->event)
    listener->handleFocusInEvent(ev);
}

void XcbConnection::processFocusOut(xcb_focus_out_event_t *ev)
{
    SELECT_LISTENER_FOR(ev->event)
    listener->handleFocusOutEvent(ev);
}

void XcbConnection::processConfigureNotify(xcb_configure_notify_event_t *ev)
{
    SELECT_LISTENER_FOR(ev->window)
    listener->handleConfigureNotifyEvent(ev);
}

#undef SELECT_LISTENER
#undef SELECT_LISTENER_FOR

#define EV_CAST(ev) reinterpret_cast<xcb_##ev##_event_t*>(event)
#define ERR_CAST    reinterpret_cast<xcb_generic_error_t*>(event)
void XcbConnection::processEvent()
{
    xcb_generic_event_t *event = fEventQueue->take();
    if (event == nullptr)
    {
        log_write(LOG_ERROR) << "XcbConnection: We're woken up by event dispatcher, "
                             << "but nothing need to be processed" << log_endl;
        return;
    }

    BeforeLeaveScope before([event]() -> void { std::free(event); });

    auto type = event->response_type & ~0x80;
    switch (type)
    {
    case 0:
        processXcbError(ERR_CAST);
        break;

    case XCB_EXPOSE:
        processExpose(EV_CAST(expose));
        break;

    case XCB_CLIENT_MESSAGE:
        processClientMessage(EV_CAST(client_message));
        break;

    case XCB_GE_GENERIC:
        processGenericEvent(EV_CAST(ge));
        break;

    case XCB_FOCUS_IN:
        processFocusIn(EV_CAST(focus_in));
        break;

    case XCB_FOCUS_OUT:
        processFocusOut(EV_CAST(focus_out));
        break;

    case XCB_CONFIGURE_NOTIFY:
        processConfigureNotify(EV_CAST(configure_notify));
        break;

    default:
        break;
    }

    xcb_flush(fConnection);
}

#undef EV_CAST
#undef ERR_CAST

CIALLO_END_NS
