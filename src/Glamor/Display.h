#ifndef COCOA_GLAMOR_DISPLAY_H
#define COCOA_GLAMOR_DISPLAY_H

#include <list>

#include "uv.h"
#include "Glamor/Glamor.h"
#include "Glamor/RenderTarget.h"
#include "Glamor/RenderClientObject.h"

#include "include/core/SkColor.h"
GLAMOR_NAMESPACE_BEGIN

#define CROP_DISPLAY_CLOSE                      1
#define CROP_DISPLAY_CREATE_RASTER_SURFACE      2
#define CROP_DISPLAY_CREATE_HW_COMPOSE_SURFACE  3
#define CROP_DISPLAY_REQUEST_MONITOR_LIST       4

#define CRSI_DISPLAY_CLOSED     1
#define CRSI_DISPLAY_MONITOR_ADDED      2
#define CRSI_DISPLAY_MONITOR_REMOVED    3

class Surface;
class Monitor;

class Display : public RenderClientObject
{
public:
    using MonitorList = std::list<Shared<Monitor>>;

    static Shared<Display> Connect(uv_loop_t *loop, const std::string& name);

    explicit Display(uv_loop_t *eventLoop);
    ~Display() override;

    g_nodiscard g_inline uv_loop_t *GetEventLoop() const {
        return event_loop_;
    }

    g_nodiscard g_inline const std::list<Shared<Surface>>& GetSurfacesList() const {
        return surfaces_list_;
    }

    g_nodiscard g_inline virtual std::vector<SkColorType> GetRasterColorFormats() = 0;

    g_async_api void Close();

    g_async_api MonitorList RequestMonitorList();
    g_async_api Shared<Surface> CreateRasterSurface(int32_t width, int32_t height, SkColorType format);
    g_async_api Shared<Surface> CreateHWComposeSurface(int32_t width, int32_t height, SkColorType format);

    g_private_api void RemoveSurfaceFromList(const Shared<Surface>& s);

protected:
    virtual Shared<Surface> OnCreateSurface(int32_t width, int32_t height, SkColorType format,
                                            RenderTarget::RenderDevice device) = 0;

    virtual void OnDispose() = 0;

    void AppendMonitor(const Shared<Monitor>& monitor);
    bool RemoveMonitor(const Shared<Monitor>& monitor);

    uv_loop_t                          *event_loop_;
    bool                                has_disposed_;
    std::list<Shared<Monitor>>          monitors_list_;
    std::list<Shared<Surface>>          surfaces_list_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_DISPLAY_H
