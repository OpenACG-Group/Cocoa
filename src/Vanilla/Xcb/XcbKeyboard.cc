#include <xcb/xcb.h>
#include <xcb/xcbext.h>

#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-x11.h>

#include "Core/Journal.h"
#include "Core/Exception.h"
#include "Vanilla/Base.h"
#include "Vanilla/KeyboardProxy.h"
#include "Vanilla/Xcb/XcbKeyboard.h"
#include "Vanilla/Xcb/XcbDisplay.h"

#define explicit __explicit
#include <xcb/xkb.h>

VANILLA_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Vanilla)

XcbKeyboard::XcbKeyboard(XcbDisplay *display)
    : fDisplay(display),
      fXkbContext(nullptr),
      fXkbKeymap(nullptr),
      fXkbState(nullptr),
      fProxy(nullptr),
      fResponseType(0),
      fCoreDeviceId(0)
{
    xcb_connection_t *connection = fDisplay->connection();

    auto reply = xcb_get_extension_data(connection, &xcb_xkb_id);
    if (!reply)
        throw VanillaException(__func__, "Failed in querying XKB extension which is required");
    fResponseType = reply->first_event;

    uint16_t major, minor;
    xkb_x11_setup_xkb_extension(connection,
                                XKB_X11_MIN_MAJOR_XKB_VERSION,
                                XKB_X11_MIN_MINOR_XKB_VERSION,
                                XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
                                &major, &minor,
                                nullptr, nullptr);
    LOGF(LOG_INFO, "XKB extension for X11 has been detected, version {}.{}", major, minor)

    int32_t deviceId = xkb_x11_get_core_keyboard_device_id(connection);
    if (deviceId == -1)
        throw VanillaException(__func__, "Failed to get core keyboard device ID");
    LOGF(LOG_INFO, "Using XInput keyboard device #{}", deviceId)

    fXkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    fXkbKeymap = xkb_x11_keymap_new_from_device(fXkbContext, connection, deviceId,
                                                XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (!fXkbKeymap)
    {
        xkb_context_unref(fXkbContext);
        throw VanillaException(__func__, "Failed to create XKB keymap");
    }

    fXkbState = xkb_x11_state_new_from_device(fXkbKeymap, connection, deviceId);
    if (!fXkbState)
    {
        xkb_keymap_unref(fXkbKeymap);
        xkb_context_unref(fXkbContext);
        throw VanillaException(__func__, "Failed to create XKB state");
    }

    uint16_t map = XCB_XKB_EVENT_TYPE_STATE_NOTIFY
                   | XCB_XKB_EVENT_TYPE_MAP_NOTIFY
                   | XCB_XKB_EVENT_TYPE_NEW_KEYBOARD_NOTIFY;

    uint16_t mapParts = XCB_XKB_MAP_PART_KEY_TYPES
                        | XCB_XKB_MAP_PART_KEY_SYMS
                        | XCB_XKB_MAP_PART_MODIFIER_MAP
                        | XCB_XKB_MAP_PART_EXPLICIT_COMPONENTS
                        | XCB_XKB_MAP_PART_KEY_ACTIONS
                        | XCB_XKB_MAP_PART_KEY_BEHAVIORS
                        | XCB_XKB_MAP_PART_VIRTUAL_MODS
                        | XCB_XKB_MAP_PART_VIRTUAL_MOD_MAP;
    xcb_xkb_select_events(connection,
                          deviceId,
                          map,
                          0,
                          map,
                          mapParts,
                          mapParts,
                          nullptr);

    fCoreDeviceId = deviceId;
    fProxy = std::make_unique<KeyboardProxy>(fXkbContext, fXkbKeymap, fXkbState);
}

XcbKeyboard::~XcbKeyboard()
{
    xkb_state_unref(fXkbState);
    xkb_keymap_unref(fXkbKeymap);
    xkb_context_unref(fXkbContext);
}

namespace {

typedef union
{
    struct {
        uint8_t response_type;
        uint8_t xkb_type;
        uint16_t sequence;
        xcb_timestamp_t time;
        uint8_t device_id;
    } common;
    xcb_xkb_new_keyboard_notify_event_t new_kb_notify;
    xcb_xkb_map_notify_event_t map_notify;
    xcb_xkb_state_notify_event_t state_notify;
} xkb_event_t;

} // namespace anonymous

void XcbKeyboard::handleXkbEvent(const xcb_generic_event_t *event)
{
    auto xkbEvent = reinterpret_cast<const xkb_event_t*>(event);
    if (xkbEvent->common.device_id != fCoreDeviceId)
        return;

    switch (xkbEvent->common.xkb_type)
    {
    case XCB_XKB_STATE_NOTIFY:
        xkb_state_update_mask(fXkbState,
                              xkbEvent->state_notify.baseMods,
                              xkbEvent->state_notify.latchedMods,
                              xkbEvent->state_notify.lockedMods,
                              xkbEvent->state_notify.baseGroup,
                              xkbEvent->state_notify.latchedGroup,
                              xkbEvent->state_notify.lockedGroup);
        break;

    case XCB_XKB_MAP_NOTIFY:
        LOGW(LOG_DEBUG, "XKB map notify")
        break;

    case XCB_XKB_NEW_KEYBOARD_NOTIFY:
        LOGW(LOG_DEBUG, "XKB new keyboard notify")
        break;
    }
}

KeySymbol XcbKeyboard::symbol(xcb_keycode_t code)
{
    xkb_keysym_t sym = xkb_state_key_get_one_sym(fXkbState, code);
    return va_translate_to_key_symbol(sym);
}

VANILLA_NS_END
