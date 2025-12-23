#include "Device.h"
#include "Queue.h"
#include "spdlog/spdlog.h"


namespace CookEngine {

Device::Device(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

    supportedFeatures.samplerAnisotropy = VK_TRUE;

    std::vector<VkDeviceQueueCreateInfo> queueInfos = Queue::CreateQueueInfosForLogicalDevice(physicalDevice, surface);

    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo deviceInfo{ .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size()),
        .pQueueCreateInfos = queueInfos.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &supportedFeatures };

    auto vkResult = vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &m_device);
    if (vkResult == VK_SUCCESS) {
        volkLoadDevice(m_device);
        spdlog::info("[Vulkan] vkDevice was created successfully");
    }
}

Device::~Device()
{
    vkDestroyDevice(m_device, nullptr);
}

Device::operator VkDevice()
{
    return m_device;
}

}// namespace CookEngine