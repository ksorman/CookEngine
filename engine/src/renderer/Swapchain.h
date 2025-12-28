#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "RHI.h"
#include "VmaUsage.h"
#include <cstdint>


namespace CookEngine {

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Swapchain
{
  public:
    Swapchain(RHI& rhi, GLFWwindow* window);
    ~Swapchain();

    operator VkSwapchainKHR();

    void RecreateSwapchain(VkRenderPass renderPass);
    void Init(VkRenderPass renderPass);

    VkExtent2D GetExtent();
    VkFormat GetFrameBufferFormat();
    VkFormat GetDepthBufferFormat();
    VkFramebuffer GetFrameBuffer(uint8_t frameIndex);

  private:
    void CleanupSwapchain();

    void CreateSwapchain(GLFWwindow* window);
    void DestroySwapchain();
    void CreateImageView();
    void DestroyImageView();

    void CreateDepthBuffer();
    void DestroyDepthBuffer();
    void CreateDepthBufferView();
    void DestroyDepthBufferView();

    void CreateFramebuffers(VkRenderPass renderPass);
    void DestroyFramebuffers();

    SwapChainSupportDetails QuerySwapChainSupport();
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

    VkSwapchainKHR m_swapchain;
    VkExtent2D m_swapchainExtent;
    VkFormat m_swapchainFormat;
    VkFormat m_depthBufferFormat;

    RHIImage m_depthBuffer;
    VkImageView m_depthBufferView;

    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;
    std::vector<VkFramebuffer> m_swapchainFramebuffers;

    RHI& m_RHICmdList;
    GLFWwindow* m_window;
};

}// namespace CookEngine

#endif