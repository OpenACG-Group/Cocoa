#include "Core/Journal.h"

#include "Vanilla/Wayland/WaylandSeat.h"
#include "Vanilla/Wayland/WaylandDisplay.h"

VANILLA_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Vanilla.Wayland.Seat)

namespace {

void pointer_handle_enter(void *data,
                          ::wl_pointer *pointer,
                          uint32_t serial,
                          ::wl_surface *surface,
                          ::wl_fixed_t sx,
                          ::wl_fixed_t sy)
{
}

void pointer_handle_leave(void *data,
                          ::wl_pointer *pointer,
                          uint32_t serial,
                          ::wl_surface *surface)
{
}

void pointer_handle_motion(void *data,
                           ::wl_pointer *pointer,
                           uint32_t time,
                           wl_fixed_t sx,
                           wl_fixed_t sy)
{
}

void pointer_handle_button(void *data,
                           ::wl_pointer *pointer,
                           uint32_t serial,
                           uint32_t time,
                           uint32_t button,
                           uint32_t state)
{
}

void pointer_handle_axis(void *data,
                         ::wl_pointer *pointer,
                         uint32_t time,
                         uint32_t axis,
                         wl_fixed_t value)
{
}

void pointer_handle_frame(void *data,
                          ::wl_pointer *pointer)
{
}

void pointer_handle_axis_source(void *data,
                                ::wl_pointer *pointer,
                                uint32_t source)
{
}

void pointer_handle_axis_stop(void *data,
                              ::wl_pointer *pointer,
                              uint32_t time,
                              uint32_t axis)
{
}

void pointer_handle_axis_discrete(void *data,
                                  ::wl_pointer *pointer,
                                  uint32_t axis,
                                  int32_t value)
{
}

const ::wl_pointer_listener pointer_listener_ = {
        pointer_handle_enter,
        pointer_handle_leave,
        pointer_handle_motion,
        pointer_handle_button,
        pointer_handle_axis,
        pointer_handle_frame,
        pointer_handle_axis_source,
        pointer_handle_axis_stop,
        pointer_handle_axis_discrete
};

const ::zwp_pointer_gesture_swipe_v1_listener pointer_swipe_listener_ = {
};

const ::zwp_pointer_gesture_pinch_v1_listener pointer_pinch_listener_ = {
};

void seat_listener_capability_handler(void *data, ::wl_seat *seat, uint32_t caps)
{
    if (caps == 0)
        return;
    auto *pSeat = reinterpret_cast<WaylandSeat*>(data);
    auto& fields = pSeat->getDataFields();

    auto& batch = pSeat->getDisplay()->getInterfacesBatch();

    if (caps & ::WL_SEAT_CAPABILITY_POINTER)
    {
        fields.pointer = ::wl_seat_get_pointer(seat);
        ::wl_pointer_set_user_data(fields.pointer, data);
        ::wl_pointer_add_listener(fields.pointer, &pointer_listener_, data);
        if (batch->pointer_gestures.first)
        {
            fields.wp_pointer_swipe = ::zwp_pointer_gestures_v1_get_swipe_gesture(
                    batch->pointer_gestures.first, fields.pointer);
            ::zwp_pointer_gesture_swipe_v1_set_user_data(fields.wp_pointer_swipe, data);
            ::zwp_pointer_gesture_swipe_v1_add_listener(fields.wp_pointer_swipe,
                                                        &pointer_swipe_listener_, data);

            fields.wp_pointer_pinch = ::zwp_pointer_gestures_v1_get_pinch_gesture(
                    batch->pointer_gestures.first, fields.pointer);
            ::zwp_pointer_gesture_pinch_v1_set_user_data(fields.wp_pointer_pinch, data);
            ::zwp_pointer_gesture_pinch_v1_add_listener(fields.wp_pointer_pinch,
                                                        &pointer_pinch_listener_, data);
        }
    }

    // TODO: Handle other devices.
}

void seat_listener_name_handler(void *data, ::wl_seat *seat, const char *name)
{
    /* We do not care about the name */
    if (!name || !std::strlen(name))
        return;
    QLOG(LOG_DEBUG, "Seat device \"{}\"", name);
}

const ::wl_seat_listener seat_listener_ = {
        seat_listener_capability_handler,
        seat_listener_name_handler
};

} // namespace anonymous

WaylandSeat::WaylandSeat(const Handle<WaylandDisplay>& display)
    : fDisposed(false)
    , fDisplay(display)
    , fWlSeat(std::get<0>(display->getInterfacesBatch()->seat))
    , fWlSeatId(std::get<1>(display->getInterfacesBatch()->seat))
{
    ::wl_seat_add_listener(fWlSeat, &seat_listener_, this);
    ::wl_seat_set_user_data(fWlSeat, this);

    if (display->getInterfacesBatch()->primary_selection_manager.first)
    {
        QLOG(LOG_DEBUG, "Wayland provides clipboard by selection manager");
        // TODO: Create clipboard
    }
    else
    {
        QLOG(LOG_WARNING, "Wayland does not provide clipboard");
    }

    fDataFields.data_device = ::wl_data_device_manager_get_data_device(
            display->getInterfacesBatch()->data_device_manager.first, fWlSeat);
    // TODO: Complete this.
}

void WaylandSeat::dispose()
{
    if (fDisposed)
        return;

    if (fDataFields.wp_pointer_swipe)
        ::zwp_pointer_gesture_swipe_v1_destroy(fDataFields.wp_pointer_swipe);
    if (fDataFields.wp_pointer_pinch)
        ::zwp_pointer_gesture_pinch_v1_destroy(fDataFields.wp_pointer_pinch);
    if (fDataFields.pointer)
        ::wl_pointer_release(fDataFields.pointer);
    if (fDataFields.data_device)
        ::wl_data_device_release(fDataFields.data_device);
    ::wl_seat_destroy(fWlSeat);

    fDisposed = true;
}

WaylandSeat::~WaylandSeat()
{
    this->dispose();
}

VANILLA_NS_END
