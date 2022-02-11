#include "Core/Journal.h"
#include "Core/Errors.h"
#include "Gsk/wayland/GskWaylandMonitor.h"
#include "Gsk/wayland/GskWaylandDisplay.h"
#include "Gsk/wayland/GskWaylandSeat.h"
GSK_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gsk.Wayland.Monitor)
#define NO_XDG_OUTPUT_DONE_SINCE_VERSION  3
#define OUTPUT_VERSION_WITH_DONE          2

namespace {

const wl_output_listener output_listener_ = {
    GskWaylandMonitor::OutputHandleGeometry,
    GskWaylandMonitor::OutputHandleMode,
    GskWaylandMonitor::OutputHandleDone,
    GskWaylandMonitor::OutputHandleScale
};

const zxdg_output_v1_listener zxdg_output_listener_ = {
    GskWaylandMonitor::XDGOutputHandleLogicalPosition,
    GskWaylandMonitor::XDGOutputHandleLogicalSize,
    GskWaylandMonitor::XDGOutputHandleDone,
    GskWaylandMonitor::XDGOutputHandleName,
    GskWaylandMonitor::XDGOutputHandleDescription
};

} // namespace anonymous

void GskWaylandMonitor::XDGOutputHandleLogicalPosition(void *data,
                                                       g_maybe_unused zxdg_output_v1 *xdg_output,
                                                       int32_t x, int32_t y)
{
    auto *pMonitor = reinterpret_cast<GskWaylandMonitor*>(data);
    QLOG(LOG_DEBUG, "(Monitor@{}) Handle logical position from XDG Output Manager: ({}, {})",
         fmt::ptr(pMonitor), x, y);
    pMonitor->fX = x;
    pMonitor->fY = y;
}

void GskWaylandMonitor::XDGOutputHandleLogicalSize(void *data,
                                                   g_maybe_unused zxdg_output_v1 *xdg_output,
                                                   int32_t width, int32_t height)
{
    auto *pMonitor = reinterpret_cast<GskWaylandMonitor*>(data);
    QLOG(LOG_DEBUG, "(Monitor@{}) Handle logical size from XDG Output Manager: {}x{}",
         fmt::ptr(pMonitor), width, height);
    pMonitor->fWidth = width;
    pMonitor->fHeight = height;
}

void GskWaylandMonitor::XDGOutputHandleDone(void *data, g_maybe_unused zxdg_output_v1 *xdg_output)
{
    auto *pMonitor = reinterpret_cast<GskWaylandMonitor*>(data);
    QLOG(LOG_DEBUG, "(Monitor@{}) XDG Output Manager notification has done", fmt::ptr(pMonitor));
    pMonitor->fXDGOutputDone = true;

    auto pDisplay = std::dynamic_pointer_cast<GskWaylandDisplay>(pMonitor->getDisplay());
    if (pMonitor->fWlOutputDone && pMonitor->shouldExpectXDGOutputDone())
        pMonitor->applyMonitorChange();
}

void GskWaylandMonitor::XDGOutputHandleName(void *data, g_maybe_unused zxdg_output_v1 *xdg_output,
                                            const char *name)
{
    auto *pMonitor = reinterpret_cast<GskWaylandMonitor*>(data);
    QLOG(LOG_DEBUG, "(Monitor@{}) Handle name notification from XDG Output Manager: {}",
         fmt::ptr(pMonitor), name);
    pMonitor->fName = name;
}

void GskWaylandMonitor::XDGOutputHandleDescription(void *data, g_maybe_unused zxdg_output_v1 *xdg_output,
                                                   const char *desc)
{
    QLOG(LOG_DEBUG, "(Monitor@{}) Handle description notification from XDG Output Manager: {}",
         fmt::ptr(data), desc);
}

void
GskWaylandMonitor::OutputHandleGeometry(void *data, g_maybe_unused wl_output *output,
                                        int32_t x, int32_t y, int32_t phyWidth,
                                        int32_t phyHeight, int32_t subpixel,
                                        const char *make, const char *model,
                                        int32_t transform)
{
    auto *pMonitor = reinterpret_cast<GskWaylandMonitor*>(data);
    QLOG(LOG_DEBUG, "(Monitor@{}) Handle geometry notification from output", fmt::ptr(pMonitor));

    pMonitor->fX = x;
    pMonitor->fY = y;

    switch (transform)
    {
    case WL_OUTPUT_TRANSFORM_90:
    case WL_OUTPUT_TRANSFORM_270:
    case WL_OUTPUT_TRANSFORM_FLIPPED_90:
    case WL_OUTPUT_TRANSFORM_FLIPPED_270:
        pMonitor->setPhysicalSize(phyHeight, phyWidth);
        break;
    default:
        pMonitor->setPhysicalSize(phyWidth, phyHeight);
        break;
    }

    switch (subpixel)
    {
    case WL_OUTPUT_SUBPIXEL_NONE:
        pMonitor->setSubpixelLayout(SubpixelLayout::kUnknown);
        break;
    case WL_OUTPUT_SUBPIXEL_UNKNOWN:
        pMonitor->setSubpixelLayout(SubpixelLayout::kNone);
        break;
    case WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB:
        pMonitor->setSubpixelLayout(SubpixelLayout::kHorizontalRGB);
        break;
    case WL_OUTPUT_SUBPIXEL_HORIZONTAL_BGR:
        pMonitor->setSubpixelLayout(SubpixelLayout::kHorizontalBGR);
        break;
    case WL_OUTPUT_SUBPIXEL_VERTICAL_RGB:
        pMonitor->setSubpixelLayout(SubpixelLayout::kVerticalRGB);
        break;
    case WL_OUTPUT_SUBPIXEL_VERTICAL_BGR:
        pMonitor->setSubpixelLayout(SubpixelLayout::kVerticalBGR);
        break;
    default:
        QLOG(LOG_WARNING, "(Monitor@{}) Invalid subpixel value from Wayland compositor",
             fmt::ptr(pMonitor));
        pMonitor->setSubpixelLayout(SubpixelLayout::kUnknown);
        break;
    }

    pMonitor->setManufacturer(make);
    pMonitor->setModel(model);

    if (pMonitor->shouldUpdateMonitor() || !pMonitor->fXDGOutput)
        pMonitor->applyMonitorChange();

    if (pMonitor->shouldUpdateMonitor())
        std::dynamic_pointer_cast<GskWaylandDisplay>(pMonitor->getDisplay())->updateScale();
}

void GskWaylandMonitor::OutputHandleDone(void *data, g_maybe_unused wl_output *output)
{
    auto *pMonitor = reinterpret_cast<GskWaylandMonitor*>(data);
    QLOG(LOG_DEBUG, "(Monitor@{}) Output notification has done", fmt::ptr(pMonitor));
    pMonitor->fWlOutputDone = true;

    if (!pMonitor->shouldExpectXDGOutputDone() || pMonitor->fXDGOutputDone)
        pMonitor->applyMonitorChange();
}

void GskWaylandMonitor::OutputHandleScale(void *data, g_maybe_unused wl_output *output, int32_t factor)
{
    auto *pMonitor = reinterpret_cast<GskWaylandMonitor*>(data);

    QLOG(LOG_DEBUG, "(Monitor@{}) Handle scale factor notification from output: factor={}",
         fmt::ptr(pMonitor), factor);

    SkIRect prevRect = pMonitor->getGeometry();
    int prevFactor = pMonitor->getScaleFactor();
    pMonitor->setScaleFactor(factor);

    if (pMonitor->fXDGOutput)
        return;

    pMonitor->fWidth = (prevRect.width() * prevFactor) / factor;
    pMonitor->fHeight = (prevRect.height() * prevFactor) / factor;
    if (pMonitor->shouldUpdateMonitor())
        pMonitor->applyMonitorChange();
}

void
GskWaylandMonitor::OutputHandleMode(void *data, g_maybe_unused wl_output *output,
                                    g_maybe_unused uint32_t flags,
                                    int32_t width, int32_t height, int32_t refresh)
{
    auto *pMonitor = reinterpret_cast<GskWaylandMonitor*>(data);
    QLOG(LOG_DEBUG, "(Monitor@{}) Handle mode notification from output: {}x{}@{}Hz",
         fmt::ptr(pMonitor), width, height, refresh / 1000.0);

    int scale = pMonitor->getScaleFactor();
    pMonitor->fWidth = width / scale;
    pMonitor->fHeight = height / scale;
    pMonitor->setRefreshRate(refresh);

    if (pMonitor->shouldUpdateMonitor() || !pMonitor->fXDGOutput)
        pMonitor->applyMonitorChange();
}

GskWaylandMonitor::GskWaylandMonitor(const Weak<GskDisplay>& display,
                                     uint32_t id,
                                     uint32_t version,
                                     wl_output *output)
    : GskMonitor(display)
    , fId(id)
    , fVersion(version)
    , fOutput(output)
    , fXDGOutput(nullptr)
    , fX(0)
    , fY(0)
    , fWidth(0)
    , fHeight(0)
    , fWlOutputDone(false)
    , fXDGOutputDone(false)
{
    wl_output_add_listener(output, &output_listener_, this);

    auto pDisplay = std::dynamic_pointer_cast<GskWaylandDisplay>(this->getDisplay());
    if (pDisplay->getGlobalsSet()->outputManager)
        setupXDGOutputManager();

    QLOG(LOG_DEBUG, "(Monitor@{}) New monitor detected, id={}", fmt::ptr(this), fId);
}

void GskWaylandMonitor::setupXDGOutputManager()
{
    auto pDisplay = std::dynamic_pointer_cast<GskWaylandDisplay>(getDisplay());
    fXDGOutput = zxdg_output_manager_v1_get_xdg_output(pDisplay->getGlobalsSet()->outputManager,
                                                       fOutput);
    zxdg_output_v1_add_listener(fXDGOutput, &zxdg_output_listener_, this);
}

bool GskWaylandMonitor::shouldExpectXDGOutputDone()
{
    auto pDisplay = std::dynamic_pointer_cast<GskWaylandDisplay>(this->getDisplay());
    uint32_t version = pDisplay->getGlobalsSet()->outputManagerVersion;
    return (fXDGOutput && version < NO_XDG_OUTPUT_DONE_SINCE_VERSION);
}

void GskWaylandMonitor::applyMonitorChange()
{
    GskMonitor::setGeometry(SkIRect::MakeXYWH(fX, fY, fWidth, fHeight));
    GskMonitor::setConnector(fName);
    fWlOutputDone = false;
    fXDGOutputDone = false;
    std::dynamic_pointer_cast<GskWaylandDisplay>(getDisplay())->updateScale();
}

bool GskWaylandMonitor::shouldUpdateMonitor()
{
    return (getGeometry().width() != 0 && fVersion < OUTPUT_VERSION_WITH_DONE);
}

void GskWaylandMonitor::invalidateFromDisplay()
{
    GskMonitor::invalidate();
}

GSK_NAMESPACE_END
