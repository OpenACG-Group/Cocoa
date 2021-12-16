#include "Core/Errors.h"
#include <xcb/xcb.h>

#include "Core/Journal.h"
#include "Vanilla/Xcb/XcbDisplay.h"
#include "Vanilla/Xcb/XcbWindow.h"
#include "Vanilla/Xcb/EventHandlerMacros.h"
VANILLA_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Vanilla)

namespace {

xcb_screen_t *match_proper_screen(int screenNumber, const xcb_setup_t *setup)
{
    xcb_screen_iterator_t itr = xcb_setup_roots_iterator(setup);
    xcb_screen_t *screen;
    do {
        screen = itr.data;
        xcb_screen_next(&itr);
        screenNumber--;
    } while (screenNumber > 0);
    return screen;
}

xcb_visualtype_t *match_proper_visual(xcb_screen_t *screen)
{
    xcb_visualtype_t *visual = nullptr;
    for (auto itr = xcb_screen_allowed_depths_iterator(screen);
         itr.rem; xcb_depth_next(&itr))
    {
        for (auto depItr = xcb_depth_visuals_iterator(itr.data);
             depItr.rem; xcb_visualtype_next(&depItr))
        {
            if (depItr.data->visual_id == screen->root_visual)
            {
                visual = depItr.data;
                break;
            }
        }
        if (visual)
            break;
    }
    return visual;
}

} // namespace anonymous

Handle<Display> Display::OpenXcb(const Handle<Context>& ctx, const char *displayName)
{
    int screenNumber;
    xcb_connection_t *connection = xcb_connect(displayName, &screenNumber);
    if (!connection)
        return nullptr;
    ScopeEpilogue before([connection]() -> void {
        xcb_disconnect(connection);
    });

    if (xcb_connection_has_error(connection))
    {
        QLOG(LOG_ERROR, "Failed to connect to X11 server");
        return nullptr;
    }

    const xcb_setup_t *setup = xcb_get_setup(connection);
    QLOG(LOG_INFO, "Connected to X server ({}, protocol {}.{})",
                   xcb_setup_vendor(setup), setup->protocol_major_version,
                   setup->protocol_minor_version);

    xcb_screen_t *screen = match_proper_screen(screenNumber, setup);
    if (!screen)
    {
        QLOG(LOG_ERROR, "Failed to query a proper X11 screen");
        return nullptr;
    }
    QLOG(LOG_INFO, "Found proper screen {}x{} with depth {}",
                   screen->width_in_pixels, screen->height_in_pixels, screen->root_depth);

    xcb_visualtype_t *visual = match_proper_visual(screen);
    if (!visual)
    {
        QLOG(LOG_ERROR, "Failed to query a proper X11 visual type");;
        return nullptr;
    }

    if (visual->bits_per_rgb_value != 8 ||
        visual->red_mask != 0xff0000 || visual->green_mask != 0xff00 ||
        visual->blue_mask != 0xff)
    {
        QLOG(LOG_ERROR, "Unsupported color format. Only supports BGRA or BGRx now");
        return nullptr;
    }

    before.abolish();
    return std::make_shared<XcbDisplay>(ctx, connection, screen,
                                          visual, SkColorType::kBGRA_8888_SkColorType);
}

XcbDisplay::XcbDisplay(const Handle<Context>& context,
                           xcb_connection_t *pConnection,
                           xcb_screen_t *pScreen,
                           xcb_visualtype_t *pVisual,
                           SkColorType format)
    : Display(DisplayBackend::kDisplay_Xcb, context),
      fConnection(pConnection),
      fScreen(pScreen),
      fVisual(pVisual),
      fFormat(format),
      fAtoms(fConnection),
      fEventQueue(this),
      fKeyboard(this),
      fDisposed(false)
{
}

XcbDisplay::~XcbDisplay()
{
    if (!fDisposed)
        this->onDispose();
}

void XcbDisplay::dispose()
{
    if (!fDisposed)
        this->onDispose();
}

void XcbDisplay::onDispose()
{
    fDisposed = true;
    fEventQueue.disposeFromMainThread();
    xcb_disconnect(fConnection);
}

void XcbDisplay::flush()
{
    xcb_flush(fConnection);
}

int32_t XcbDisplay::width()
{
    return fScreen->width_in_pixels;
}

int32_t XcbDisplay::height()
{
    return fScreen->height_in_pixels;
}

Handle<Window> XcbDisplay::onCreateWindow(vec::float2 size, vec::float2 pos, Handle<Window> parent)
{
    xcb_window_t parentWindow = fScreen->root;
    if (parent != nullptr)
        parentWindow = std::dynamic_pointer_cast<XcbWindow>(parent)->window();

    auto x = static_cast<int16_t>(pos[0]), y = static_cast<int16_t>(pos[1]);
    auto w = static_cast<int32_t>(size[0]), h = static_cast<int32_t>(size[1]);

    constexpr uint32_t valueMask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t values[2] = {
            fScreen->black_pixel,
            XCB_EVENT_MASK_EXPOSURE
            | XCB_EVENT_MASK_STRUCTURE_NOTIFY
            | XCB_EVENT_MASK_KEY_PRESS
            | XCB_EVENT_MASK_KEY_RELEASE
    };

    xcb_window_t window = xcb_generate_id(fConnection);
    xcb_create_window(fConnection,
                      fScreen->root_depth,
                      window,
                      parentWindow,
                      x, y, w, h, 10,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      fVisual->visual_id,
                      valueMask,
                      values);
    xcb_atom_t deleteWindowAtom = fAtoms.get(XcbAtoms::WM_DELETE_WINDOW);
    xcb_change_property(fConnection,
                        XCB_PROP_MODE_REPLACE,
                        window,
                        fAtoms.get(XcbAtoms::WM_PROTOCOLS),
                        XCB_ATOM_ATOM,
                        32, 1,
                        &deleteWindowAtom);

    selectXInputEventForWindow(window);
    xcb_flush(fConnection);
    return std::make_shared<XcbWindow>(shared_from_this(),
                                         window, w, h, fFormat);
}

Handle<XcbWindow> XcbDisplay::matchWindow(xcb_window_t window)
{
    Handle<XcbWindow> result = nullptr;
    Display::forEachWindow([window, &result](Handle<Window> pWindow) -> bool {
        auto pXcbWindow = std::dynamic_pointer_cast<XcbWindow>(std::move(pWindow));
        if (window == pXcbWindow->window())
        {
            result = pXcbWindow;
            return false;
        }
        return true;
    });
    return result;
}

KeyboardProxy *XcbDisplay::keyboardProxy()
{
    return fKeyboard.proxy();
}

#define event_cast(type) \
auto type = reinterpret_cast<const xcb_##type##_event_t*>(event);

#define invoke_window_handler(type, win) \
auto window = matchWindow(win);          \
if (window)                              \
    window->VA_WIN_HANDLER_SIGNATURE(type) (type);

#define invoke_window_handler_case(_case, type, window_field) \
case _case:                                                   \
{                                                             \
    event_cast(type)                                          \
    invoke_window_handler(type, type->window_field)           \
    break;                                                    \
}

namespace {

const char *xcb_errors[] = {
        "Success",
        "BadRequest",
        "BadValue",
        "BadWindow",
        "BadPixmap",
        "BadAtom",
        "BadCursor",
        "BadFont",
        "BadMatch",
        "BadDrawable",
        "BadAccess",
        "BadAlloc",
        "BadColor",
        "BadGC",
        "BadIDChoice",
        "BadName",
        "BadLength",
        "BadImplementation",
        "Unknown"
};

const char *xcb_requests[] = {
        "Null",
        "CreateWindow",
        "ChangeWindowAttributes",
        "GetWindowAttributes",
        "DestroyWindow",
        "DestroySubwindows",
        "ChangeSaveSet",
        "ReparentWindow",
        "MapWindow",
        "MapSubwindows",
        "UnmapWindow",
        "UnmapSubwindows",
        "ConfigureWindow",
        "CirculateWindow",
        "GetGeometry",
        "QueryTree",
        "InternAtom",
        "GetAtomName",
        "ChangeProperty",
        "DeleteProperty",
        "GetProperty",
        "ListProperties",
        "SetSelectionOwner",
        "GetSelectionOwner",
        "ConvertSelection",
        "SendEvent",
        "GrabPointer",
        "UngrabPointer",
        "GrabButton",
        "UngrabButton",
        "ChangeActivePointerGrab",
        "GrabKeyboard",
        "UngrabKeyboard",
        "GrabKey",
        "UngrabKey",
        "AllowEvents",
        "GrabServer",
        "UngrabServer",
        "QueryPointer",
        "GetMotionEvents",
        "TranslateCoords",
        "WarpPointer",
        "SetInputFocus",
        "GetInputFocus",
        "QueryKeymap",
        "OpenFont",
        "CloseFont",
        "QueryFont",
        "QueryTextExtents",
        "ListFonts",
        "ListFontsWithInfo",
        "SetFontPath",
        "GetFontPath",
        "CreatePixmap",
        "FreePixmap",
        "CreateGC",
        "ChangeGC",
        "CopyGC",
        "SetDashes",
        "SetClipRectangles",
        "FreeGC",
        "ClearArea",
        "CopyArea",
        "CopyPlane",
        "PolyPoint",
        "PolyLine",
        "PolySegment",
        "PolyRectangle",
        "PolyArc",
        "FillPoly",
        "PolyFillRectangle",
        "PolyFillArc",
        "PutImage",
        "GetImage",
        "PolyText8",
        "PolyText16",
        "ImageText8",
        "ImageText16",
        "CreateColormap",
        "FreeColormap",
        "CopyColormapAndFree",
        "InstallColormap",
        "UninstallColormap",
        "ListInstalledColormaps",
        "AllocColor",
        "AllocNamedColor",
        "AllocColorCells",
        "AllocColorPlanes",
        "FreeColors",
        "StoreColors",
        "StoreNamedColor",
        "QueryColors",
        "LookupColor",
        "CreateCursor",
        "CreateGlyphCursor",
        "FreeCursor",
        "RecolorCursor",
        "QueryBestSize",
        "QueryExtension",
        "ListExtensions",
        "ChangeKeyboardMapping",
        "GetKeyboardMapping",
        "ChangeKeyboardControl",
        "GetKeyboardControl",
        "Bell",
        "ChangePointerControl",
        "GetPointerControl",
        "SetScreenSaver",
        "GetScreenSaver",
        "ChangeHosts",
        "ListHosts",
        "SetAccessControl",
        "SetCloseDownMode",
        "KillClient",
        "RotateProperties",
        "ForceScreenSaver",
        "SetPointerMapping",
        "GetPointerMapping",
        "SetModifierMapping",
        "GetModifierMapping",
        "Unknown"
};

constinit size_t xcb_errors_size = sizeof(xcb_errors) / sizeof(const char*);
constinit size_t xcb_requests_size = sizeof(xcb_requests) / sizeof(const char*);

void handle_xcb_error(const xcb_generic_error_t *ptr)
{
    uint16_t clamped_err = std::min<uint16_t>(ptr->error_code, xcb_errors_size - 1);
    uint16_t clamped_major = std::min<uint16_t>(ptr->major_code, xcb_requests_size - 1);
    QLOG(LOG_ERROR, "XCB Error: {} ({}), request {}:{} ({}), sequence {}, resource {}",
                   ptr->error_code, xcb_errors[clamped_err],
                   ptr->major_code, ptr->minor_code,
                   xcb_requests[clamped_major],
                   ptr->sequence,
                   ptr->resource_id);
}

} // namespace anonymous

void XcbDisplay::handleEvent(const xcb_generic_event_t *event)
{
    CHECK(event != nullptr);

    uint32_t type = event->response_type & ~0x80;
    bool handled = true;
    switch (type)
    {
    case 0:
        handle_xcb_error(reinterpret_cast<const xcb_generic_error_t*>(event));
        break;

    invoke_window_handler_case(XCB_EXPOSE, expose, window)
    invoke_window_handler_case(XCB_CONFIGURE_NOTIFY, configure_notify, window)
    invoke_window_handler_case(XCB_CLIENT_MESSAGE, client_message, window)
    invoke_window_handler_case(XCB_MAP_NOTIFY, map_notify, window)
    invoke_window_handler_case(XCB_UNMAP_NOTIFY, unmap_notify, window)
    invoke_window_handler_case(XCB_KEY_PRESS, key_press, event)
    invoke_window_handler_case(XCB_KEY_RELEASE, key_release, event)

    case XCB_REPARENT_NOTIFY:
    {
        event_cast(reparent_notify)
        QLOG(LOG_INFO, "Window 0x{:08x} re-parenting, new parent is 0x{:08x}",
                reparent_notify->window, reparent_notify->parent);
        break;
    }

    case XCB_GE_GENERIC:
        handleXInputEvent(reinterpret_cast<const xcb_ge_event_t*>(event));
        break;

    default:
        handled = false;
        break;
    }

    if (handled)
    {
        xcb_flush(fConnection);
        return;
    }

    if (type == fKeyboard.xkbResponseType())
        fKeyboard.handleXkbEvent(event);
    else
        QLOG(LOG_WARNING, "Unknown event from X server: response_type = 0x{:04x}",
                event->response_type);

    xcb_flush(fConnection);
}

#undef event_cast
#undef invoke_window_handler
#undef invoke_window_handler_case
VANILLA_NS_END
