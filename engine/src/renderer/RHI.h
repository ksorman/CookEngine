#ifndef RHI_H
#define RHI_H

#include "RHIBuffer.h"
#include "VmaUsage.h"
#include "CommandPool.h"
#include <cstdint>

namespace CookEngine {
class RHI
{
  public:
    RHI(VmaAllocator& vmaAllocator, VkDevice& device, uint32_t graphicQuequeIndex) : m_vmaAllocator(vmaAllocator), m_device(device), m_commandPool(device, graphicQuequeIndex) {}

    RHIBuffer CreateBuffer(VkDeviceSize size,
      VkBufferUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VmaAllocationCreateFlags vmaFlags);

    void DestroyBuffer(RHIBuffer& buffer);
    
    VmaAllocator& GetAllocator();
    CommandPool& GetCommandPool();
  private:
    VmaAllocator& m_vmaAllocator;
    VkDevice& m_device;
    CommandPool m_commandPool;
};
}// namespace CookEngine

#endif