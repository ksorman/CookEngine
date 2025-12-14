#ifndef RHI_H
#define RHI_H

#include "RHIBuffer.h"
#include "VmaUsage.h"

namespace CookEngine {
class RHI
{
  public:
    RHI(VmaAllocator& vmaAllocator, VkDevice& device) : m_vmaAllocator(vmaAllocator), m_device(device) {}

    RHIBuffer CreateBuffer(VkDeviceSize size,
      VkBufferUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VmaAllocationCreateFlags vmaFlags);

    void DestroyBuffer(RHIBuffer& buffer);
    
    VmaAllocator& GetAllocator();
  private:
    VmaAllocator& m_vmaAllocator;
    VkDevice& m_device;
};
}// namespace CookEngine

#endif