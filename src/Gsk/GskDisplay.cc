#include "Core/Errors.h"
#include "Core/Journal.h"

#include "Gsk/GskDisplay.h"
#include "Gsk/GskDisplayManager.h"
#include "Gsk/GskEventQueue.h"
#include "Gsk/GskEvent.h"
#include "Gsk/GskDevice.h"
#include "Gsk/GskSeat.h"
GSK_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Display)

Handle<GskDisplay> GskDisplay::Make(const std::string& displayName)
{
    auto manager = GskGlobalScope::Instance()->getDisplayManager();
    CHECK(manager);
    return manager->openDisplay(displayName);
}

GskDisplay::GskDisplay()
    : fClosed(false)
    , fDClickTime(250)
    , fDClickDistance(5)
    , fLastEventTime(GSK_CURRENT_TIME)
{
}

GskDisplay::~GskDisplay()
{
    CHECK(fClosed && "Leaking: Display is not closed");
}

Handle<GskSeat> GskDisplay::getDefaultSeat()
{
    return this->onGetDefaultSeat();
}

bool GskDisplay::hasSurfaceAlphaChannel() const
{
    return fPrivateData.has_alpha_channel;
}

bool GskDisplay::hasSurfaceComposited() const
{
    return fPrivateData.has_composited;
}

bool GskDisplay::hasInputShapes() const
{
    return fPrivateData.has_input_shapes;
}

void GskDisplay::close()
{
    if (!fClosed)
    {
        for (auto& pair : fDeviceGrabs)
        {
            for (DeviceGrabInfo *pInfo : pair.second)
                delete pInfo;
        }
        fDeviceGrabs.clear();
        for (auto& pair : fPointersInfo)
            delete pair.second;
        fPointersInfo.clear();
        fManager.reset();
        for (Handle<GskMonitor>& monitor : fMonitors)
            monitor.reset();
        for (Handle<GskSeat>& seat : fSeats)
            seat.reset();
        fClipboard.reset();
        fPrimaryClipboard.reset();
        fEventQueue.reset();
        this->onDispose();
        fClosed = true;
        g_signal_emit(Closed);
    }
}

void GskDisplay::sync()
{
    this->onSync();
}

void GskDisplay::flush()
{
    this->onFlush();
}

Handle<GskMonitor> GskDisplay::getMonitorOnSurface(const Handle<GskSurface>& surface)
{
    // TODO(surface): Complete this (gdkdisplay.c:2026)
    return nullptr;
}

Handle<GskEvent> GskDisplay::getEventFromQueue()
{
    if (fEventQueue->getPauseCount() == 0)
        this->onEnqueueEvents(fEventQueue);
    return fEventQueue->popFirstEvent();
}

void GskDisplay::initialize()
{
    fEventQueue = std::make_unique<GskEventQueue>(weak_from_this());
}

void GskDisplay::generateGrabBrokenEvent(const Handle<GskSurface>& surface,
                                         const Handle<GskDevice>& device,
                                         bool implicit,
                                         const Handle<GskSurface>& grabSurface)
{
    CHECK(surface);
    Handle<GskEvent> event = std::make_shared<GskGrabBrokenEvent>(surface,
                                                                  device,
                                                                  grabSurface,
                                                                  implicit);
    fEventQueue->push(event);
}

void GskDisplay::addMonitor(const Handle<GskMonitor>& monitor)
{
    if (std::find(fMonitors.begin(), fMonitors.end(), monitor) == fMonitors.end())
        fMonitors.push_back(monitor);
    else
        QLOG(LOG_WARNING, "Monitor {} cannot be added twice", fmt::ptr(monitor.get()));
}

Handle<GskSeat> GskDisplay::onGetDefaultSeat()
{
    if (fSeats.empty())
        return nullptr;
    return *fSeats.begin();
}

GskDisplay::DeviceGrabInfo *GskDisplay::getLastDeviceGrab(const Handle<GskDevice>& device)
{
    auto itr = fDeviceGrabs.find(device);
    if (itr != fDeviceGrabs.end() && !itr->second.empty())
    {
        return itr->second.back();
    }
    return nullptr;
}

GskDisplay::DeviceGrabInfo *GskDisplay::addDeviceGrab(const Handle<GskDevice>& device,
                                                      const Handle<GskSurface>& surface,
                                                      bool ownerEvents,
                                                      Bitfield<GskEventMask> eventMask,
                                                      ulong serialStart,
                                                      uint32_t time,
                                                      bool implicit)
{
    auto *info = new DeviceGrabInfo;

    info->surface = surface;
    info->serialStart = serialStart;
    info->serialEnd = UINT64_MAX;
    info->ownerEvents = ownerEvents;
    info->eventMask = eventMask;
    info->time = time;
    info->implicit = implicit;

    if (fDeviceGrabs.count(device) == 0)
    {
        fDeviceGrabs[device] = { info };
        return info;
    }

    auto& grabs = fDeviceGrabs[device];
    auto itr = grabs.begin();
    for (; itr != grabs.end(); itr++)
    {
        if (info->serialStart < (*itr)->serialStart)
            break;
    }
    grabs.insert(itr, info);

    itr = std::find(grabs.begin(), grabs.end(), info);
    if (itr != grabs.begin())
    {
        itr--;
        (*itr)->serialEnd = info->serialStart;
    }

    return info;
}

Handle<GskSurface> GskDisplay::getCurrentToplevel(const Handle<GskDevice>& device,
                                                  Vec2i& pos,
                                                  Bitfield<GskModifierType>& state)
{
    Vec2d posf;
    Handle<GskSurface> pointerSurface = device->surfaceAtPosition(posf, state);
    if (pointerSurface && false /* TODO: && surface not destroyed (gdkdisplay.c:605) */)
        pointerSurface = nullptr;

    pos[0] = static_cast<Vec2i::value_type>(std::round(posf[0]));
    pos[1] = static_cast<Vec2i::value_type>(std::round(posf[1]));

    return pointerSurface;
}

void GskDisplay::switchToPointerGrab(const Handle<GskDevice>& device,
                                     DeviceGrabInfo *grab,
                                     DeviceGrabInfo *lastGrab,
                                     uint32_t time, ulong serial)
{
    auto oldGrabs = std::move(fDeviceGrabs[device]);
    fDeviceGrabs.erase(device);
    PointerSurfaceInfo *info = getPointerInfo(device);

    if (grab)
    {
        if (!grab->implicit)
        {
            if (!grab->ownerEvents && info->surfaceUnderPointer != grab->surface)
                setSurfaceUnderPointer(device, nullptr);
        }
        grab->activated = true;
    }

    Handle<GskSurface> newToplevel;
    Vec2i pos;
    Bitfield<GskModifierType> state;
    if (lastGrab)
    {
        if (grab == nullptr || (!lastGrab->ownerEvents && grab->ownerEvents))
        {
            newToplevel = getCurrentToplevel(device, pos, state);
            if (newToplevel)
            {
                info->toplevelPos = pos;
                info->state = state;
            }
        }
        if (grab == nullptr)
            setSurfaceUnderPointer(device, newToplevel);
    }
    fDeviceGrabs[device] = std::move(oldGrabs);
}

void GskDisplay::updateLastEvent(const Handle<GskEvent>& event)
{
    if (event->getTime() != GSK_CURRENT_TIME)
        fLastEventTime = event->getTime();
}

void GskDisplay::deviceGrabUpdate(const Handle<GskDevice>& device, uint32_t currentSerial)
{
    if (fDeviceGrabs.count(device) == 0)
        return;
    uint32_t time = fLastEventTime;

    /* We do a copy-construction */
    std::list<DeviceGrabInfo*> grabs = fDeviceGrabs[device];

    DeviceGrabInfo *current, *next;
    for (auto itr = grabs.begin(); itr != grabs.end(); itr++)
    {
        current = *itr;

        if (current->serialStart > currentSerial)
            return;
        if (current->serialEnd > currentSerial)
        {
            if (!current->activated)
            {
                if (device->getSource() != GskInputSource::kKeyboard)
                    switchToPointerGrab(device, current, nullptr, time, currentSerial);
            }
            break;
        }

        next = nullptr;
        if (++itr != grabs.end())
        {
            next = *itr;
            if (next->serialStart > currentSerial)
                next = nullptr;
        }
        itr--;

        if ((next == nullptr && current->implicitUnGrab) ||
            (next != nullptr && current->surface != next->surface))
        {
            generateGrabBrokenEvent(current->surface, device, current->implicit,
                                    next ? next->surface : nullptr);
        }

        if (device->getSource() != GskInputSource::kKeyboard)
            switchToPointerGrab(device, next, current, time, currentSerial);

        delete current;
    }
}

Maybe<std::list<GskDisplay::DeviceGrabInfo*>::iterator>
GskDisplay::findDeviceGrab(const Handle<GskDevice>& device, ulong serial)
{
    if (fDeviceGrabs.count(device) == 0)
        return {};
    auto& l = fDeviceGrabs[device];
    for (auto itr = l.begin(); itr != l.end(); itr++)
    {
        if (serial >= (*itr)->serialStart && serial < (*itr)->serialEnd)
            return itr;
    }
    return {};
}

GskDisplay::DeviceGrabInfo *GskDisplay::hasDeviceGrab(const Handle<GskDevice>& device, ulong serial)
{
    auto maybe = findDeviceGrab(device, serial);
    return (maybe ? *(*maybe) : nullptr);
}

bool GskDisplay::endDeviceGrab(const Handle<GskDevice>& device,
                               ulong serial,
                               const Handle<GskSurface>& ifChild,
                               bool implicit)
{
    std::list<DeviceGrabInfo*>::iterator itr;
    if (auto maybe = findDeviceGrab(device, serial))
        itr = *maybe;
    else
        return false;
    DeviceGrabInfo *info = *itr;
    if (ifChild == nullptr || ifChild == info->surface)
    {
        info->serialEnd = serial;
        info->implicitUnGrab = implicit;
        return (++itr == fDeviceGrabs[device].end());
    }
    return false;
}

GskDisplay::PointerSurfaceInfo *GskDisplay::getPointerInfo(Handle<GskDevice> device)
{
    if (device)
    {
        Handle<GskSeat> seat = device->getSeat();
        if (device == seat->getKeyboard())
            device = seat->getPointer();
    }

    if (UNLIKELY(!device))
        return nullptr;

    PointerSurfaceInfo *info;
    if (fPointersInfo.count(device) == 0)
    {
        info = new PointerSurfaceInfo{};
        fPointersInfo[device] = info;
    }
    else
        info = fPointersInfo[device];

    return info;
}

void GskDisplay::pointerInfoForeach(const PointerSurfaceInfoForeachPfn& pfn)
{
    for (const auto& pair : fPointersInfo)
        pfn(shared_from_this(), pair.first, pair.second);
}

bool GskDisplay::deviceGrabInfo(const Handle<GskDevice>& device,
                                Handle<GskSurface>& outSurface,
                                bool& ownerEvents)
{
    DeviceGrabInfo *info = getLastDeviceGrab(device);
    if (info)
    {
        outSurface = info->surface;
        ownerEvents = info->ownerEvents;
        return true;
    }
    return false;
}

bool GskDisplay::deviceIsGrabbed(const Handle<GskDevice>& device)
{
    DeviceGrabInfo *info = getLastDeviceGrab(device);
    return (info && !info->implicit);
}

Handle<GskClipboard> GskDisplay::getClipboard()
{
    // TODO(clipboard): create clipboard if not (gdkdisplay.c:1025)
    return fClipboard;
}

Handle<GskClipboard> GskDisplay::getPrimaryClipboard()
{
    // TODO(clipboard): create clipboard if not (gdkdisplay.c:1047)
    return fPrimaryClipboard;
}

void GskDisplay::setInputShapes(bool inputShapes)
{
    fPrivateData.has_input_shapes = inputShapes;
}

ulong GskDisplay::getNextSerial()
{
    return this->onGetNextSerial();
}

void GskDisplay::pauseEvents()
{
    fEventQueue->increasePauseCount();
}

void GskDisplay::unpauseEvents()
{
    fEventQueue->decreasePauseCount();
}

Handle<GskSurface> GskDisplay::createSurface(SurfaceType type,
                                             const Handle<GskSurface>& parent,
                                             const Vec2i& pos,
                                             const Vec2i& size)
{
    return this->onCreateSurface(type, parent, pos, size);
}

Handle<GskKeymap> GskDisplay::getKeymap()
{
    return this->onGetKeymap();
}

void GskDisplay::setComposited(bool composited)
{
    fPrivateData.has_composited = composited;
}

void GskDisplay::setRGBA(bool rgba)
{
    fPrivateData.has_alpha_channel = rgba;
}

void GskDisplay::addSeat(const Handle<GskSeat>& seat)
{
    if (std::find(fSeats.begin(), fSeats.end(), seat) == fSeats.end())
    {
        fSeats.push_back(seat);
        fSeatSlotConnections[seat] = seat->signalDeviceRemoved()
                .connect([this] (const Handle<GskDevice>& device) {
            if (fDeviceGrabs.count(device))
            {
                for (DeviceGrabInfo *pInfo: fDeviceGrabs[device])
                    delete pInfo;
                fDeviceGrabs.erase(device);
            }

            if (fPointersInfo.count(device))
            {
                delete fPointersInfo[device];
                fPointersInfo.erase(device);
            }
        });
        g_signal_emit(SeatAdded, seat);
    }
    else
        QLOG(LOG_WARNING, "Seat {} cannot be added twice", fmt::ptr(seat.get()));
}

void GskDisplay::removeSeat(const Handle<GskSeat>& seat)
{
    if (std::find(fSeats.begin(), fSeats.end(), seat) != fSeats.end())
    {
        fSeats.remove(seat);
        fSeatSlotConnections[seat].disconnect();
        fSeatSlotConnections.erase(seat);
        g_signal_emit(SeatRemoved, seat);
    }
}

void GskDisplay::setCursorTheme(const std::string& name, int size)
{
    this->onSetCursorTheme(name, size);
}

void GskDisplay::setDClickTime(uint32_t msec)
{
    fDClickTime = msec;
}

void GskDisplay::setDClickDistance(uint32_t distance)
{
    fDClickDistance = distance;
}

void GskDisplay::setSurfaceUnderPointer(const Handle<GskDevice>& device,
                                        const Handle<GskSurface>& surface)
{
    // TODO(surface): Complete this.
}

void GskDisplay::removeMonitor(const Handle<GskMonitor>& monitor)
{
    fMonitors.remove(monitor);
}

GSK_NAMESPACE_END
