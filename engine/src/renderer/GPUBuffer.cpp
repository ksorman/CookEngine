#include "GPUBuffer.h"
#include "VmaUsage.h"
#include <spdlog/spdlog.h>

namespace CookEngine {

GPUBuffer::~GPUBuffer()
{
    DestroyBuffer();
}

VkBuffer& GPUBuffer::GetBuffer() {
    return m_buffer;
}

VmaAllocation& GPUBuffer::GetAllocation()
{
    return m_bufferMemory;
}

void GPUBuffer::CreateBuffer(VkDeviceSize size,
  VkBufferUsageFlags usage,
  VkMemoryPropertyFlags property,
  VmaAllocationCreateFlags vmaFlags)
{
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.flags = vmaFlags;
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.requiredFlags = property;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &m_buffer, &m_bufferMemory, nullptr) != VK_SUCCESS) {
        spdlog::error("[Vulkan] Failed to create buffer!");
    }
}

void GPUBuffer::DestroyBuffer()
{
    vmaDestroyBuffer(m_allocator, m_buffer, m_bufferMemory);
}
}// namespace CookEngine