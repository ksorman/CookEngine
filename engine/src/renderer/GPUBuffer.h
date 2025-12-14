#ifndef GPU_BUFFER_H
#define GPU_BUFFER_H

#include "Renderer.h"
#include "VmaUsage.h"
#include <cstring>
#include <vector>

namespace CookEngine {

class GPUBuffer
{
  public:
    GPUBuffer(VmaAllocator& allocator, VkDevice& device) : m_allocator(allocator), m_device(device) {}
    ~GPUBuffer();

    template<class T> void InitBuffer(Renderer& renderer, std::vector<T>& data, VkBufferUsageFlagBits usage)
    {
        VkDeviceSize bufferSize = sizeof(data[0]) * data.size();

        GPUBuffer stagingBuffer(m_allocator, renderer.GetDevice());
        stagingBuffer.CreateBuffer(bufferSize,
          VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
          VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

        void* stagingData;
        vmaMapMemory(m_allocator, stagingBuffer.GetAllocation(), &stagingData);
        memcpy(stagingData, data.data(), bufferSize);
        vmaUnmapMemory(m_allocator, stagingBuffer.GetAllocation());

        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);

        renderer.CopyBuffer(stagingBuffer.GetBuffer(), m_buffer, bufferSize);

    }

    void CreateBuffer(VkDeviceSize size,
      VkBufferUsageFlags usage,
      VkMemoryPropertyFlags property,
      VmaAllocationCreateFlags vmaFlags);

    VkBuffer& GetBuffer();
    VmaAllocation& GetAllocation();

  private:
    void DestroyBuffer();

    bool m_initialized = false;

    VkBuffer m_buffer;
    VmaAllocation m_bufferMemory;

    VmaAllocator& m_allocator;
    VkDevice& m_device;
};

}// namespace CookEngine

#endif