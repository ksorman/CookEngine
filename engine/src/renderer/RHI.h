#ifndef RHI_H
#define RHI_H

#include "CommandPool.h"
#include "RHIBuffer.h"
#include "VmaUsage.h"
#include <cstdint>


namespace CookEngine {
class RHI
{
  public:
    RHI(VmaAllocator& vmaAllocator, VkDevice& device, uint32_t graphicQuequeIndex);

    RHIBuffer CreateBuffer(VkDeviceSize size,
      VkBufferUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VmaAllocationCreateFlags vmaFlags);

    void DestroyBuffer(RHIBuffer& buffer);

    VkCommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    VmaAllocator& GetAllocator();
    CommandPool& GetCommandPool();

  private:
    VmaAllocator& m_vmaAllocator;
    VkDevice& m_device;

    VkQueue m_graphicsQueue;

    CommandPool m_commandPool;
};
}// namespace CookEngine

#endif