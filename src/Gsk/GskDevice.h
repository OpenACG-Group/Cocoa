#ifndef COCOA_GSKDEVICE_H
#define COCOA_GSKDEVICE_H

#include <list>
#include <vector>

#include "Core/Errors.h"
#include "Core/EnumClassBitfield.h"
#include "Gsk/Gsk.h"
#include "Gsk/GskEnumerations.h"
GSK_NAMESPACE_BEGIN

enum class GskInputSource
{
    kMouse,
    kPen,
    kKeyboard,
    kTouchscreen,
    kTouchpad,
    kTrackPoint,
    kTabletPen,
    kUnknown
};

class GskDisplay;
class GskSeat;
class GskSurface;
class GskCursor;

class GskDevice : public std::enable_shared_from_this<GskDevice>
{
public:
    struct AxisInfo
    {
        GskAxisUse use;
        double minAxis;
        double maxAxis;
        double minValue;
        double maxValue;
        double resolution;
    };

    explicit GskDevice(const Weak<GskDisplay>& display);
    virtual ~GskDevice() = default;

    g_nodiscard g_inline const std::string& getName() const {
        return fName;
    }

    g_nodiscard g_inline const std::string& getVendorId() const {
        return fVendorId;
    }

    g_nodiscard g_inline const std::string& getProductId() const {
        return fProductId;
    }

    g_nodiscard g_inline Handle<GskDisplay> getDisplay() const {
        CHECK(!fDisplay.expired());
        return fDisplay.lock();
    }

    g_nodiscard g_inline Handle<GskSeat> getSeat() const {
        CHECK(!fSeat.expired());
        return fSeat.lock();
    }

    g_nodiscard g_inline GskInputSource getSource() const {
        return fSource;
    }

    g_nodiscard g_inline bool hasCursor() const {
        return fHasCursor;
    }

    g_nodiscard g_inline uint32_t getNumTouches() const {
        return fNumTouches;
    }

    g_nodiscard Bitfield<GskModifierType> getModifierState() const;
    g_nodiscard bool hasBidiLayouts() const;
    g_nodiscard TextDirection getTextDirection() const;
    g_nodiscard bool getCapsLockState() const;
    g_nodiscard bool getNumLockState() const;
    g_nodiscard bool getScrollLockState() const;

    g_nodiscard Handle<GskSurface> getSurfaceAtPosition(Vec2d& pos);

    g_nodiscard g_inline uint32_t getTimestamp() const {
        return fTimestamp;
    }

    void setSeat(const Weak<GskSeat>& seat);
    void setAssociatedDevice(const Weak<GskDevice>& device);
    void resetAxes();
    uint32_t addAxis(GskAxisUse use, double minValue, double maxValue, double resolution);
    void getAxisInfo(uint32_t index, GskAxisUse& use, double& minValue, double& maxValue, double& resolution);
    bool translateSurfaceCoord(const Handle<GskSurface>& surface, uint32_t index, double value,
                               double& axisValue);
    bool translateScreenCoord(const Handle<GskSurface>& surface,
                              Vec2d surfaceRootPos,
                              Vec2d screenWH,
                              uint32_t index,
                              double value,
                              double& axisValue);
    bool translateAxis(uint32_t index, double value, double& axisValue);
    const std::list<Handle<GskDevice>>& getPhysicalDevices();
    void addPhysicalDevice(const Handle<GskDevice>& physical);
    void removePhysicalDevice(const Handle<GskDevice>& physical);
    Handle<GskSurface> surfaceAtPosition(Vec2d& pos, Bitfield<GskModifierType>& mask);
    GskGrabStatus grab(const Handle<GskSurface>& surface,
                       bool ownerEvents,
                       Bitfield<GskEventMask> eventMasks,
                       const Handle<GskCursor>& cursor,
                       uint32_t time_);
    void unGrab(uint32_t time_);
    int getNumberAxes();
    Maybe<uint32_t> getAxis(GskAxisUse use);
    GskAxisUse getAxisUse(uint32_t index);
    void setTimestamp(uint32_t timestamp);

protected:
    virtual Handle<GskSurface> onSurfaceAtPosition(Vec2d& pos, Bitfield<GskModifierType>& mask) = 0;
    virtual void setSurfaceCursor(const Handle<GskSurface>& surface,
                                  const Handle<GskCursor>& cursor) = 0;
    virtual GskGrabStatus onGrab(const Handle<GskSurface>& surface,
                                 bool ownerEvents,
                                 Bitfield<GskEventMask> eventMasks,
                                 const Handle<GskSurface>& confineTo,
                                 const Handle<GskCursor>& cursor,
                                 uint32_t time_) = 0;
    virtual void onUnGrab(uint32_t time_) = 0;

private:
    std::string             fName;
    GskInputSource          fSource;
    bool                    fHasCursor;
    Weak<GskDisplay>        fDisplay;
    Weak<GskDevice>         fAssociated;
    std::list<Handle<GskDevice>> fPhysicalDevices;
    std::vector<AxisInfo>   fAxes;
    uint32_t                fNumTouches;
    std::string             fVendorId;
    std::string             fProductId;
    Weak<GskSeat>           fSeat;
    uint32_t                fTimestamp;
};

GSK_NAMESPACE_END
#endif //COCOA_GSKDEVICE_H
