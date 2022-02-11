#ifndef COCOA_GSKWAYLANDSEAT_H
#define COCOA_GSKWAYLANDSEAT_H

#include <wayland-client-protocol.h>
#include "Gsk/wayland/protos/pointer-gestures-unstable-v1-client-protocol.h"
#include "Gsk/wayland/protos/tablet-unstable-v2-client-protocol.h"

#include "Gsk/Gsk.h"
#include "Gsk/GskSeat.h"
#include "Gsk/GskKeymap.h"

GSK_NAMESPACE_BEGIN

struct GskWaylandTouchData
{
    uint32_t id;
    Vec2d xy;
    Handle<GskSurface> surface;
    uint32_t touchDownSerial;
    uint32_t initialTouch : 1;
};

struct GskWaylandPointerFrameData
{
    Handle<GskEvent> event;

    /* Specific to the scroll event */
    Vec2d deltaXY;
    Vec2i discreteXY;
    bool isScrollStop;
    wl_pointer_axis_source source;
};

struct GskWaylandPointerData
{
    Handle<GskSurface> focus;

    Vec2d surfaceXY;

    Bitfield<GskModifierType> buttonModifiers;

    uint32_t time;
    uint32_t enterSerial;
    uint32_t pressSerial;

    Handle<GskSurface> grabSurface;
    uint32_t grabTime;

    wl_surface *pointerSurface;
    bool cursorIsDefault;
    Handle<GskCursor> cursor;
    uint32_t cursorTimeoutId;
    uint32_t cursorImageIndex;
    uint32_t cursorImageDelay;
    uint32_t touchpadEventSequence;

    uint32_t currentOutputScale;
    // TODO(type): pointer_surface_outputs (gdkdevice-wayland.c:140)

    GskWaylandPointerFrameData frame;
};

struct GskWaylandTabletData;
struct GskWaylandTabletToolData
{
    Weak<GskSeat> seat;
    zwp_tablet_tool_v2 *wpTabletTool;
    Bitfield<GskAxisFlags> axes;
    // TODO: device_tool_type

    uint64_t hardwareSerial;
    uint64_t hardwareIdWacom;

    GskWaylandTabletData *currentTablet;
};

struct GskWaylandTabletData
{
    zwp_tablet_v2 *wpTablet;
    std::string name;
    std::string path;
    uint32_t vid;
    uint32_t pid;

    Handle<GskDevice> logicalDevice;
    Handle<GskDevice> stylusDevice;
    Weak<GskSeat> seat;
    GskWaylandPointerData pointerInfo;

    // TODO(type): pads (gdkdevice-wayland.c:211)

    GskWaylandTabletToolData *currentTool;

    static const size_t AXIS_SIZE =
            static_cast<std::underlying_type_t<GskAxisUse>>(GskAxisUse::kLast);

    int axisIndices[AXIS_SIZE];
    double axes[AXIS_SIZE];
};

struct GskWaylandTabletPadData
{
    Weak<GskSeat> seat;
    zwp_tablet_pad_v2 *wpTabletPad;
    Handle<GskDevice> device;

    GskWaylandTabletData *currentTablet;

    uint32_t enterSerial;
    uint32_t nButtons;
    std::string path;

    // TODO(type): rings, strips, mode_groups (gdkdevice-wayland.c:193)
};

struct GskWaylandTabletPadGroupData
{
    GskWaylandTabletPadData *pad;
    zwp_tablet_pad_group_v2 *wpTabletPadGroup;

    // TODO(type): rings, strips, buttons (gdkdevice-wayland.c:166)

    uint32_t modeSwitchSerial;
    uint32_t nModes;
    uint32_t currentMode;

    struct {
        uint32_t source;
        bool isStop;
        double value;
    } axisTmpInfo;
};

class GskWaylandSeat : public GskSeat
{
public:
    friend class GskWaylandDevice;

    GskWaylandSeat(const Weak<GskDisplay>& display, wl_seat *wlSeat, uint32_t id);
    ~GskWaylandSeat() override;

    static void SeatHandleName(void *data, wl_seat *pSeat, const char *name);
    static void SeatHandleCapabilities(void *data, wl_seat *pSeat, uint32_t caps);

    g_nodiscard g_inline wl_seat *getWlSeat() const {
        return fWlSeat;
    }

    void updateCursorScale() {}
    void clearTouchpoint(const Handle<GskSurface>& surface);

    GskWaylandTabletData *findTablet(const Handle<GskDevice>& device);
    GskWaylandTabletPadData *findPad(const Handle<GskDevice>& device);

private:
    Bitfield<SeatCapabilities> onGetCapabilities() override;
    GskGrabStatus onGrab(const Handle<GskSurface> &surface,
                         Bitfield<SeatCapabilities> capabilities,
                         bool ownerEvents,
                         const Handle<GskCursor> &cursor,
                         const Handle<GskEvent> &event,
                         SeatGrabPreparePfn preparePfn) override;
    void onUnGrab() override;
    Handle<GskDevice> onGetLogicalDevice(SeatCapabilities cap) override;
    const std::list<Handle<GskDevice>> & onGetDevices(Bitfield<SeatCapabilities> caps) override;

    uint32_t            fId;
    wl_seat            *fWlSeat;

    wl_pointer         *fWlPointer;
    wl_keyboard        *fWlKeyboard;
    wl_touch           *fWlTouch;
    zwp_pointer_gesture_swipe_v1 *fWpGestureSwipe;
    zwp_pointer_gesture_pinch_v1 *fWpGesturePinch;
    zwp_tablet_seat_v2  *fWpTabletSeat;

    Handle<GskDevice>   fLogicalPointer;
    Handle<GskDevice>   fLogicalKeyboard;
    Handle<GskDevice>   fPointer;
    Handle<GskDevice>   fWheelScrolling;
    Handle<GskDevice>   fFingerScrolling;
    Handle<GskDevice>   fContinuousScrolling;
    Handle<GskDevice>   fKeyboard;
    Handle<GskDevice>   fLogicalTouch;
    Handle<GskCursor>   fCursor;
    Handle<GskKeymap>   fKeymap;

    // TODO(seat-wayland): touches, tablets (gdkdevice-wayland.c:246)
    std::list<GskWaylandTabletData*> fTablets;
    std::list<GskWaylandTabletPadData*> fTabletPads;

    GskWaylandPointerData   fPointerInfo;
    GskWaylandPointerData   fTouchInfo;

    Bitfield<GskModifierType>   fKeyModifiers;
    Handle<GskSurface>          fKeyboardFocus;
    Handle<GskSurface>          fGrabSurface;
    uint32_t                    fGrabTime;
    bool                        fHaveServerRepeat;
    uint32_t                    fServerRepeatRate;
    uint32_t                    fServerRepeatDelay;

    // TODO(clipboard-drag-drop-wayland): gdkdevice-wayland.c:262&275

    wl_callback                *fRepeatCallback;
    uint32_t                    fRepeatTimer;
    uint32_t                    fRepeatKey;
    uint32_t                    fRepeatCount;
    int64_t                     fRepeatDeadline;
    uint32_t                    fKeyboardTime;
    uint32_t                    fKeyboardKeySerial;

    uint32_t                    fGestureNFingers;
    double                      fGestureScale;

    Handle<GskCursor>           fGrabCursor;
};

GSK_NAMESPACE_END
#endif //COCOA_GSKWAYLANDSEAT_H
