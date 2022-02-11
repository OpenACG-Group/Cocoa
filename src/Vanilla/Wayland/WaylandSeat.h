#ifndef COCOA_WAYLANDSEAT_H
#define COCOA_WAYLANDSEAT_H

#include <wayland-client-protocol.h>
#include "Vanilla/Wayland/pointer-gestures-unstable-v1-client-protocol.h"

#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

class WaylandDisplay;

class WaylandSeat
{
public:
    struct DataFields
    {
        DataFields() = default;
        ::wl_pointer *pointer = nullptr;
        ::wl_data_device *data_device = nullptr;
        ::zwp_pointer_gesture_swipe_v1 *wp_pointer_swipe = nullptr;
        ::zwp_pointer_gesture_pinch_v1 *wp_pointer_pinch = nullptr;
    };

    explicit WaylandSeat(const Handle<WaylandDisplay>& display);
    ~WaylandSeat();

    va_nodiscard inline DataFields& getDataFields() {
        return fDataFields;
    }

    va_nodiscard inline Handle<WaylandDisplay> getDisplay() const {
        return fDisplay.lock();
    }

    void dispose();

private:
    bool                           fDisposed;
    WeakHandle<WaylandDisplay>     fDisplay;
    ::wl_seat                     *fWlSeat;
    uint32_t                       fWlSeatId;
    DataFields                     fDataFields;
};

VANILLA_NS_END
#endif //COCOA_WAYLANDSEAT_H
