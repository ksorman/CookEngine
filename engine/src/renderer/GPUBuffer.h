#ifndef GPU_BUFFER_H
#define GPU_BUFFER_H

#include "RHI.h"
#include "Renderer.h"
#include "VmaUsage.h"
#include <cstring>
#include <vector>

namespace CookEngine {

class GPUBuffer
{
  public:
    GPUBuffer(RHI& cmdList) : m_RHICmdList(cmdList) {}
    ~GPUBuffer();

    template<class T> void InitBuffer(Renderer& renderer, std::vector<T>& data, VkBufferUsageFlagBits usage)
    {
        VkDeviceSize bufferSize = sizeof(data[0]) * data.size();

        GPUBuffer stagingBuffer(*renderer.GetRHICmdList());
        stagingBuffer.CreateBuffer(bufferSize,
          VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
          VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

        void* stagingData;
        vmaMapMemory(renderer.GetRHICmdList()->GetAllocator(), stagingBuffer.GetBuffer().bufferAllocation, &stagingData);
        memcpy(stagingData, data.data(), bufferSize);
        vmaUnmapMemory(renderer.GetRHICmdList()->GetAllocator(), stagingBuffer.GetBuffer().bufferAllocation);

        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);

        renderer.CopyBuffer(stagingBuffer.GetBuffer().buffer, m_buffer.buffer, bufferSize);

    }

    void CreateBuffer(VkDeviceSize size,
      VkBufferUsageFlags usage,
      VkMemoryPropertyFlags property,
      VmaAllocationCreateFlags vmaFlags);

    RHIBuffer& GetBuffer();

  private:
    void DestroyBuffer();

    bool m_initialized = false;

    RHIBuffer m_buffer;

    RHI& m_RHICmdList;
};

}// namespace CookEngine

#endif