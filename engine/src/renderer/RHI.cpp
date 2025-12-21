#include "RHI.h"
#include "CommandPool.h"
#include "RHIBuffer.h"
#include "VmaUsage.h"
#include "spdlog/spdlog.h"

namespace CookEngine {
RHI::RHI(VmaAllocator& vmaAllocator, VkDevice& device, uint32_t graphicQuequeIndex)
  : m_vmaAllocator(vmaAllocator), m_device(device), m_commandPool(device, graphicQuequeIndex)
{
    vkGetDeviceQueue(m_device, graphicQuequeIndex, 0, &m_graphicsQueue);
}


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

void RHI::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    EndSingleTimeCommands(commandBuffer);
}

VkCommandBuffer RHI::BeginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void RHI::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.commandBufferCount = 1;

    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphicsQueue);

    vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
}

VmaAllocator& RHI::GetAllocator()
{
    return m_vmaAllocator;
}

CommandPool& RHI::GetCommandPool()
{
    return m_commandPool;
}
}// namespace CookEngine