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

#ifndef COCOA_GLAMOR_SKIAGPUCONTEXTOWNER_H
#define COCOA_GLAMOR_SKIAGPUCONTEXTOWNER_H

#include <list>
#include <vulkan/vulkan_core.h>

#include "include/gpu/GrDirectContext.h"

#include "Glamor/Glamor.h"
#include "Glamor/GraphicsResourcesTrackable.h"
GLAMOR_NAMESPACE_BEGIN

class HWComposeDevice;

class SkiaGpuContextOwner : public GraphicsResourcesTrackable
{
public:
    SkiaGpuContextOwner();
    ~SkiaGpuContextOwner() override = default;

    g_nodiscard g_inline GrDirectContext *GetSkiaGpuContext() const {
        return direct_context_.get();
    }

    int32_t ExportSemaphoreFd(VkSemaphore semaphore);
    VkSemaphore ImportSemaphoreFromFd(int32_t fd);

    VkDevice GetVkDevice();

    void Trace(Tracer *tracer) noexcept override;

protected:
    void TakeOverSkiaGpuContext(sk_sp<GrDirectContext> direct_context);
    void DisposeSkiaGpuContext();

    /**
     * Implementors should return a non-null pointer of `HWComposeDevice`
     * which was used to create the `GrDirectContext`. Returned pointer
     * must keep valid until `DisposeSkiaGpuContext()` is called.
     * It may be called multiple times, and implementors should return
     * the same pointer for each call.
     */
    virtual HWComposeDevice *OnGetHWComposeDevice() = 0;

private:
    sk_sp<GrDirectContext>       direct_context_;
    bool                         device_support_memory_sharing_;
    bool                         device_support_semaphore_sharing_;

    PFN_vkGetSemaphoreFdKHR      pfn_vkGetSemaphoreFdKHR_;
    PFN_vkImportSemaphoreFdKHR   pfn_vkImportSemaphoreFdKHR_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_SKIAGPUCONTEXTOWNER_H
