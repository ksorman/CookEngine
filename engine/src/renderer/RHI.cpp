#include "RHI.h"
#include "CommandPool.h"
#include "Device.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "Queue.h"
#include "RHIBuffer.h"
#include "RHIImage.h"
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

RHIImage RHI::CreateImage(uint32_t width,
  uint32_t height,
  VkFormat format,
  VkImageTiling tiling,
  VkImageUsageFlags usage,
  VkMemoryPropertyFlags properties,
  VmaAllocationCreateFlags vmaFlags)
{
    RHIImage image;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.flags = vmaFlags;
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.requiredFlags = properties;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vmaCreateImage(m_vulkanAllocator, &imageInfo, &allocInfo, &image.image, &image.imageAllocation, nullptr)
        != VK_SUCCESS) {
        spdlog::error("[Vulkan] Failed to create image!");
    }
    return image;
}

void RHI::DestroyTextureImage(RHIImage& image)
{
    vmaDestroyImage(m_vulkanAllocator, image.image, image.imageAllocation);
}

void RHI::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };
    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    EndSingleTimeCommands(commandBuffer);
}

void RHI::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;
    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (HasStencilComponent(format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
               && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
               && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask =
          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else {
        spdlog::error("[Vulkan] Unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    EndSingleTimeCommands(commandBuffer);
}

VkImageView RHI::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectMask)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectMask;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(m_device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        spdlog::error("[Vulkan] Failed to create texture image view!");
    }

    return imageView;
}

VkFormat RHI::FindSupportedFormat(const std::vector<VkFormat>& candidates,
  VkImageTiling tiling,
  VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    spdlog::error("[Vulkan] Failed to find supported format!");
    return VK_FORMAT_UNDEFINED;
}

VkFormat RHI::FindDepthFormat()
{
    return FindSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
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

bool RHI::HasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}
}// namespace CookEngine