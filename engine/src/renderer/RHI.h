#ifndef RHI_H
#define RHI_H

#include "CommandPool.h"
#include "Device.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "Queue.h"
#include "RHIBuffer.h"
#include "RHIImage.h"
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

    RHIImage CreateImage(uint32_t width,
      uint32_t height,
      VkFormat format,
      VkImageTiling tiling,
      VkImageUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VmaAllocationCreateFlags vmaFlags);
    void DestroyTextureImage(RHIImage& image);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectMask);

    VkFormat
      FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat FindDepthFormat();

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
    bool HasStencilComponent(VkFormat format);

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