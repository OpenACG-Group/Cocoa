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

#include "include/core/SkImage.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"

#include "Glamor/SkiaGpuContextOwner.h"
#include "Glamor/HWComposeDevice.h"
GLAMOR_NAMESPACE_BEGIN

SkiaGpuContextOwner::SkiaGpuContextOwner()
    : direct_context_(nullptr)
    , device_support_memory_sharing_(false)
    , device_support_semaphore_sharing_(false)
    , pfn_vkGetSemaphoreFdKHR_(nullptr)
    , pfn_vkImportSemaphoreFdKHR_(nullptr)
{
}

void SkiaGpuContextOwner::TakeOverSkiaGpuContext(sk_sp<GrDirectContext> context)
{
    CHECK(context && context->unique());
    direct_context_ = std::move(context);

    auto extensions = OnGetHWComposeDevice()->GetEnabledExtensions();
    VkDevice device = OnGetHWComposeDevice()->GetVkDevice();

#define DEVICE_HAS_EXT(v) \
    (std::find(extensions.begin(), extensions.end(), v) != extensions.end())

    device_support_memory_sharing_ =
            DEVICE_HAS_EXT(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME) &&
            DEVICE_HAS_EXT(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);

    device_support_semaphore_sharing_ =
            DEVICE_HAS_EXT(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME) &&
            DEVICE_HAS_EXT(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);

    if (device_support_semaphore_sharing_)
    {
        pfn_vkGetSemaphoreFdKHR_ = reinterpret_cast<PFN_vkGetSemaphoreFdKHR>(
                vkGetDeviceProcAddr(device, "vkGetSemaphoreFdKHR"));
        pfn_vkImportSemaphoreFdKHR_ = reinterpret_cast<PFN_vkImportSemaphoreFdKHR>(
                vkGetDeviceProcAddr(device, "vkImportSemaphoreFdKHR"));
    }

#undef DEVICE_HAS_EXT
}

void SkiaGpuContextOwner::DisposeSkiaGpuContext()
{
    if (!direct_context_)
        return;
    direct_context_ = nullptr;
}

VkDevice SkiaGpuContextOwner::GetVkDevice()
{
    if (!direct_context_)
        return VK_NULL_HANDLE;
    return OnGetHWComposeDevice()->GetVkDevice();
}

VkSemaphore SkiaGpuContextOwner::ImportSemaphoreFromFd(int32_t fd)
{
    if (!device_support_semaphore_sharing_ || !direct_context_)
        return VK_NULL_HANDLE;

    VkDevice device = OnGetHWComposeDevice()->GetVkDevice();

    VkSemaphore semaphore;
    VkSemaphoreCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkResult result = vkCreateSemaphore(device, &create_info, nullptr, &semaphore);
    if (result != VK_SUCCESS)
        return VK_NULL_HANDLE;

    VkImportSemaphoreFdInfoKHR import_fd_info{};
    import_fd_info.sType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR;
    import_fd_info.fd = fd;
    import_fd_info.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;
    import_fd_info.semaphore = semaphore;
    result = pfn_vkImportSemaphoreFdKHR_(device, &import_fd_info);
    if (result != VK_SUCCESS)
    {
        vkDestroySemaphore(device, semaphore, nullptr);
        return VK_NULL_HANDLE;
    }
    return semaphore;
}

int32_t SkiaGpuContextOwner::ExportSemaphoreFd(VkSemaphore semaphore)
{
    if (!device_support_semaphore_sharing_ || !direct_context_)
        return -1;

    VkDevice device = OnGetHWComposeDevice()->GetVkDevice();

    VkSemaphoreGetFdInfoKHR get_fd_info{};
    get_fd_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR;
    get_fd_info.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT;
    get_fd_info.semaphore = semaphore;
    int32_t fd;
    VkResult result = pfn_vkGetSemaphoreFdKHR_(device, &get_fd_info, &fd);
    if (result != VK_SUCCESS)
        return -1;

    return fd;
}

void SkiaGpuContextOwner::Trace(Tracer *tracer) noexcept
{
    if (!direct_context_)
        return;

    tracer->TraceResource("GrDirectContext",
                          TRACKABLE_TYPE_CLASS_OBJECT,
                          TRACKABLE_DEVICE_GPU,
                          TRACKABLE_OWNERSHIP_SHARED,
                          TraceIdFromPointer(direct_context_.get()));
}

GLAMOR_NAMESPACE_END
