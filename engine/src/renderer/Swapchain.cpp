#include "Swapchain.h"
#include "GLFW/glfw3.h"
#include "RHI.h"
#include "spdlog/spdlog.h"
#include <algorithm>

namespace CookEngine {

Swapchain::Swapchain(RHI& rhi, GLFWwindow* window) : m_RHICmdList(rhi), m_window(window)
{
    CreateSwapchain(m_window);
}

void Swapchain::CreateSwapchain(GLFWwindow* window)
{
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport();

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    m_swapchainExtent = ChooseSwapExtent(swapChainSupport.capabilities, window);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    VkSwapchainCreateInfoKHR createInfo{ .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = m_RHICmdList.GetSurface(),
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = m_swapchainExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,// Optional
        .pQueueFamilyIndices = nullptr,// Optional
        .preTransform = swapChainSupport.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE };

    auto result = vkCreateSwapchainKHR(m_RHICmdList.GetDevice(), &createInfo, nullptr, &m_swapchain);

    if (result != VK_SUCCESS) {
        spdlog::error("[Vulkan] Failed to create swap chain!");
        m_swapchainFormat = surfaceFormat.format;
    }

    vkGetSwapchainImagesKHR(m_RHICmdList.GetDevice(), m_swapchain, &imageCount, nullptr);
    m_swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_RHICmdList.GetDevice(), m_swapchain, &imageCount, m_swapchainImages.data());

    spdlog::info("[Vulkan] Swapchain created successfully");

    m_swapchainFormat = surfaceFormat.format;
    m_depthBufferFormat = m_RHICmdList.FindDepthFormat();
}

Swapchain::~Swapchain()
{
    CleanupSwapchain();
}

Swapchain::operator VkSwapchainKHR()
{
    return m_swapchain;
}

VkExtent2D Swapchain::GetExtent()
{
    return m_swapchainExtent;
}

VkFormat Swapchain::GetFrameBufferFormat()
{
    return m_swapchainFormat;
}
VkFormat Swapchain::GetDepthBufferFormat()
{
    return m_depthBufferFormat;
}

VkFramebuffer Swapchain::GetFrameBuffer(uint8_t frameIndex)
{
    return m_swapchainFramebuffers[frameIndex];
}

void Swapchain::DestroySwapchain()
{
    vkDestroySwapchainKHR(m_RHICmdList.GetDevice(), m_swapchain, nullptr);
}

void Swapchain::CreateImageView()
{
    m_swapchainImageViews.resize(m_swapchainImages.size());

    VkImageSubresourceRange subresourceRange{
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    for (size_t i = 0; i < m_swapchainImages.size(); i++) {
        m_swapchainImageViews[i] =
          m_RHICmdList.CreateImageView(m_swapchainImages[i], m_swapchainFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
    spdlog::info("[Vulkan] ImageView for swapchain successfully created");
}

void Swapchain::DestroyImageView()
{
    for (auto imageView : m_swapchainImageViews) {
        vkDestroyImageView(m_RHICmdList.GetDevice(), imageView, nullptr);
    }
}

void Swapchain::CreateDepthBuffer()
{
    m_depthBufferFormat = m_RHICmdList.FindDepthFormat();

    m_depthBuffer = m_RHICmdList.CreateImage(m_swapchainExtent.width,
      m_swapchainExtent.height,
      m_depthBufferFormat,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      0);

    m_RHICmdList.TransitionImageLayout(m_depthBuffer.image,
      m_depthBufferFormat,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void Swapchain::CreateDepthBufferView()
{
    m_depthBufferView =
      m_RHICmdList.CreateImageView(m_depthBuffer.image, m_depthBufferFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Swapchain::DestroyDepthBufferView()
{
    vkDestroyImageView(m_RHICmdList.GetDevice(), m_depthBufferView, nullptr);
}

void Swapchain::CreateFramebuffers(VkRenderPass renderPass)
{
    m_swapchainFramebuffers.resize(m_swapchainImageViews.size());

    for (size_t i = 0; i < m_swapchainImageViews.size(); i++) {
        VkImageView attachments[] = { m_swapchainImageViews[i], m_depthBufferView };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_swapchainExtent.width;
        framebufferInfo.height = m_swapchainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_RHICmdList.GetDevice(), &framebufferInfo, nullptr, &m_swapchainFramebuffers[i])
            == VK_SUCCESS) {
            spdlog::info("[Vulkan] Framebuffer crated successfully");
        }
    }
}

void Swapchain::DestroyFramebuffers()
{
    for (auto framebuffer : m_swapchainFramebuffers) {
        vkDestroyFramebuffer(m_RHICmdList.GetDevice(), framebuffer, nullptr);
    }
}

void Swapchain::DestroyDepthBuffer()
{
    m_RHICmdList.DestroyTextureImage(m_depthBuffer);
}

void Swapchain::Init(VkRenderPass renderPass)
{
    CreateDepthBuffer();
    CreateDepthBufferView();

    CreateImageView();
    
    CreateFramebuffers(renderPass);
}

void Swapchain::RecreateSwapchain(VkRenderPass renderPass)
{
    vkDeviceWaitIdle(m_RHICmdList.GetDevice());

    CleanupSwapchain();

    CreateSwapchain(m_window);
    Init(renderPass);
}

void Swapchain::CleanupSwapchain()
{
    DestroyFramebuffers();
    DestroyImageView();
    DestroySwapchain();

    DestroyDepthBufferView();
    DestroyDepthBuffer();
}

SwapChainSupportDetails Swapchain::QuerySwapChainSupport()
{
    SwapChainSupportDetails details;
    VkPhysicalDevice physicalDevice = m_RHICmdList.GetPhysicalDevice();
    VkSurfaceKHR surface = m_RHICmdList.GetSurface();

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
          physicalDevice, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR Swapchain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
            && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    spdlog::warn("[Vulkan] Needed format didnt found!");
    return availableFormats[0];
}

VkPresentModeKHR Swapchain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

    actualExtent.width =
      std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height =
      std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
}

}// namespace CookEngine