#ifndef COCOA_GSKSEAT_H
#define COCOA_GSKSEAT_H

#include <list>
#include <functional>
#include <sigc++/sigc++.h>

#include "Core/EnumClassBitfield.h"
#include "Core/Errors.h"
#include "Gsk/Gsk.h"
#include "Gsk/GskEnumerations.h"
GSK_NAMESPACE_BEGIN

class GskDisplay;
class GskDevice;
class GskSurface;
class GskCursor;
class GskEvent;

class GskSeat : public std::enable_shared_from_this<GskSeat>
{
public:
    using SeatGrabPreparePfn = std::function<void(const Handle<GskSeat>&, const Handle<GskSurface>&)>;

    virtual ~GskSeat() = default;

    g_nodiscard g_inline Handle<GskDisplay> getDisplay() const {
        CHECK(!fDisplay.expired());
        return fDisplay.lock();
    }

    Bitfield<SeatCapabilities> getCapabilities();
    const std::list<Handle<GskDevice>>& getDevices(Bitfield<SeatCapabilities> caps);

    Handle<GskDevice> getPointer();
    Handle<GskDevice> getKeyboard();

    /* Internal methods */

    void deviceAdded(const Handle<GskDevice>& device);
    void deviceRemoved(const Handle<GskDevice>& device);
    GskGrabStatus grab(const Handle<GskSurface>& surface,
                       Bitfield<SeatCapabilities> capabilities,
                       bool ownerEvents,
                       const Handle<GskCursor>& cursor,
                       const Handle<GskEvent>& event,
                       const SeatGrabPreparePfn& preparePfn);
    void unGrab();

    g_signal_getter(DeviceAdded);
    g_signal_getter(DeviceRemoved);

protected:
    explicit GskSeat(const Weak<GskDisplay>& display);

    virtual Bitfield<SeatCapabilities> onGetCapabilities() = 0;
    virtual GskGrabStatus onGrab(const Handle<GskSurface>& surface,
                                 Bitfield<SeatCapabilities> capabilities,
                                 bool ownerEvents,
                                 const Handle<GskCursor>& cursor,
                                 const Handle<GskEvent>& event,
                                 SeatGrabPreparePfn preparePfn) = 0;
    virtual void onUnGrab() = 0;
    virtual const std::list<Handle<GskDevice>>& onGetDevices(Bitfield<SeatCapabilities> caps) = 0;
    virtual Handle<GskDevice> onGetLogicalDevice(SeatCapabilities cap) = 0;

private:
    g_signal_fields(
        g_signal_signature(void(const Handle<GskDevice>&), DeviceAdded)
        g_signal_signature(void(const Handle<GskDevice>&), DeviceRemoved)
    )

    Weak<GskDisplay>    fDisplay;
};

GSK_NAMESPACE_END
#endif //COCOA_GSKSEAT_H
