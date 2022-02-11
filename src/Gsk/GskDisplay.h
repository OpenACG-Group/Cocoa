#ifndef COCOA_GSKDISPLAY_H
#define COCOA_GSKDISPLAY_H

#include <sigc++/sigc++.h>

#include "Core/EnumClassBitfield.h"
#include "Gsk/Gsk.h"
#include "Gsk/GskEnumerations.h"
GSK_NAMESPACE_BEGIN

class GskDisplayManager;
class GskSeat;
class GskMonitor;
class GskDevice;
class GskClipboard;
class GskSurface;
class GskEvent;
class GskEventQueue;
class GskKeymap;

class GskDisplay : public std::enable_shared_from_this<GskDisplay>
{
public:
    struct GskDisplayPrivate
    {
        bool has_alpha_channel {true};
        bool has_composited {true};
        bool has_input_shapes {true};
    };

    /* Tracks information about the device grab on this display */
    struct DeviceGrabInfo
    {
        Handle<GskSurface> surface;
        ulong serialStart;
        ulong serialEnd;
        Bitfield<GskEventMask> eventMask;
        uint32_t time;

        bool activated;
        bool implicitUnGrab;
        bool ownerEvents;
        bool implicit;
    };

    /*
     * Tracks information about which surface and position the pointer last was in.
     * This is useful when we need to synthesize events later.
     * Note that we track `toplevelUnderPointer` using enter/leave events,
     * so in the case of a grab, either with `ownerEvents == false` or with the
     * pointer in no clients surface the x/y coordinates may actually be outside
     * the surface.
     */
    struct PointerSurfaceInfo
    {
        /* surface that last got a normal enter event */
        Handle<GskSurface> surfaceUnderPointer;
        Vec2d toplevelPos;
        Bitfield<GskModifierType> state;
        uint32_t button;
        Handle<GskDevice> lastPhysicalDevice;
    };
    using PointerSurfaceInfoForeachPfn = std::function<void(const Handle<GskDisplay>&,
                                                            const Handle<GskDevice>&,
                                                            PointerSurfaceInfo *)>;

    static Handle<GskDisplay> Make(const std::string& displayName = "");

    GskDisplay();
    virtual ~GskDisplay();

    g_nodiscard g_inline Handle<GskDisplayManager> getDisplayManager() {
        return fManager.lock();
    }

    g_nodiscard g_inline const std::list<Handle<GskMonitor>>& getMonitor() const {
        return fMonitors;
    }

    g_nodiscard g_inline const std::list<Handle<GskSeat>>& getSeats() const {
        return fSeats;
    }

    g_nodiscard Handle<GskClipboard> getClipboard();
    g_nodiscard Handle<GskClipboard> getPrimaryClipboard();

    g_nodiscard g_inline GskEventQueue *getUniqueEventQueue() {
        return fEventQueue.get();
    }

    g_nodiscard Handle<GskSeat> getDefaultSeat();
    g_nodiscard Handle<GskMonitor> getMonitorOnSurface(const Handle<GskSurface>& surface);

    g_nodiscard bool hasSurfaceAlphaChannel() const;
    g_nodiscard bool hasSurfaceComposited() const;
    g_nodiscard bool hasInputShapes() const;

    bool deviceIsGrabbed(const Handle<GskDevice>& device);

    void flush();
    void sync();
    void close();

    g_nodiscard g_inline bool isClosed() const {
        return fClosed;
    }

    void setCursorTheme(const std::string& name, int size);

    /* Internal methods. User must not call them. */
    void addMonitor(const Handle<GskMonitor>& monitor);
    void addSeat(const Handle<GskSeat>& seat);
    void removeSeat(const Handle<GskSeat>& seat);
    void updateLastEvent(const Handle<GskEvent>& event);
    void deviceGrabUpdate(const Handle<GskDevice>& device, uint32_t currentSerial);
    DeviceGrabInfo *getLastDeviceGrab(const Handle<GskDevice>& device);
    DeviceGrabInfo *addDeviceGrab(const Handle<GskDevice>& device,
                                  const Handle<GskSurface>& surface,
                                  bool ownerEvents,
                                  Bitfield<GskEventMask> eventMask,
                                  ulong serialStart,
                                  uint32_t time,
                                  bool implicit);
    DeviceGrabInfo *hasDeviceGrab(const Handle<GskDevice>& device, ulong serial);
    bool endDeviceGrab(const Handle<GskDevice>& device,
                       ulong serial,
                       const Handle<GskSurface>& ifChild,
                       bool implicit);
    PointerSurfaceInfo *getPointerInfo(Handle<GskDevice> device);
    void pointerInfoForeach(const PointerSurfaceInfoForeachPfn& pfn);
    bool deviceGrabInfo(const Handle<GskDevice>& device, Handle<GskSurface>& outSurface, bool& ownerEvents);
    ulong getNextSerial();
    void pauseEvents();
    void unpauseEvents();
    Handle<GskSurface> createSurface(SurfaceType type,
                                     const Handle<GskSurface>& parent,
                                     const Vec2i& pos,
                                     const Vec2i& size);
    void setRGBA(bool rgba);
    void setComposited(bool composited);
    void setInputShapes(bool inputShapes);
    Handle<GskKeymap> getKeymap();
    void setDClickTime(uint32_t msec);
    void setDClickDistance(uint32_t distance);
    void setSurfaceUnderPointer(const Handle<GskDevice>& device, const Handle<GskSurface>& surface);


    g_signal_getter(Closed);
    g_signal_getter(SeatAdded);
    g_signal_getter(SeatRemoved);

protected:
    void initialize();
    Handle<GskEvent> getEventFromQueue();

    virtual bool onHasPending() = 0;
    virtual void onDispose() = 0;
    virtual void onFlush() = 0;
    virtual void onSync() = 0;
    virtual Handle<GskSeat> onGetDefaultSeat();
    virtual void onEnqueueEvents(Unique<GskEventQueue>& queue) = 0;
    virtual void onMakeDefault() = 0;
    virtual uint32_t onGetNextSerial() = 0;
    virtual Handle<GskSurface> onCreateSurface(SurfaceType type,
                                               const Handle<GskSurface>& parent,
                                               Vec2i pos,
                                               Vec2i size) = 0;
    virtual Handle<GskKeymap> onGetKeymap() = 0;
    virtual Handle<GskMonitor> onGetMonitorAtSurface(const Handle<GskSurface>& surface) = 0;
    virtual void onSetCursorTheme(const std::string& name, int size) = 0;

    void generateGrabBrokenEvent(const Handle<GskSurface>& surface, const Handle<GskDevice>& device,
                                 bool implicit, const Handle<GskSurface>& grabSurface);
    Handle<GskSurface> getCurrentToplevel(const Handle<GskDevice>& device, Vec2i& pos,
                                          Bitfield<GskModifierType>& state);
    void switchToPointerGrab(const Handle<GskDevice>& device,
                             DeviceGrabInfo *grab,
                             DeviceGrabInfo *lastGrab,
                             uint32_t time,
                             ulong serial);
    Maybe<std::list<DeviceGrabInfo*>::iterator> findDeviceGrab(const Handle<GskDevice>& device,
                                                               ulong serial);

    void removeMonitor(const Handle<GskMonitor>& monitor);

private:
    g_signal_fields(
        g_signal_signature(void(void), Closed)
        g_signal_signature(void(const Handle<GskSeat>&), SeatAdded)
        g_signal_signature(void(const Handle<GskSeat>&), SeatRemoved)
    )

    Weak<GskDisplayManager>         fManager;
    Unique<GskEventQueue>           fEventQueue;
    std::list<Handle<GskMonitor>>   fMonitors;
    std::list<Handle<GskSeat>>      fSeats;
    std::unordered_map<Handle<GskSeat>, sigc::connection>
                                    fSeatSlotConnections;
    Handle<GskClipboard>            fClipboard;
    Handle<GskClipboard>            fPrimaryClipboard;
    GskDisplayPrivate               fPrivateData;
    std::unordered_map<Handle<GskDevice>, std::list<DeviceGrabInfo*>>
                                    fDeviceGrabs;
    std::unordered_map<Handle<GskDevice>, PointerSurfaceInfo*>
                                    fPointersInfo;
    bool                            fClosed;
    uint32_t                        fDClickTime;        /* Maximum time between clicks in msecs */
    uint32_t                        fDClickDistance;    /* Maximum distance between clicks in pixels */
    uint32_t                        fLastEventTime;
};

GSK_NAMESPACE_END
#endif //COCOA_GSKDISPLAY_H
