#include <xcb/xcb.h>
#include <xcb/xinput.h>

#include "Core/Journal.h"
#include "Vanilla/Base.h"
#include "Vanilla/Xcb/VaXcbDisplay.h"
#include "Vanilla/Xcb/VaXcbWindow.h"
VANILLA_NS_BEGIN

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

struct XInputMask
{
    xcb_input_event_mask_t head;
    uint32_t mask;
};

} // namespace anonymous

void VaXcbDisplay::selectXInputEventForWindow(xcb_window_t window)
{
    XInputMask mask{};
    mask.head.deviceid = XCB_INPUT_DEVICE_ALL_MASTER;
    mask.head.mask_len = 1;
    mask.mask = XCB_INPUT_XI_EVENT_MASK_BUTTON_PRESS
                | XCB_INPUT_XI_EVENT_MASK_BUTTON_RELEASE
                | XCB_INPUT_XI_EVENT_MASK_MOTION
                | XCB_INPUT_XI_EVENT_MASK_TOUCH_BEGIN
                | XCB_INPUT_XI_EVENT_MASK_TOUCH_UPDATE
                | XCB_INPUT_XI_EVENT_MASK_TOUCH_END;

    xcb_input_xi_select_events(fConnection, window, 1, &mask.head);
}

void VaXcbDisplay::handleXInputEvent(const xcb_ge_event_t *event)
{
    switch (event->event_type)
    {
    invoke_window_handler_case(XCB_INPUT_BUTTON_PRESS, input_button_press, event)
    invoke_window_handler_case(XCB_INPUT_BUTTON_RELEASE, input_button_release, event)
    invoke_window_handler_case(XCB_INPUT_MOTION, input_motion, event)
    invoke_window_handler_case(XCB_INPUT_TOUCH_UPDATE, input_touch_update, event)
    invoke_window_handler_case(XCB_INPUT_TOUCH_BEGIN, input_touch_begin, event)
    invoke_window_handler_case(XCB_INPUT_TOUCH_END, input_touch_end, event)

    default:
        Journal::Ref()(LOG_WARNING, "Unknown X11 event from XInput: event_type = {:04x}",
                       event->event_type);
    }
}

#undef invoke_window_handler_case
#undef invoke_window_handler
#undef event_cast

VANILLA_NS_END
