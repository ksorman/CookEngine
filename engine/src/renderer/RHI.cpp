#include "RHI.h"
#include "CommandPool.h"
#include "Device.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "Queue.h"
#include "RHIBuffer.h"
#include "Surface.h"
#include "VmaUsage.h"
#include "spdlog/spdlog.h"

namespace CookEngine {

RHI::RHI(GLFWwindow* window)
  : m_instance(), m_surface(m_instance, window), m_physicalDevice(m_instance), m_device(m_physicalDevice, m_surface),
    m_vulkanAllocator(m_physicalDevice, m_device, m_instance), m_queue(m_physicalDevice, m_surface, m_device),
    m_commandPool(m_device, m_queue.GetGraphicQueueIndex())
{}

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

    if (vmaCreateBuffer(m_vulkanAllocator, &bufferInfo, &allocInfo, &buffer.buffer, &buffer.bufferAllocation, nullptr)
        != VK_SUCCESS) {
        spdlog::error("[Vulkan] Failed to create buffer!");
    }
    return buffer;
}

void RHI::DestroyBuffer(RHIBuffer& buffer)
{
    // TODO [k.samokhvalov] It is tmp fix
    vkDeviceWaitIdle(m_device);
    vmaDestroyBuffer(m_vulkanAllocator, buffer.buffer, buffer.bufferAllocation);
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

    vkQueueSubmit(m_queue.GetGraphicQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_queue.GetGraphicQueue());

    vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
}

Instance& RHI::GetInstance()
{
    return m_instance;
}

Surface& RHI::GetSurface()
{
    return m_surface;
}

PhysicalDevice& RHI::GetPhysicalDevice()
{
    return m_physicalDevice;
}

Device& RHI::GetDevice()
{
    return m_device;
}

Queue& RHI::GetQueue()
{
    return m_queue;
}

VmaAllocator RHI::GetAllocator()
{
    return m_vulkanAllocator;
}

CommandPool& RHI::GetCommandPool()
{
    return m_commandPool;
}
}// namespace CookEngine