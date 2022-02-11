#include <algorithm>

#include "Gsk/GskDevice.h"
#include "Gsk/GskKeymap.h"
#include "Gsk/GskDisplay.h"
GSK_NAMESPACE_BEGIN

GskDevice::GskDevice(const Weak<GskDisplay>& display)
    : fSource(GskInputSource::kUnknown)
    , fHasCursor(false)
    , fDisplay(display)
    , fAssociated()
    , fPhysicalDevices()
    , fAxes()
    , fNumTouches(0)
    , fVendorId()
    , fProductId()
    , fSeat()
    , fTimestamp(0)
{
}

Handle<GskSurface> GskDevice::getSurfaceAtPosition(Vec2d& pos)
{
    if (fSource == GskInputSource::kKeyboard)
        return nullptr;
    g_maybe_unused Bitfield<GskModifierType> mods;
    return this->surfaceAtPosition(pos, mods);
}

GskAxisUse GskDevice::getAxisUse(uint32_t index)
{
    if (fSource == GskInputSource::kKeyboard || index >= fAxes.size())
        return GskAxisUse::kIgnore;
    return fAxes[index].use;
}

void GskDevice::setAssociatedDevice(const Weak<GskDevice>& device)
{
    if (!fAssociated.expired() || device.expired())
        return;
    fAssociated = device;
}

const std::list<Handle<GskDevice>>& GskDevice::getPhysicalDevices()
{
    return fPhysicalDevices;
}

void GskDevice::addPhysicalDevice(const Handle<GskDevice>& physical)
{
    if (std::find(fPhysicalDevices.begin(), fPhysicalDevices.end(), physical) == fPhysicalDevices.end())
    {
        fPhysicalDevices.push_back(physical);
    }
}

void GskDevice::removePhysicalDevice(const Handle<GskDevice>& physical)
{
    auto itr = std::find(fPhysicalDevices.begin(), fPhysicalDevices.end(), physical);
    if (itr != fPhysicalDevices.end())
        fPhysicalDevices.erase(itr);
}

int GskDevice::getNumberAxes()
{
    if (fSource == GskInputSource::kKeyboard)
        return 0;
    return static_cast<int>(fAxes.size());
}

Maybe<uint32_t> GskDevice::getAxis(GskAxisUse use)
{
    if (fSource == GskInputSource::kKeyboard || fAxes.empty())
        return {};

    for (uint32_t i = 0; i < fAxes.size(); i++)
    {
        if (fAxes[i].use == use)
            return i;
    }
    return {};
}

namespace {

Bitfield<GskEventMask> getNativeGrabEventMask(Bitfield<GskEventMask> mask)
{
    using T = GskEventMask;
    return Bitfield<T>{T::kPointerMotion, T::kButtonPress, T::kButtonRelease,
                       T::kEnterNotify, T::kLeaveNotify, T::kScroll} |
                       (mask & ~(Bitfield<T>{T::kButtonMotion, T::kButton1Motion,
                                             T::kButton2Motion, T::kButton3Motion}));
}

}

GskGrabStatus GskDevice::grab(const Handle<GskSurface>& surface,
                              bool ownerEvents,
                              Bitfield<GskEventMask> eventMasks,
                              const Handle<GskCursor>& cursor,
                              uint32_t time_)
{
    // TODO: surface and this device must belong to the same display
    if (!surface /* TODO: or surface has been destroyed */)
        return GskGrabStatus::kNotViewable;

    GskGrabStatus res = this->onGrab(surface,
                                     ownerEvents,
                                     getNativeGrabEventMask(eventMasks),
                                     nullptr,
                                     cursor,
                                     time_);
    if (res == GskGrabStatus::kSuccess)
    {
        getDisplay()->addDeviceGrab(shared_from_this(),
                                    surface,
                                    ownerEvents,
                                    eventMasks,
                                    getDisplay()->getNextSerial(),
                                    time_,
                                    false);
    }

    return res;
}

void GskDevice::unGrab(uint32_t time_)
{
    this->onUnGrab(time_);
}

void GskDevice::resetAxes()
{
    fAxes.clear();
}

uint32_t GskDevice::addAxis(GskAxisUse use, double minValue, double maxValue, double resolution)
{
    AxisInfo info{};
    info.use = use;
    info.minValue = minValue;
    info.maxValue = maxValue;
    info.resolution = resolution;

    switch (use)
    {
    case GskAxisUse::kX:
    case GskAxisUse::kY:
        info.minAxis = 0;
        info.maxAxis = 0;
        break;

    case GskAxisUse::kXTilt:
    case GskAxisUse::kYTilt:
        info.minAxis = -1;
        info.maxAxis = 1;
        break;

    default:
        info.minAxis = 0;
        info.maxAxis = 1;
        break;
    }

    fAxes.push_back(info);
    return fAxes.size() - 1;
}

void GskDevice::getAxisInfo(uint32_t index, GskAxisUse& use, double& minValue,
                            double& maxValue, double& resolution)
{
    if (index >= fAxes.size())
        return;
    use = fAxes[index].use;
    minValue = fAxes[index].minValue;
    maxValue = fAxes[index].maxValue;
    resolution = fAxes[index].resolution;
}

bool
GskDevice::translateSurfaceCoord(const Handle<GskSurface>& surface,
                                 uint32_t index,
                                 double value,
                                 double& axisValue)
{
    if (index >= fAxes.size())
        return false;

    AxisInfo& axisInfo = fAxes[index];
    AxisInfo *axisInfoX, *axisInfoY;

    if (axisInfo.use != GskAxisUse::kX && axisInfo.use != GskAxisUse::kY)
        return false;

    if (axisInfo.use == GskAxisUse::kX)
    {
        axisInfoX = &axisInfo;
        if (auto maybe = getAxis(GskAxisUse::kY))
            axisInfoY = &fAxes[*maybe];
        else
            return false;
    }
    else
    {
        axisInfoY = &axisInfo;
        if (auto maybe = getAxis(GskAxisUse::kX))
            axisInfoX = &fAxes[*maybe];
        else
            return false;
    }

    double deviceWidth = axisInfoX->maxValue - axisInfoX->minValue;
    double deviceHeight = axisInfoY->maxValue - axisInfoY->minValue;
    double xMin = axisInfoX->minValue;
    double yMin = axisInfoY->minValue;

    // TODO: surface geometry (gdkdevice.c:974)
    double surfaceWidth = 0;
    double surfaceHeight = 0;

    double xResolution = axisInfoX->resolution;
    double yResolution = axisInfoY->resolution;

    /*
   * Some drivers incorrectly report the resolution of the device
   * as zero (in particular linuxwacom < 0.5.3 with usb tablets).
   * This causes the device_aspect to become NaN and totally
   * breaks windowed mode.  If this is the case, the best we can
   * do is to assume the resolution is non-zero is equal in both
   * directions (which is true for many devices).  The absolute
   * value of the resolution doesn't matter since we only use the
   * ratio.
   */
    if (xResolution == 0 || yResolution == 0)
    {
        xResolution = 1;
        yResolution = 1;
    }

    double deviceAspect = (deviceHeight * yResolution) / (deviceWidth * xResolution);

    double xScale, yScale, xOffset, yOffset;
    if (deviceAspect * surfaceWidth >= surfaceHeight)
    {
        /* Device taller than surface */
        xScale = surfaceWidth / deviceWidth;
        yScale = (xScale * xResolution) / yResolution;
        xOffset = 0;
        yOffset = -(deviceHeight * yScale - surfaceHeight) / 2;
    }
    else
    {
        /* Surface taller than device */
        yScale = surfaceHeight / deviceHeight;
        xScale = (yScale * yResolution) / xResolution;
        yOffset = 0;
        xOffset = -(deviceWidth * xScale - surfaceWidth) / 2;
    }

    if (axisInfo.use == GskAxisUse::kX)
        axisValue = xOffset + xScale * (value - xMin);
    else
        axisValue = yOffset + yScale * (value - yMin);

    return true;
}

bool
GskDevice::translateScreenCoord(const Handle<GskSurface>& surface,
                                Vec2d surfaceRootPos,
                                Vec2d screenWH,
                                uint32_t index,
                                double value,
                                double& axisValue)
{
    if (index >= fAxes.size())
        return false;

    AxisInfo& axisInfo = fAxes[index];
    if (axisInfo.use != GskAxisUse::kX && axisInfo.use != GskAxisUse::kY)
        return false;

    double axisWidth = axisInfo.maxValue - axisInfo.minValue;
    double scale, offset;
    if (axisInfo.use == GskAxisUse::kX)
    {
        if (axisWidth > 0)
            scale = screenWH[0] / axisWidth;
        else
            scale = 1;
        offset = -surfaceRootPos[0];
    }
    else
    {
        if (axisWidth > 0)
            scale = screenWH[1] / axisWidth;
        else
            scale = 1;
        offset = -surfaceRootPos[1];
    }

    axisValue = offset + scale * (value - axisInfo.minValue);

    return true;
}

bool GskDevice::translateAxis(uint32_t index, double value, double& axisValue)
{
    if (index >= fAxes.size())
        return false;

    AxisInfo& axisInfo = fAxes[index];
    if (axisInfo.use != GskAxisUse::kX && axisInfo.use != GskAxisUse::kY)
        return false;

    double axisWidth = axisInfo.maxValue - axisInfo.minValue;

    axisValue = (axisInfo.minAxis * (value - axisInfo.minValue) +
                 axisInfo.minAxis + (axisInfo.maxValue - value)) / axisWidth;

    return true;
}

Handle<GskSurface> GskDevice::surfaceAtPosition(Vec2d& pos, Bitfield<GskModifierType>& mask)
{
    return this->onSurfaceAtPosition(pos, mask);
}

void GskDevice::setSeat(const Weak<GskSeat>& seat)
{
    fSeat = seat;
}

bool GskDevice::getCapsLockState() const
{
    Handle<GskKeymap> keymap = getDisplay()->getKeymap();

    if (fSource == GskInputSource::kKeyboard)
        return keymap->getCapsLockState();

    return false;
}

bool GskDevice::getNumLockState() const
{
    Handle<GskKeymap> keymap = getDisplay()->getKeymap();

    if (fSource == GskInputSource::kKeyboard)
        return keymap->getNumLockState();

    return false;
}

bool GskDevice::getScrollLockState() const
{
    Handle<GskKeymap> keymap = getDisplay()->getKeymap();

    if (fSource == GskInputSource::kKeyboard)
        return keymap->getScrollLockState();

    return false;
}

Bitfield<GskModifierType> GskDevice::getModifierState() const
{
    Handle<GskKeymap> keymap = getDisplay()->getKeymap();

    if (fSource == GskInputSource::kKeyboard)
        return keymap->getModifierState();

    return {};
}

TextDirection GskDevice::getTextDirection() const
{
    Handle<GskKeymap> keymap = getDisplay()->getKeymap();

    if (fSource == GskInputSource::kKeyboard)
        return keymap->getDirection();

    return TextDirection::kNeutral;
}

bool GskDevice::hasBidiLayouts() const
{
    Handle<GskKeymap> keymap = getDisplay()->getKeymap();

    if (fSource == GskInputSource::kKeyboard)
        return keymap->hasBidiLayouts();

    return false;
}

void GskDevice::setTimestamp(uint32_t timestamp)
{
    fTimestamp = timestamp;
}

GSK_NAMESPACE_END
