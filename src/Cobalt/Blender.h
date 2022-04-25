#ifndef COCOA_COBALT_BLENDER_H
#define COCOA_COBALT_BLENDER_H

#include "Cobalt/Cobalt.h"
#include "Cobalt/RenderClientObject.h"
#include "Cobalt/RenderTarget.h"

#include "include/core/SkDeferredDisplayList.h"

COBALT_NAMESPACE_BEGIN

class Surface;

/**
 * Cobalt 2D Rendering Pipeline:
 *
 * Without hardware acceleration:
 * Main Thread => SkPicture
 * -> RenderThread => SkSurface     [Rasterize]
 * -> RenderThread => Composite     [Composite]
 *
 * With hardware acceleration:
 * Main Thread => SkPicture
 * -> RenderHelperThread => SkDeferredDisplayList
 * -> RenderThread => SkSurface
 * -> RenderThread => Composite
 */

class Blender : public RenderClientObject
{
public:
    static co_sp<Blender> Make(const co_sp<Surface>& surface);

    explicit Blender(const co_sp<Surface>& surface);
    ~Blender() override;

    g_nodiscard g_inline co_sp<Surface> GetOutputSurface() const {
        return output_surface_;
    }

    g_nodiscard RenderTarget::RenderDevice GetRenderDeviceType() const;
    g_nodiscard int32_t GetWidth() const;
    g_nodiscard int32_t GetHeight() const;
    g_nodiscard SkColorInfo GetOutputColorInfo() const;

private:
    co_sp<Surface>                  output_surface_;
};

COBALT_NAMESPACE_END
#endif //COCOA_COBALT_BLENDER_H
