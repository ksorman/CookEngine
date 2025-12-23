#include "VulkanAllocator.h"
#include "VmaUsage.h"
#include "spdlog/spdlog.h"

namespace CookEngine {
VulkanAllocator::VulkanAllocator(VkPhysicalDevice physicalDevice, VkDevice device, VkInstance instance)
{
    VmaVulkanFunctions vmaFunctions = {};
    vmaFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vmaFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.device = device;
    allocatorInfo.pVulkanFunctions = &vmaFunctions;
    allocatorInfo.instance = instance;
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_0;

    if (VK_SUCCESS == vmaCreateAllocator(&allocatorInfo, &m_allocator)) {
        spdlog::info("[Vulkan] VmaAllocator was created successfully");
    }
    spdlog::error("[Vulkan] VmaAllocator creating is faild!");
}

VulkanAllocator::~VulkanAllocator()
{
    vmaDestroyAllocator(m_allocator);
}
}// namespace CookEngine