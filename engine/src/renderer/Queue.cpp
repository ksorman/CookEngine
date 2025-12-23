#include "Queue.h"
#include "spdlog/spdlog.h"

#include <set>

namespace CookEngine {
Queue::Queue(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkDevice device)
{
    m_queueFamilyIndices = ChooseQueue(physicalDevice, surface);

    vkGetDeviceQueue(device, m_queueFamilyIndices.graphicsFamily.value(), 0, &m_graphicQueue);
    vkGetDeviceQueue(device, m_queueFamilyIndices.presentFamily.value(), 0, &m_presentQueue);
}

QueueFamilyIndices Queue::ChooseQueue(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    QueueFamilyIndices indexes;

    uint32_t numQueues = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueues, nullptr);

    if (numQueues > 0) {
        std::vector<VkQueueFamilyProperties> queuesFamilies(numQueues);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueues, queuesFamilies.data());
        uint32_t i = 0;
        for (const auto& queueFamily : queuesFamilies) {
            VkBool32 presentSupport = false;
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indexes.graphicsFamily = i;
            }

            // TODO(Kostia) In original execution queue family with index 3 dosnt support Surface
            // But in RenderDoc it supports, so in RenderDoc drawing dosnt work
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
            // if (presentSupport) { indexes.presentFamily = i; }
            if (presentSupport && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indexes.presentFamily = i;
            }
            i++;
        }
    }
    return indexes;
}

std::vector<VkDeviceQueueCreateInfo> Queue::CreateQueueInfosForLogicalDevice(VkPhysicalDevice physicalDevice,
  VkSurfaceKHR surface)
{
    auto queueFamilyIndices = ChooseQueue(physicalDevice, surface);
    if (!queueFamilyIndices.isComplete()) {
        spdlog::error("[Vulkan] Choosing Queue is faild!");
        return {};
    }

    float queuePriority = 1.0f;
    std::set<uint32_t> uniqueQueueFamilies = { queueFamilyIndices.graphicsFamily.value(),
        queueFamilyIndices.presentFamily.value() };
    std::vector<VkDeviceQueueCreateInfo> queueInfos;

    for (uint32_t uniqueQueueFamilie : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueInfo{ .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = uniqueQueueFamilie,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority };

        queueInfos.push_back(queueInfo);
    }

    return queueInfos;
}

VkQueue Queue::GetGraphicQueue()
{
    return m_graphicQueue;
}

VkQueue Queue::GetPresentQueue()
{
    return m_presentQueue;
}

uint32_t Queue::GetGraphicQueueIndex()
{
    return m_queueFamilyIndices.graphicsFamily.value();
}

}// namespace CookEngine
