#ifndef COCOA_GSKWAYLANDDEVICE_H
#define COCOA_GSKWAYLANDDEVICE_H

#include <wayland-client-protocol.h>
#include <xkbcommon/xkbcommon.h>

#include "Core/EventSource.h"
#include "Gsk/GskDevice.h"
GSK_NAMESPACE_BEGIN

class GskKeymap;
struct GskWaylandPointerData;
struct GskWaylandTouchData;

class GskWaylandDevice : public GskDevice
{
public:
    explicit GskWaylandDevice(const Weak<GskDisplay>& display);
    ~GskWaylandDevice() override;

    g_nodiscard wl_seat *getWlSeat();
    g_nodiscard wl_pointer *getWlPointer();
    g_nodiscard wl_keyboard *getWlKeyboard();
    g_nodiscard xkb_keymap *getXKBKeymap();
    g_nodiscard std::string getNodePath();

    g_private_api Handle<GskKeymap> getKeymap();
    g_private_api KeepInLoop updateSurfaceCursor();

private:
    GskGrabStatus onGrab(const Handle<GskSurface> &surface,
                         bool ownerEvents,
                         Bitfield<GskEventMask> eventMasks,
                         const Handle<GskSurface> &confineTo,
                         const Handle<GskCursor> &cursor, uint32_t time_) override;
    Handle<GskSurface> onSurfaceAtPosition(Vec2d &pos,
                                           Bitfield<GskModifierType> &mask) override;
    void onUnGrab(uint32_t time_) override;

    GskWaylandTouchData     *fEmulatingTouch;
    GskWaylandPointerData   *fPointer;
};

GSK_NAMESPACE_END
#endif //COCOA_GSKWAYLANDDEVICE_H
