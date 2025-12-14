#include "GPUBuffer.h"
#include "RHIBuffer.h"
#include "VmaUsage.h"
#include <spdlog/spdlog.h>

namespace CookEngine {

GPUBuffer::~GPUBuffer()
{
    DestroyBuffer();
}

RHIBuffer& GPUBuffer::GetBuffer() {
    return m_buffer;
}

void GPUBuffer::CreateBuffer(VkDeviceSize size,
  VkBufferUsageFlags usage,
  VkMemoryPropertyFlags property,
  VmaAllocationCreateFlags vmaFlags)
{
    m_buffer = m_RHICmdList.CreateBuffer(size, usage, property, vmaFlags);
}

void GPUBuffer::DestroyBuffer()
{
    m_RHICmdList.DestroyBuffer(m_buffer);
}
}// namespace CookEngine