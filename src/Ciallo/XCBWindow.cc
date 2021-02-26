#include <pthread.h>
#include <xcb/xcb.h>

#include "Core/PropertyTree.h"
#include "Core/Journal.h"
#include "Ciallo/DDR/GrXcbPlatform.h"
#include "Ciallo/GraphicsContext.h"
#include "Ciallo/XCBWindow.h"
CIALLO_BEGIN_NS

namespace {

void PopulatePlatformOptions(GrPlatformOptions& opts)
{
    opts.vulkan_debug = PropertyTree::Instance()->asNode("/runtime/features/enableGpuDebugJournal")
                            ->cast<PropertyTreeDataNode>()->extract<bool>();
    opts.use_strict_accel = PropertyTree::Instance()->asNode("/runtime/features/useStrictHardwareDraw")
                                ->cast<PropertyTreeDataNode>()->extract<bool>();
    opts.use_gpu_accel = PropertyTree::Instance()->asNode("/runtime/features/useGpuDraw")
                                ->cast<PropertyTreeDataNode>()->extract<bool>();
    opts.use_opencl_accel = PropertyTree::Instance()->asNode("/runtime/features/useOpenCl")
                                ->cast<PropertyTreeDataNode>()->extract<bool>();

    /* TODO: Support OpenCL platform and device keyword */
}

} // namespace anonymous

#define TO_CHECKED(e)                                   \
    if (e != nullptr) {                                 \
        throw RuntimeException::Builder(__FUNCTION__)   \
            .append("XCB request failed, error ")       \
            .append(e->error_code)                      \
            .make<RuntimeException>();                  \
    }

XCBWindow::XCBWindow(const char *pDisplayName, int32_t width, int32_t height)
    : BaseWindow("XCB", width, height),
      fIsClosed(false),
      fConnection(nullptr),
      fSetup(nullptr),
      fScreen(nullptr),
      fScreenNum(0),
      fWindow(XCB_NONE)
{
    /* The destructor will not be called if an exception is thrown by constructor.
       We need to cleanup manually */
    BeforeLeaveScope before([&]() -> void {
        if (fConnection != nullptr)
            xcb_disconnect(fConnection);
    });

    fConnection = xcb_connect(pDisplayName, &fScreenNum);
    if (fConnection == nullptr)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Couldn\'t connect to X11 server")
                .make<RuntimeException>();
    }

    fSetup = xcb_get_setup(fConnection);
    if (fSetup == nullptr)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Couldn\'t get setup information from connection")
                .make<RuntimeException>();
    }

    xcb_screen_iterator_t screenIterator = xcb_setup_roots_iterator(fSetup);
    for (int32_t i = fScreenNum; i > 0; i--)
        xcb_screen_next(&screenIterator);

    fScreen = screenIterator.data;
    before.cancel();
}

XCBWindow::~XCBWindow()
{
    /* Destroy GraphicsContext first to suppress any draw operations. */
    destroyGContext();

    /* Then close window */
    if (fWindow != XCB_NONE)
        xcb_destroy_window(fConnection, fWindow);

    /* The connection thread should exit after the window is closed */
    if (fConnectionThread.joinable())
        fConnectionThread.join();

    /* Finally, destroy other resources */
    if (fConnection != nullptr)
        xcb_disconnect(fConnection);
}

void XCBWindow::connectionThread()
{
#if defined(__linux__)
    pthread_setname_np(pthread_self(), "XCBConnection");
#endif

    xcb_generic_event_t *ev;
    EventResponse resp;
    while ((ev = xcb_wait_for_event(fConnection)))
    {
        resp = threadHandleEvent(ev);
        std::free(ev);

        if (resp == EventResponse::kThreadExit)
            break;
    }

    if (resp != EventResponse::kThreadExit)
        log_write(LOG_WARNING) << "XCB event loop exits without WM\'s notify" << log_endl;
    fIsClosed = true;
}

XCBWindow::EventResponse XCBWindow::threadHandleEvent(xcb_generic_event_t *ev)
{
    switch (ev->response_type & ~0x80)
    {
    case XCB_EXPOSE:
        return threadHandleExpose(reinterpret_cast<xcb_expose_event_t*>(ev));

    case XCB_CLIENT_MESSAGE:
        return threadHandleClientMessage(reinterpret_cast<xcb_client_message_event_t*>(ev));

    case XCB_NO_EXPOSURE:
        return EventResponse::kNormal;
    }

    /* TODO: Handle more events */
    log_write(LOG_DEBUG) << "Uncaught XCB event " << ev
                         << " (" << (ev->response_type & ~0x80) << ")" << log_endl;
    return EventResponse::kNormal;
}

XCBWindow::EventResponse XCBWindow::threadHandleExpose(xcb_expose_event_t *ev)
{
    if (GContext() != nullptr)
    {
        /* Here we must wait until the composition is done. */
        GContext()->emitCmdPresent()->wait();
    }
    return EventResponse::kNormal;
}

XCBWindow::EventResponse XCBWindow::threadHandleClientMessage(xcb_client_message_event_t *ev)
{
    if (ev->data.data32[0] == fWmDeleteWindowAtom)
        return EventResponse::kThreadExit;

    return EventResponse::kNormal;
}

bool XCBWindow::onIsClosed()
{
    return fIsClosed;
}

void XCBWindow::onCreateWindow()
{
    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t maskValues[2] = {
            fScreen->white_pixel,
            XCB_EVENT_MASK_EXPOSURE |
            XCB_EVENT_MASK_POINTER_MOTION |
            XCB_EVENT_MASK_BUTTON_PRESS |
            XCB_EVENT_MASK_BUTTON_RELEASE |
            XCB_EVENT_MASK_BUTTON_MOTION |
            XCB_EVENT_MASK_KEY_PRESS |
            XCB_EVENT_MASK_KEY_RELEASE
    };

    fWindow = xcb_generate_id(fConnection);
    xcb_create_window(fConnection,
                      XCB_COPY_FROM_PARENT,
                      fWindow,
                      fScreen->root,
                      0, 0,
                      this->width(), this->height(),
                      10,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      fScreen->root_visual,
                      mask,
                      maskValues);

    xcb_map_window(fConnection, fWindow);
    xcb_flush(fConnection);

    {
        xcb_generic_error_t *pError = nullptr;
        auto deleteWindowCookie = xcb_intern_atom(fConnection, 0, 16,
                                                  "WM_DELETE_WINDOW");
        auto protocolCookie = xcb_intern_atom(fConnection, 1, 12,
                                              "WM_PROTOCOLS");

        auto *pDeleteWindowReply = xcb_intern_atom_reply(fConnection, deleteWindowCookie, &pError);
        TO_CHECKED(pError)

        auto *pWmProtocolReply = xcb_intern_atom_reply(fConnection, protocolCookie, &pError);
        TO_CHECKED(pError)

        fWmDeleteWindowAtom = pDeleteWindowReply->atom;
        fWmProtocolAtom = pWmProtocolReply->atom;

        std::free(pDeleteWindowReply);
        std::free(pWmProtocolReply);
    }

    xcb_change_property_checked(fConnection,
                                XCB_PROP_MODE_REPLACE,
                                fWindow,
                                fWmProtocolAtom,
                                XCB_ATOM_ATOM,
                                32, 1,
                                &fWmDeleteWindowAtom);

    /* Now, an XCB window is created and configured.
       Start connection thread and create GraphicsContext then. */

    GrPlatformOptions opts;
    PopulatePlatformOptions(opts);

    createGContext(GrXcbPlatform::MakeFromXcbWindow(fConnection,
                                                    fWindow,
                                                    fScreenNum,
                                                    opts));
    fConnectionThread = std::thread(&XCBWindow::connectionThread, this);
}

void XCBWindow::onWindowExpose()
{
    xcb_expose_event_t event {
        .response_type = XCB_EXPOSE,
        .sequence = 0,
        .window = fWindow,
        .x = 0,
        .y = 0,
        .width = static_cast<uint16_t>(this->width()),
        .height = static_cast<uint16_t>(this->height()),
        .count = 1
    };

    xcb_send_event(fConnection,
                   false,
                   fWindow,
                   XCB_EVENT_MASK_EXPOSURE,
                   reinterpret_cast<const char*>(&event));
    xcb_flush(fConnection);
}

void XCBWindow::onSetWindowTitle(const std::string& name)
{
    xcb_change_property(fConnection,
                        XCB_PROP_MODE_REPLACE,
                        fWindow,
                        XCB_ATOM_WM_NAME,
                        XCB_ATOM_STRING,
                        8,
                        name.length(),
                        name.c_str());
    xcb_flush(fConnection);
}

CIALLO_END_NS
