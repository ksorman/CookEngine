#ifndef VULKAN_ALLOCATOR_H
#define VULKAN_ALLOCATOR_H

#include "VmaUsage.h"

namespace CookEngine {
class VulkanAllocator
{
  public:
    VulkanAllocator(VkPhysicalDevice physicalDevice, VkDevice device, VkInstance instance);
    ~VulkanAllocator();

    VulkanAllocator(VulkanAllocator&) = delete;
    VulkanAllocator(VulkanAllocator&&) = delete;
    VulkanAllocator operator=(VulkanAllocator&) = delete;
    VulkanAllocator operator=(VulkanAllocator&&) = delete;

    operator VmaAllocator();

  private:
    VmaAllocator m_allocator;
};
}// namespace CookEngine

#endif