#include <cassert>
#include <xcb/xcb.h>

#include "Core/Journal.h"
#include "Vanilla/Xcb/VaXcbDisplay.h"
#include "Vanilla/Xcb/VaXcbWindow.h"
#include "Vanilla/Xcb/EventHandlerMacros.h"
VANILLA_NS_BEGIN

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

Handle<VaDisplay> VaDisplay::OpenXcb(const Handle<Context>& ctx, const char *displayName)
{
    int screenNumber;
    xcb_connection_t *connection = xcb_connect(displayName, &screenNumber);
    if (!connection)
        return nullptr;
    BeforeLeaveScope before([connection]() -> void {
        xcb_disconnect(connection);
    });

    if (xcb_connection_has_error(connection))
    {
        Journal::Ref()(LOG_ERROR, "Failed to connect to X11 server");
        return nullptr;
    }

    const xcb_setup_t *setup = xcb_get_setup(connection);
    Journal::Ref()(LOG_INFO, "Connected to X server, details:\n  Vendor: %fg<hl>{}%reset\n  Protocol Version: %fg<gr,hl>{}.{}%reset",
                   xcb_setup_vendor(setup), setup->protocol_major_version,
                   setup->protocol_minor_version);

    xcb_screen_t *screen = match_proper_screen(screenNumber, setup);
    if (!screen)
    {
        Journal::Ref()(LOG_ERROR, "Failed to query a proper X11 screen");
        return nullptr;
    }
    Journal::Ref()(LOG_INFO, "Found proper screen {}x{} with depth {}",
                   screen->width_in_pixels, screen->height_in_pixels, screen->root_depth);

    xcb_visualtype_t *visual = match_proper_visual(screen);
    if (!visual)
    {
        Journal::Ref()(LOG_ERROR, "Failed to query a proper X11 visual type");
        return nullptr;
    }

    if (visual->bits_per_rgb_value != 8 ||
        visual->red_mask != 0xff0000 || visual->green_mask != 0xff00 ||
        visual->blue_mask != 0xff)
    {
        Journal::Ref()(LOG_ERROR, "Unsupported color format. Only supports BGRA or BGRx now");
        return nullptr;
    }

    before.cancel();
    return std::make_shared<VaXcbDisplay>(ctx, connection, screen,
                                          visual, SkColorType::kBGRA_8888_SkColorType);
}

VaXcbDisplay::VaXcbDisplay(const Handle<Context>& context,
                           xcb_connection_t *pConnection,
                           xcb_screen_t *pScreen,
                           xcb_visualtype_t *pVisual,
                           SkColorType format)
    : VaDisplay(DisplayBackend::kDisplay_Xcb, context),
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

VaXcbDisplay::~VaXcbDisplay()
{
    if (!fDisposed)
        onDispose();
}

void VaXcbDisplay::dispose()
{
    this->onDispose();
}

void VaXcbDisplay::onDispose()
{
    fDisposed = true;
    fEventQueue.disposeFromMainThread();
    xcb_disconnect(fConnection);
}

void VaXcbDisplay::flush()
{
    xcb_flush(fConnection);
}

int32_t VaXcbDisplay::width()
{
    return fScreen->width_in_pixels;
}

int32_t VaXcbDisplay::height()
{
    return fScreen->height_in_pixels;
}

Handle<VaWindow> VaXcbDisplay::onCreateWindow(VaVec2f size, VaVec2f pos, Handle<VaWindow> parent)
{
    xcb_window_t parentWindow = fScreen->root;
    if (parent != nullptr)
        parentWindow = std::dynamic_pointer_cast<VaXcbWindow>(parent)->window();

    auto x = static_cast<int16_t>(pos.x()), y = static_cast<int16_t>(pos.y());
    auto w = static_cast<int32_t>(size.x()), h = static_cast<int32_t>(size.y());

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
    xcb_atom_t deleteWindowAtom = fAtoms.get(VaXcbAtoms::WM_DELETE_WINDOW);
    xcb_change_property(fConnection,
                        XCB_PROP_MODE_REPLACE,
                        window,
                        fAtoms.get(VaXcbAtoms::WM_PROTOCOLS),
                        XCB_ATOM_ATOM,
                        32, 1,
                        &deleteWindowAtom);

    selectXInputEventForWindow(window);
    xcb_flush(fConnection);
    return std::make_shared<VaXcbWindow>(shared_from_this(),
                                         window, w, h, fFormat);
}

Handle<VaXcbWindow> VaXcbDisplay::matchWindow(xcb_window_t window)
{
    Handle<VaXcbWindow> result = nullptr;
    VaDisplay::forEachWindow([window, &result](Handle<VaWindow> pWindow) -> bool {
        auto pXcbWindow = std::dynamic_pointer_cast<VaXcbWindow>(std::move(pWindow));
        if (window == pXcbWindow->window())
        {
            result = pXcbWindow;
            return false;
        }
        return true;
    });
    return result;
}

VaKeyboardProxy *VaXcbDisplay::keyboardProxy()
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
    Journal::Ref()(LOG_ERROR, "XCB Error: {} ({}), request {}:{} ({}), sequence {}, resource {}",
                   ptr->error_code, xcb_errors[clamped_err],
                   ptr->major_code, ptr->minor_code,
                   xcb_requests[clamped_major],
                   ptr->sequence,
                   ptr->resource_id);
}

} // namespace anonymous

void VaXcbDisplay::handleEvent(const xcb_generic_event_t *event)
{
    assert(event != nullptr);

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
        Journal::Ref()(LOG_INFO, "Window 0x{:08x} re-parenting, new parent is 0x{:08x}",
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
        Journal::Ref()(LOG_WARNING, "Unknown event from X server: response_type = 0x{:04x}",
                event->response_type);

    xcb_flush(fConnection);
}

#undef event_cast
#undef invoke_window_handler
#undef invoke_window_handler_case
VANILLA_NS_END
