#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Gsk/wayland/GskWaylandSeat.h"
#include "Gsk/wayland/GskWaylandDisplay.h"
#include "Gsk/GskDevice.h"
GSK_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gsk.Wayland.Seat)

namespace {

} // namespace anonymous

GskWaylandSeat::GskWaylandSeat(const Weak<GskDisplay>& displayWeak, wl_seat *wlSeat, uint32_t id)
    : GskSeat(displayWeak)
    , fId(id)
    , fWlSeat(wlSeat)
{
    auto display = std::dynamic_pointer_cast<GskWaylandDisplay>(getDisplay());
    CHECK(display != nullptr);

    wl_seat_add_listener(wlSeat, nullptr, this);
    wl_seat_set_user_data(wlSeat, this);

    // TODO(seat): Remaining steps (gdkdisplay-wayland.c:233).
}

GskWaylandSeat::~GskWaylandSeat()
{
}

const std::list<Handle<GskDevice>>& GskWaylandSeat::onGetDevices(Bitfield<SeatCapabilities> caps)
{
}

void GskWaylandSeat::onUnGrab()
{
}

GskGrabStatus
GskWaylandSeat::onGrab(const Handle<GskSurface>& surface, Bitfield<SeatCapabilities> capabilities, bool ownerEvents,
                       const Handle<GskCursor>& cursor, const Handle<GskEvent>& event, SeatGrabPreparePfn preparePfn)
{

}

Bitfield<SeatCapabilities> GskWaylandSeat::onGetCapabilities()
{
}

Handle<GskDevice> GskWaylandSeat::onGetLogicalDevice(SeatCapabilities cap)
{
}

GskWaylandTabletData *GskWaylandSeat::findTablet(const Handle<GskDevice>& device)
{
    for (GskWaylandTabletData *ptr : fTablets)
    {
        if (ptr->logicalDevice == device || ptr->stylusDevice == device)
            return ptr;
    }

    return nullptr;
}

GskWaylandTabletPadData *GskWaylandSeat::findPad(const Handle<GskDevice>& device)
{
    for (GskWaylandTabletPadData *pad : fTabletPads)
    {
        if (pad->device == device)
            return pad;
    }

    return nullptr;
}

GSK_NAMESPACE_END
