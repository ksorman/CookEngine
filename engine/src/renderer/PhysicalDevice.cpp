#include "PhysicalDevice.h"
#include "spdlog/spdlog.h"

namespace CookEngine {
PhysicalDevice::PhysicalDevice(VkInstance instance)
{
    uint32_t numDevices = 0;
    auto vkResult = vkEnumeratePhysicalDevices(instance, &numDevices, nullptr);
    if (vkResult != VK_SUCCESS) {
        spdlog::error("[Vulkan] Devices not found");
    }

    if (numDevices > 0) {
        std::vector<VkPhysicalDevice> devices(numDevices);
        auto vkResult = vkEnumeratePhysicalDevices(instance, &numDevices, devices.data());
        if (vkResult == VK_SUCCESS) {
            for (const auto& device : devices) {
                VkPhysicalDeviceProperties property;
                vkGetPhysicalDeviceProperties(device, &property);
                spdlog::info("[Vulkan] Device: {}", property.deviceName);
            }
        }

        m_device = devices[0];
    }
}

PhysicalDevice::operator VkPhysicalDevice()
{
    return m_device;
}
}// namespace CookEngine