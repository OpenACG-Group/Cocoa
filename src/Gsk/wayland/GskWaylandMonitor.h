#ifndef COCOA_GSKWAYLANDMONITOR_H
#define COCOA_GSKWAYLANDMONITOR_H

#include <wayland-client-protocol.h>
#include "Gsk/wayland/protos/xdg-output-unstable-v1-protocol.h"
#include "include/core/SkRect.h"

#include "Gsk/Gsk.h"
#include "Gsk/GskMonitor.h"
GSK_NAMESPACE_BEGIN

class GskWaylandMonitor : public GskMonitor
{
public:
    GskWaylandMonitor(const Weak<GskDisplay>& display,
                      uint32_t id,
                      uint32_t version,
                      wl_output *output);
    ~GskWaylandMonitor() override = default;

    void setupXDGOutputManager();

    g_nodiscard g_inline uint32_t getId() const {
        return fId;
    }

    void invalidateFromDisplay();

#define XDG_OUT_PARAM   void *data, struct zxdg_output_v1 *xdg_output
    static void XDGOutputHandleLogicalPosition(XDG_OUT_PARAM, int32_t x, int32_t y);
    static void XDGOutputHandleLogicalSize(XDG_OUT_PARAM, int32_t width, int32_t height);
    static void XDGOutputHandleDone(XDG_OUT_PARAM);
    static void XDGOutputHandleName(XDG_OUT_PARAM, const char *name);
    static void XDGOutputHandleDescription(XDG_OUT_PARAM, const char *desc);
#undef XDG_OUT_PARAM

#define OUT_PARAM       void *data, struct wl_output *output
    static void OutputHandleGeometry(OUT_PARAM,
                                     int32_t x, int32_t y,
                                     int32_t phyWidth, int32_t phyHeight,
                                     int32_t subpixel, const char *make,
                                     const char *model, int32_t transform);
    static void OutputHandleMode(OUT_PARAM,
                                 uint32_t flags,
                                 int32_t width, int32_t height,
                                 int32_t refresh);
    static void OutputHandleDone(OUT_PARAM);
    static void OutputHandleScale(OUT_PARAM, int32_t factor);
#undef OUT_PARAM

private:
    bool shouldExpectXDGOutputDone();
    bool shouldUpdateMonitor();
    void applyMonitorChange();

    uint32_t        fId;
    uint32_t        fVersion;
    wl_output      *fOutput;
    zxdg_output_v1 *fXDGOutput;
    int32_t         fX;
    int32_t         fY;
    int32_t         fWidth;
    int32_t         fHeight;
    std::string     fName;
    bool            fWlOutputDone;
    bool            fXDGOutputDone;
};

GSK_NAMESPACE_END
#endif //COCOA_GSKWAYLANDMONITOR_H
