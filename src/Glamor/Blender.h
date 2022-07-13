#ifndef COCOA_GLAMOR_BLENDER_H
#define COCOA_GLAMOR_BLENDER_H

#include "Glamor/Glamor.h"
#include "Glamor/RenderClientObject.h"
#include "Glamor/RenderTarget.h"

class SkSurface;

GLAMOR_NAMESPACE_BEGIN

class Surface;

class FrameGeneratorBase;
class Blender;
class LayerTree;

#define CROP_BLENDER_BEGIN_FRAME    1
#define CROP_BLENDER_PAINT          2
#define CROP_BLENDER_SUBMIT         3
#define CROP_BLENDER_DISPOSE        4
#define CROP_BLENDER_SUBMIT_TO_TEXTURE_IMAGE 5
#define CROP_BLENDER_UPDATE         6

class Blender : public RenderClientObject
{
public:
    enum class FrameScheduleState
    {
        // Blender is completely idle and ready to schedule a new frame.
        kIdle,

        // There is a frame which has been begun and waiting to be submitted.
        // A new frame request has been sent to WSI layer, and we wait for WSI layer to notify
        // us when it is a good time to present a new frame.
        // The only way to change into this state is to call `Update` method.
        // If the scheduler has already been in `kPendingFrame` state, invocations of `Update`
        // have no effect.
        kPendingFrame,

        kPresented,
        kDisposed
    };

    static Shared<Blender> Make(const Shared<Surface>& surface);

    explicit Blender(const Shared<Surface>& surface);
    ~Blender() override;

    g_nodiscard g_inline Shared<Surface> GetOutputSurface() const {
        return output_surface_;
    }

    g_nodiscard g_inline const Shared<LayerTree>& GetLayerTree() const {
        return layer_tree_;
    }

    g_nodiscard RenderTarget::RenderDevice GetRenderDeviceType() const;
    g_nodiscard int32_t GetWidth() const;
    g_nodiscard int32_t GetHeight() const;
    g_nodiscard SkColorInfo GetOutputColorInfo() const;

    g_async_api void Update(const Shared<LayerTree>& layer_tree);
    g_async_api void Dispose();

private:
    void SurfaceResizeSlot(int32_t width, int32_t height);
    void SurfaceFrameSlot();

    bool                           disposed_;
    uint32_t                       surface_resize_slot_id_;
    uint32_t                       surface_frame_slot_id_;
    Shared<Surface>                output_surface_;
    Unique<FrameGeneratorBase>     frame_generator_;
    Shared<LayerTree>              layer_tree_;
    SkIRect                        current_dirty_rect_;
    FrameScheduleState             frame_schedule_state_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_BLENDER_H
