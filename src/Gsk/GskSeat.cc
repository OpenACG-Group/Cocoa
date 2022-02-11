#include "Gsk/GskSeat.h"
#include "Gsk/GskDevice.h"
GSK_NAMESPACE_BEGIN

GskSeat::GskSeat(const Weak<GskDisplay>& display)
    : fDisplay(display)
{
}

Handle<GskDevice> GskSeat::getPointer()
{
    return this->onGetLogicalDevice(SeatCapabilities::kPointer);
}

Handle<GskDevice> GskSeat::getKeyboard()
{
    return this->onGetLogicalDevice(SeatCapabilities::kKeyboard);
}

Bitfield<SeatCapabilities> GskSeat::getCapabilities()
{
    return this->onGetCapabilities();
}

GskGrabStatus
GskSeat::grab(const Handle<GskSurface>& surface,
              Bitfield<SeatCapabilities> capabilities,
              bool ownerEvents,
              const Handle<GskCursor>& cursor,
              const Handle<GskEvent>& event,
              const SeatGrabPreparePfn& preparePfn)
{
    return this->onGrab(surface,
                        capabilities,
                        ownerEvents,
                        cursor,
                        event,
                        preparePfn);
}

void GskSeat::unGrab()
{
    this->onUnGrab();
}

const std::list<Handle<GskDevice>>& GskSeat::getDevices(Bitfield<SeatCapabilities> caps)
{
    return this->onGetDevices(caps);
}

void GskSeat::deviceAdded(const Handle<GskDevice>& device)
{
    CHECK(device);
    device->setSeat(weak_from_this());
    g_signal_emit(DeviceAdded, device);
}

void GskSeat::deviceRemoved(const Handle<GskDevice>& device)
{
    CHECK(device);
    device->setSeat(std::weak_ptr<GskSeat>());
    g_signal_emit(DeviceRemoved, device);
}

GSK_NAMESPACE_END
