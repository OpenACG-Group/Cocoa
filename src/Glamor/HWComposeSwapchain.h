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

#ifndef COCOA_GLAMOR_HWCOMPOSESWAPCHAIN_H
#define COCOA_GLAMOR_HWCOMPOSESWAPCHAIN_H

#include <vector>

#include <vulkan/vulkan.h>

#include "include/gpu/GrDirectContext.h"
#include "Glamor/Glamor.h"
#include "Glamor/GraphicsResourcesTrackable.h"
GLAMOR_NAMESPACE_BEGIN

class HWComposeContext;

class HWComposeSwapchain : public std::enable_shared_from_this<HWComposeSwapchain>,
                           public GraphicsResourcesTrackable
{
public:
    struct GpuBufferInfo
    {
        GpuBufferInfo();
        ~GpuBufferInfo();

        VkDevice                    device;
        int32_t                     buffer_index;
        VkSemaphore                 semaphore;
        bool                        acquired;
    };

    struct SwapchainDetails
    {
        VkSurfaceCapabilitiesKHR            caps;
        std::vector<VkSurfaceFormatKHR>     formats;
        std::vector<VkPresentModeKHR>       present_modes;
    };

    static Shared<HWComposeSwapchain> Make(const Shared<HWComposeContext>& context,
                                           VkSurfaceKHR surface, int32_t width, int32_t height);

    HWComposeSwapchain(Shared<HWComposeContext> ctx, VkSurfaceKHR surface);
    ~HWComposeSwapchain();

    bool Resize(int32_t width, int32_t height);
    SkSurface *NextFrame();
    void SubmitFrame();

    g_inline const sk_sp<GrDirectContext>& GetSkiaDirectContext() const {
        return skia_direct_context_;
    }

    g_nodiscard SkColorType GetImageFormat() const;
    g_nodiscard SkAlphaType GetImageAlphaFormat() const;

    void Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept override;

private:
    bool CreateOrRecreateSwapchain(int32_t width, int32_t height);
    bool CreateGpuBuffers();
    void ReleaseEntireSwapchain();

    Shared<HWComposeContext>         context_;
    VkSurfaceKHR                    vk_surface_;
    SwapchainDetails                details_;
    VkPresentModeKHR                vk_present_mode_;
    VkDevice                        vk_device_;
    VkQueue                         vk_graphics_queue_;
    VkQueue                         vk_present_queue_;
    int32_t                         graphics_queue_index_;
    int32_t                         present_queue_index_;
    VkSwapchainKHR                  vk_swapchain_;
    uint32_t                        vk_images_count_;
    VkFormat                        vk_image_format_;
    VkExtent2D                      vk_swapchain_extent_;
    VkSharingMode                   vk_images_sharing_mode_;

    sk_sp<GrDirectContext>          skia_direct_context_;
    std::vector<GpuBufferInfo>      gpu_buffers_;
    std::vector<sk_sp<SkSurface>>   skia_surfaces_;
    uint32_t                        current_buffer_idx_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_HWCOMPOSESWAPCHAIN_H
