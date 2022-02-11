#include "Gsk/GskMonitor.h"
GSK_NAMESPACE_BEGIN

GskMonitor::GskMonitor(const Weak<GskDisplay>& display)
    : fDisplay(display)
    , fGeometry(SkIRect::MakeEmpty())
    , fWidthMM(0)
    , fHeightMM(0)
    , fScaleFactor(1)
    , fRefreshRate(0)
    , fSubpixel(SubpixelLayout::kUnknown)
    , fValid(true)
{
}

void GskMonitor::setManufacturer(const std::string& manufacturer)
{
    if (fManufacturer == manufacturer)
        return;
    fManufacturer = manufacturer;
    g_signal_emit(ManufacturerChanged, manufacturer);
}

void GskMonitor::setModel(const std::string& model)
{
    if (fModel == model)
        return;
    fModel = model;
    g_signal_emit(ModelChanged, model);
}

void GskMonitor::setConnector(const std::string& connector)
{
    if (fConnector == connector)
        return;
    fConnector = connector;
    g_signal_emit(ConnectorChanged, connector);
}

void GskMonitor::setGeometry(const SkIRect& rect)
{
    if (fGeometry == rect)
        return;
    fGeometry = rect;
    g_signal_emit(GeometryChanged, rect);
}

void GskMonitor::setPhysicalSize(int widthMM, int heightMM)
{
    if (widthMM == fWidthMM && heightMM == fHeightMM)
        return;
    fWidthMM = widthMM;
    fHeightMM = heightMM;
    g_signal_emit(PhysicalSizeChanged, widthMM, heightMM);
}

void GskMonitor::setScaleFactor(int factor)
{
    if (fScaleFactor == factor)
        return;
    fScaleFactor = factor;
    g_signal_emit(ScaleFactorChanged, factor);
}

void GskMonitor::setRefreshRate(int rate)
{
    if (fRefreshRate == rate)
        return;
    fRefreshRate = rate;
    g_signal_emit(RefreshRateChanged, rate);
}

void GskMonitor::setSubpixelLayout(SubpixelLayout subpixel)
{
    if (fSubpixel == subpixel)
        return;
    fSubpixel = subpixel;
    g_signal_emit(SubpixelLayoutChanged, subpixel);
}

void GskMonitor::invalidate()
{
    if (fValid)
    {
        fValid = false;
        g_signal_emit(Invalidate);
    }
}

GSK_NAMESPACE_END
