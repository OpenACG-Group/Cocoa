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

#include <functional>

#include "Core/Journal.h"
#include "Glamor/HWComposeContext.h"
#include "Glamor/HWComposeDevice.h"
#include "Glamor/VulkanAMDAllocatorImpl.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.HWComposeDevice)

namespace {

template<typename T>
std::vector<T> vk_typed_enumerate(const std::function<void(uint32_t*, T*)>& enumerator)
{
    uint32_t count;
    enumerator(&count, nullptr);
    if (count == 0)
        return {};
    std::vector<T> elements(count);
    enumerator(&count, elements.data());
    return std::move(elements);
}

using DeviceQueueSpecifier = HWComposeDevice::DeviceQueueSpecifier;
using DeviceQueueSelector = HWComposeDevice::DeviceQueueSelector;

struct QueueMatcher
{
    constexpr static int32_t kNoGpuQueue = -1;

    DeviceQueueSpecifier specifier;
    std::function<bool(const QueueMatcher*, int32_t, const VkQueueFamilyProperties&)> matcher;
    int32_t matched_family_index = kNoGpuQueue;
    int32_t queue_count = 0;
    std::vector<float> priorities;
};

std::vector<QueueMatcher>
queue_matchers_from_specifiers(VkPhysicalDevice physical_device,
                               const std::vector<DeviceQueueSpecifier>& queue_specs)
{
    std::vector<QueueMatcher> queue_matchers;
    queue_matchers.reserve(queue_specs.size());
    for (const DeviceQueueSpecifier& specifier : queue_specs)
    {
        CHECK(specifier.count < 0 || specifier.priorities.size() == specifier.count);

#define SIMPLE_MATCHER(bit)                                                                                         \
        [](const QueueMatcher *self, int32_t idx, const VkQueueFamilyProperties& props) {                           \
            if (self->specifier.transfer && (props.queueFlags & VK_QUEUE_TRANSFER_BIT) != VK_QUEUE_TRANSFER_BIT)    \
                return false;                                                                                       \
            return (props.queueFlags & bit) == bit &&                                                               \
                   (static_cast<int32_t>(props.queueCount) >= self->specifier.count);                               \
        };

        QueueMatcher matcher;
        matcher.specifier = specifier;
        if (specifier.selector == DeviceQueueSelector::kGraphics)
        {
            matcher.matcher = SIMPLE_MATCHER(VK_QUEUE_GRAPHICS_BIT)
        }
        else if (specifier.selector == DeviceQueueSelector::kCompute)
        {
            matcher.matcher = SIMPLE_MATCHER(VK_QUEUE_COMPUTE_BIT)
        }
        else if (specifier.selector == DeviceQueueSelector::kVideoDecode)
        {
            matcher.matcher = SIMPLE_MATCHER(VK_QUEUE_VIDEO_DECODE_BIT_KHR)
        }
        else if (specifier.selector == DeviceQueueSelector::kVideoEncode)
        {
            matcher.matcher = SIMPLE_MATCHER(VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
        }
        else if (specifier.selector == DeviceQueueSelector::kGraphicsWithPresent)
        {
            if (specifier.present_surface == VK_NULL_HANDLE)
            {
                QLOG(LOG_ERROR, "Device queue specifier requires a VkSurfaceKHR for present queue");
                return {};
            }

            matcher.matcher = [physical_device](const QueueMatcher *self, int32_t idx,
                                                const VkQueueFamilyProperties& props) {
                // This selects a graphics queue with present support
                if (props.queueCount < self->specifier.count || !(props.queueFlags & VK_QUEUE_GRAPHICS_BIT))
                    return false;
                if (self->specifier.transfer && (props.queueFlags & VK_QUEUE_TRANSFER_BIT) != VK_QUEUE_TRANSFER_BIT)
                    return false;
                VkBool32 support = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(physical_device,
                                                     idx,
                                                     self->specifier.present_surface,
                                                     &support);
                return (support == VK_TRUE);
            };
        }
        queue_matchers.emplace_back(matcher);
    }

    return queue_matchers;
}

VkDevice create_vk_device(VkPhysicalDevice physical_device,
                          const std::vector<std::string>& enabled_extensions,
                          std::vector<QueueMatcher>& queue_matchers,
                          const VkPhysicalDeviceFeatures2& enabled_features)
{
    auto queue_families = vk_typed_enumerate<VkQueueFamilyProperties>(
        [physical_device](auto *c, auto *out) {
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, c, out);
        }
    );
    int32_t nb_satisfied_matchers = 0;

    for (int32_t family = 0; family < queue_families.size(); family++)
    {
        for (QueueMatcher& matcher : queue_matchers)
        {
            if (matcher.matched_family_index >= 0)
                continue;
            if (matcher.matcher(&matcher, family, queue_families[family]))
            {
                matcher.matched_family_index = family;
                nb_satisfied_matchers++;
                break;
            }
        }
        if (nb_satisfied_matchers == queue_matchers.size())
            break;
    }

    if (nb_satisfied_matchers < queue_matchers.size())
    {
        QLOG(LOG_ERROR, "Queue families provided by physical device cannot satisfy"
                        " the requirements of device creation");
        return VK_NULL_HANDLE;
    }
    CHECK(nb_satisfied_matchers == queue_matchers.size());

    // To detect duplicated family id (if more than one matcher matched the same family)
    std::map<int, bool> selected_queue_families;

    // Populate device queue create infos, which will be used
    // to create VkDevice later.
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    queue_create_infos.reserve(queue_matchers.size());
    for (QueueMatcher& matcher : queue_matchers)
    {
        CHECK(matcher.matched_family_index >= 0);
        if (selected_queue_families.contains(matcher.matched_family_index))
        {
            QLOG(LOG_ERROR, "more than one matcher matched the same queue family");
            return VK_NULL_HANDLE;
        }

        if (matcher.specifier.count < 0)
        {
            matcher.queue_count = static_cast<int32_t>(queue_families[matcher.matched_family_index].queueCount);
            matcher.priorities.resize(matcher.queue_count, 1.0f / static_cast<float>(matcher.queue_count));
        }
        else
        {
            matcher.queue_count = matcher.specifier.count;
            matcher.priorities = matcher.specifier.priorities;
        }

        queue_create_infos.push_back({
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .queueFamilyIndex = static_cast<uint32_t>(matcher.matched_family_index),
            .queueCount = static_cast<uint32_t>(matcher.queue_count),
            .pQueuePriorities = matcher.priorities.data()
        });
    }

    // Extract C-string pointers from `enabled_extensions`
    std::vector<const char*> enabled_ext_cstr;
    enabled_ext_cstr.reserve(enabled_extensions.size());
    for (const std::string& str : enabled_extensions)
        enabled_ext_cstr.push_back(str.c_str());

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = enabled_features.pNext;
    device_create_info.queueCreateInfoCount = queue_create_infos.size();
    device_create_info.pQueueCreateInfos = queue_create_infos.data();
    device_create_info.pEnabledFeatures = &enabled_features.features;
    device_create_info.enabledExtensionCount = enabled_ext_cstr.size();
    device_create_info.ppEnabledExtensionNames = enabled_ext_cstr.data();

    // TODO(sora): Enable validation layer if HWComposeContext is in debug mode
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = nullptr;

    VkDevice result;
    if (vkCreateDevice(physical_device, &device_create_info, nullptr, &result) != VK_SUCCESS)
    {
        // TODO(sora): Give a more detailed error report
        QLOG(LOG_ERROR, "Failed to create a VkDevice");
        return VK_NULL_HANDLE;
    }

    return result;
}

} // namespace anonymous

std::unique_ptr<HWComposeDevice>
HWComposeDevice::Make(const std::shared_ptr<HWComposeContext>& context,
                      const std::vector<DeviceQueueSpecifier>& queue_specs,
                      const std::vector<std::string>& extra_device_ext,
                      const VkPhysicalDeviceFeatures2& enabled_features)
{
    if (!context)
        return nullptr;

    VkPhysicalDevice physical_device = context->GetVkPhysicalDevice();

    // Queue matchers are used to select appropriate device queues
    std::vector<QueueMatcher> queue_matchers =
            queue_matchers_from_specifiers(physical_device, queue_specs);
    if (!queue_specs.empty() && queue_matchers.empty())
        return nullptr;

    // Combine extensions enabled by `HWComposeContext` and `extra_device_ext`.
    std::vector<std::string> enabled_extensions;
    auto extensions_add = [&enabled_extensions](const std::string& ext) {
        // Eliminate duplicate extensions
        auto itr = std::find(enabled_extensions.begin(),
                             enabled_extensions.end(), ext);
        if (itr != enabled_extensions.end())
            return;
        enabled_extensions.emplace_back(ext);
    };
    for (const std::string& ext : extra_device_ext)
        extensions_add(ext);

    // Now we can create Vulkan logical device
    VkDevice vk_device = create_vk_device(physical_device, enabled_extensions, queue_matchers, enabled_features);
    if (vk_device == VK_NULL_HANDLE)
        return nullptr;

    // `Matcher::matched_family_index` is modified by `create_vk_device`,
    // filled with the matched queue family index. Create `QueueMultiMap`
    // from `queue_matchers`.
    QueueMultiMap queue_multimap;
    for (const QueueMatcher& matcher : queue_matchers)
    {
        CHECK(queue_multimap.count(matcher.specifier.selector) == 0);
        DeviceQueueSelector selector = matcher.specifier.selector;

        std::vector<DeviceQueue> queues;
        for (int32_t i = 0; i < matcher.queue_count; i++)
        {
            VkQueue queue = VK_NULL_HANDLE;
            vkGetDeviceQueue(vk_device, matcher.matched_family_index, i, &queue);
            queues.emplace_back(DeviceQueue{
                .queue = queue,
                .family_index = matcher.matched_family_index
            });
        }

        queue_multimap.emplace(selector, std::move(queues));
    }

    return std::make_unique<HWComposeDevice>(context,
                                             std::move(enabled_extensions),
                                             vk_device,
                                             std::move(queue_multimap));
}

HWComposeDevice::HWComposeDevice(std::shared_ptr<HWComposeContext> context,
                                 std::vector<std::string> enabled_extensions,
                                 VkDevice vk_device,
                                 QueueMultiMap device_queue_multimap)
    : context_(std::move(context))
    , enabled_extensions_(std::move(enabled_extensions))
    , vk_device_(vk_device)
    , device_queue_multimap_(std::move(device_queue_multimap))
{
}

HWComposeDevice::~HWComposeDevice()
{
    if (vk_device_ != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(vk_device_);
        vkDestroyDevice(vk_device_, nullptr);
    }
}

std::optional<HWComposeDevice::DeviceQueue>
HWComposeDevice::GetDeviceQueue(DeviceQueueSelector selector, int32_t index) const
{
    if (device_queue_multimap_.count(selector) == 0)
        return {};
    const std::vector<DeviceQueue>& queues = device_queue_multimap_.at(selector);
    if (index < 0 || index >= queues.size())
        return {};
    return queues[index];
}

int32_t HWComposeDevice::GetDeviceQueueCount(DeviceQueueSelector selector) const
{
    if (!device_queue_multimap_.contains(selector))
        return 0;
    return static_cast<int32_t>(device_queue_multimap_.at(selector).size());
}

sk_sp<VulkanAMDAllocatorImpl>
HWComposeDevice::CreateAllocator(bool external_sync,
                                 const skgpu::VulkanExtensions *extensions)
{
    // FIXME(sora): On Linux Intel HD405 (other Intel models are not tested), we are
    //              seeing issues doing read pixels with non-coherent memory.
    //              For that model of graphics card, we should enable the option
    //              `force_coherent_host_visible_mem` as a workaround.
    sk_sp<VulkanAMDAllocatorImpl> allocator_impl = VulkanAMDAllocatorImpl::Make(
            context_->GetVkInstance(),
            context_->GetVkPhysicalDevice(),
            vk_device_,
            VK_API_VERSION_1_3,
            external_sync,
            extensions,
            /* force_coherent_host_visible_mem= */ false);

    return allocator_impl;
}

VkResult HWComposeDevice::CreateBufferSimple(VkDeviceSize size, VkBufferUsageFlags usage,
                                             VkMemoryPropertyFlags properties,
                                             VkBuffer& out_buffer, VkDeviceMemory& out_memory)
{
    VkBuffer buffer;
    VkDeviceMemory memory;

    VkBufferCreateInfo buffer_create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr
    };
    VkResult result = vkCreateBuffer(vk_device_, &buffer_create_info, nullptr, &buffer);
    if (result != VK_SUCCESS)
        return result;

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(vk_device_, buffer, &mem_reqs);

    // Find memory type index
    uint32_t memory_type_idx = std::numeric_limits<uint32_t>::max();

    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(context_->GetVkPhysicalDevice(), &mem_props);
    for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++)
    {
        if ((mem_reqs.memoryTypeBits & (1 << i)) &&
            (mem_props.memoryTypes[i].propertyFlags & properties) == properties)
        {
            memory_type_idx = i;
            break;
        }
    }
    if (memory_type_idx == std::numeric_limits<uint32_t>::max())
    {
        vkDestroyBuffer(vk_device_, buffer, nullptr);
        return VK_ERROR_UNKNOWN;
    }

    VkMemoryAllocateInfo memory_allocate_info{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = mem_reqs.size,
        .memoryTypeIndex = memory_type_idx
    };
    result = vkAllocateMemory(vk_device_, &memory_allocate_info, nullptr, &memory);
    if (result != VK_SUCCESS)
    {
        vkDestroyBuffer(vk_device_, buffer, nullptr);
        return result;
    }

    vkBindBufferMemory(vk_device_, buffer, memory, 0);
    out_buffer = buffer;
    out_memory = memory;

    return VK_SUCCESS;
}

void HWComposeDevice::Trace(Tracer *tracer) noexcept
{
    tracer->TraceResource("VkDevice",
                          TRACKABLE_TYPE_HANDLE,
                          TRACKABLE_DEVICE_GPU,
                          TRACKABLE_OWNERSHIP_STRICT_OWNED,
                          TraceIdFromPointer(vk_device_));
}

GLAMOR_NAMESPACE_END
