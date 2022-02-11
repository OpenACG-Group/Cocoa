#ifndef COCOA_GSKMONITOR_H
#define COCOA_GSKMONITOR_H

#include <sigc++/sigc++.h>
#include "include/core/SkRect.h"

#include "Gsk/Gsk.h"

GSK_NAMESPACE_BEGIN

class GskDisplay;

class GskMonitor
{
public:
    friend class GskDisplay;

    enum class SubpixelLayout : uint32_t
    {
        kUnknown,
        kNone,
        kHorizontalRGB,
        kHorizontalBGR,
        kVerticalRGB,
        kVerticalBGR
    };

    explicit GskMonitor(const Weak<GskDisplay>& display);
    virtual ~GskMonitor() = default;

    g_nodiscard g_inline Handle<GskDisplay> getDisplay() const {
        return fDisplay.lock();
    }

    g_nodiscard g_inline const SkIRect& getGeometry() const {
        return fGeometry;
    }

    g_nodiscard g_inline int getWidthMM() const {
        return fWidthMM;
    }

    g_nodiscard g_inline int getHeightMM() const {
        return fHeightMM;
    }

    g_nodiscard g_inline const std::string& getManufacturer() const {
        return fManufacturer;
    }

    g_nodiscard g_inline const std::string& getModel() const {
        return fModel;
    }

    g_nodiscard g_inline const std::string& getConnector() const {
        return fConnector;
    }

    g_nodiscard g_inline int getScaleFactor() const {
        return fScaleFactor;
    }

    g_nodiscard g_inline int getRefreshRate() const {
        return fRefreshRate;
    }

    g_nodiscard g_inline SubpixelLayout getSubpixelLayout() const {
        return fSubpixel;
    }

    g_nodiscard g_inline bool isValid() const {
        return fValid;
    }

    g_signal_getter(ManufacturerChanged);
    g_signal_getter(ModelChanged);
    g_signal_getter(ConnectorChanged);
    g_signal_getter(GeometryChanged);
    g_signal_getter(PhysicalSizeChanged);
    g_signal_getter(ScaleFactorChanged);
    g_signal_getter(RefreshRateChanged);
    g_signal_getter(SubpixelLayoutChanged);
    g_signal_getter(Invalidate);

protected:
    void setManufacturer(const std::string& manufacturer);
    void setModel(const std::string& model);
    void setConnector(const std::string& connector);
    void setGeometry(const SkIRect& rect);
    void setPhysicalSize(int widthMM, int heightMM);
    void setScaleFactor(int factor);
    void setRefreshRate(int rate);
    void setSubpixelLayout(SubpixelLayout subpixel);
    void invalidate();

private:
    g_signal_fields(
        g_signal_signature(void(const std::string&), ManufacturerChanged)
        g_signal_signature(void(const std::string&), ModelChanged)
        g_signal_signature(void(const std::string&), ConnectorChanged)
        g_signal_signature(void(const SkIRect&), GeometryChanged)
        g_signal_signature(void(int, int), PhysicalSizeChanged)
        g_signal_signature(void(int), ScaleFactorChanged)
        g_signal_signature(void(int), RefreshRateChanged)
        g_signal_signature(void(SubpixelLayout), SubpixelLayoutChanged)
        g_signal_signature(void(), Invalidate)
    )

    Weak<GskDisplay>    fDisplay;
    std::string         fManufacturer;
    std::string         fModel;
    std::string         fConnector;
    SkIRect             fGeometry;
    int                 fWidthMM;
    int                 fHeightMM;
    int                 fScaleFactor;
    int                 fRefreshRate;
    SubpixelLayout      fSubpixel;
    bool                fValid;
};

GSK_NAMESPACE_END
#endif //COCOA_GSKMONITOR_H
