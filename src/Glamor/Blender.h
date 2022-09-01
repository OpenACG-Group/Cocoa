/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef COCOA_GLAMOR_BLENDER_H
#define COCOA_GLAMOR_BLENDER_H

#include <optional>

#include "include/core/SkImage.h"

#include "Core/Data.h"
#include "Glamor/Glamor.h"
#include "Glamor/RenderClientObject.h"
#include "Glamor/RenderTarget.h"
#include "Glamor/Texture.h"
#include "Glamor/GraphicsResourcesTrackable.h"

class SkSurface;

GLAMOR_NAMESPACE_BEGIN

class Surface;

class Blender;
class LayerTree;
class TextureManager;

#define GLOP_BLENDER_DISPOSE                            1
#define GLOP_BLENDER_UPDATE                             2
#define GLOP_BLENDER_DELETE_TEXTURE                     3
#define GLOP_BLENDER_NEW_TEXTURE_DELETION_SUBSCRIPTION_SIGNAL   4
#define GLOP_BLENDER_CREATE_TEXTURE_FROM_IMAGE          5
#define GLOP_BLENDER_CREATE_TEXTURE_FROM_ENCODED_DATA   6
#define GLOP_BLENDER_CREATE_TEXTURE_FROM_PIXMAP         7

class Blender : public RenderClientObject,
                public GraphicsResourcesTrackable
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

    explicit Blender(const Shared<Surface>& surface,
                     Unique<TextureManager> texture_manager);
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

    using MaybeTextureId = std::optional<Texture::TextureId>;

    g_async_api MaybeTextureId CreateTextureFromImage(const sk_sp<SkImage>& image,
                                                      const std::string& annotation);

    g_async_api MaybeTextureId CreateTextureFromEncodedData(const Shared<Data>& data,
                                                            std::optional<SkAlphaType> alpha_type,
                                                            const std::string& annotation);

    g_async_api MaybeTextureId CreateTextureFromPixmap(const void *pixels,
                                                       const SkImageInfo& image_info,
                                                       const std::string& annotation);

    g_async_api bool DeleteTexture(Texture::TextureId id);

    g_async_api int32_t NewTextureDeletionSubscriptionSignal(Texture::TextureId id);

    void Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept override;

private:
    void SurfaceResizeSlot(int32_t width, int32_t height);
    void SurfaceFrameSlot();

    bool                           disposed_;
    uint32_t                       surface_resize_slot_id_;
    uint32_t                       surface_frame_slot_id_;
    Shared<Surface>                output_surface_;
    Shared<LayerTree>              layer_tree_;
    SkIRect                        current_dirty_rect_;
    FrameScheduleState             frame_schedule_state_;
    Unique<TextureManager>         texture_manager_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_BLENDER_H
