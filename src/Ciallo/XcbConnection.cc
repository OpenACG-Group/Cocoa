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
                         << " (" << pRequest << "), Sequence " << err->sequence << ", "
                         << "Error " << I(err->error_code) << " (" << pError << ")" << log_endl;
#undef I
}

XcbWindowEventListener *XcbConnection::selectListener(xcb_window_t window)
{
    if (!fWindowEventListeners.contains(window))
    {
        log_write(LOG_ERROR) << "XcbConnection: The X11 server reported an event for a window, "
                             << "but we could not find any event listeners registered for this window."
                             << log_endl;
        return nullptr;
    }

    return fWindowEventListeners[window];
}

#define SELECT_LISTENER                                            \
    XcbWindowEventListener *listener = selectListener(ev->window); \
    if (listener == nullptr)                                       \
        return;

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

#undef SELECT_LISTENER

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

    default:
        break;
    }

    xcb_flush(fConnection);
}

#undef EV_CAST
#undef ERR_CAST

CIALLO_END_NS
