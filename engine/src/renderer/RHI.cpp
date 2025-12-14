#include "RHI.h"
#include "RHIBuffer.h"
#include "VmaUsage.h"
#include "spdlog/spdlog.h"

namespace CookEngine {
RHIBuffer RHI::CreateBuffer(VkDeviceSize size,
  VkBufferUsageFlags usage,
  VkMemoryPropertyFlags properties,
  VmaAllocationCreateFlags vmaFlags)
{
    RHIBuffer buffer;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.flags = vmaFlags;
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.requiredFlags = properties;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vmaCreateBuffer(m_vmaAllocator, &bufferInfo, &allocInfo, &buffer.buffer, &buffer.bufferAllocation, nullptr)
        != VK_SUCCESS) {
        spdlog::error("[Vulkan] Failed to create buffer!");
    }
    return buffer;
}

void RHI::DestroyBuffer(RHIBuffer& buffer)
{
    // TODO [k.samokhvalov] It is tmp fix
    vkDeviceWaitIdle(m_device);
    vmaDestroyBuffer(m_vmaAllocator, buffer.buffer, buffer.bufferAllocation);
}

VmaAllocator& RHI::GetAllocator()
{
    return m_vmaAllocator;
}
}// namespace CookEngine