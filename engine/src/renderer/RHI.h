#ifndef RHI_H
#define RHI_H

#include "CommandPool.h"
#include "Device.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "Queue.h"
#include "RHIBuffer.h"
#include "Surface.h"
#include "VmaUsage.h"
#include "VulkanAllocator.h"

namespace CookEngine {
class RHI
{
  public:
    RHI(GLFWwindow* window);

    RHIBuffer CreateBuffer(VkDeviceSize size,
      VkBufferUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VmaAllocationCreateFlags vmaFlags);

    void DestroyBuffer(RHIBuffer& buffer);
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    VkCommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    Instance& GetInstance();
    Surface& GetSurface();
    PhysicalDevice& GetPhysicalDevice();
    Device& GetDevice();
    Queue& GetQueue();
    VmaAllocator GetAllocator();
    CommandPool& GetCommandPool();

  private:
    Instance m_instance;
    Surface m_surface;
    PhysicalDevice m_physicalDevice;
    Device m_device;
    VulkanAllocator m_vulkanAllocator;
    Queue m_queue;

    CommandPool m_commandPool;
};
}// namespace CookEngine

#endif