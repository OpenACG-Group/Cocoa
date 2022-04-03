#ifndef COCOA_COBALT_DISPLAY_H
#define COCOA_COBALT_DISPLAY_H

#include <list>

#include "uv.h"
#include "Cobalt/Cobalt.h"
#include "Cobalt/RenderTarget.h"
#include "Cobalt/RenderClientObject.h"

#include "include/core/SkColor.h"
COBALT_NAMESPACE_BEGIN

#define CROP_DISPLAY_CLOSE                      1
#define CROP_DISPLAY_CREATE_RASTER_SURFACE      2
#define CROP_DISPLAY_CREATE_HW_COMPOSE_SURFACE  3

#define CRSI_DISPLAY_CLOSED     1

class Surface;

class Display : public RenderClientObject
{
public:
    static co_sp<Display> Connect(uv_loop_t *loop, const std::string& name);

    explicit Display(uv_loop_t *eventLoop);
    ~Display() override;

    g_nodiscard g_inline uv_loop_t *GetEventLoop() const {
        return event_loop_;
    }

    g_nodiscard g_inline const std::list<co_sp<Surface>>& GetSurfacesList() const {
        return surfaces_list_;
    }

    g_nodiscard g_inline virtual std::vector<SkColorType> GetRasterColorFormats() = 0;

    g_async_api void Close();

    g_async_api co_sp<Surface> CreateRasterSurface(int32_t width, int32_t height, SkColorType format);
    g_async_api co_sp<Surface> CreateHWComposeSurface(int32_t width, int32_t height, SkColorType format);

    g_private_api void RemoveSurfaceFromList(const co_sp<Surface>& s);

protected:
    virtual co_sp<Surface> OnCreateSurface(int32_t width, int32_t height, SkColorType format,
                                           RenderTarget::RenderDevice device) = 0;

    virtual void OnDispose() = 0;

    uv_loop_t                          *event_loop_;
    bool                                has_disposed_;
    std::list<co_sp<Surface>>           surfaces_list_;
};

COBALT_NAMESPACE_END
#endif //COCOA_COBALT_DISPLAY_H
