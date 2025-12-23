#ifndef QUEUE_H
#define QUEUE_H

#include "VmaUsage.h"
#include <cstdint>
#include <optional>
#include <vector>


namespace CookEngine {
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

class Queue
{
  public:
    Queue(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkDevice device);
    static QueueFamilyIndices ChooseQueue(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    static std::vector<VkDeviceQueueCreateInfo> CreateQueueInfosForLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
  private:
    QueueFamilyIndices m_queueFamilyIndices;

    VkQueue m_graphicQueue;
    VkQueue m_presentQueue;
};
}// namespace CookEngine

#endif