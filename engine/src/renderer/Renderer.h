#ifndef RENDERER_H
#define RENDERER_H

#include <optional>
#include <string>
#include <vector>
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace CookEngine {

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Renderer
{
  public:
    void Init(GLFWwindow *window);
    void DrawFrame();
    void Deinit();

  private:
    void CreateInstance();
    void CreatePhysicalDevice();
    void CreateLogicalDevice();
    void CreateSurface(GLFWwindow *window);
    VkFormat CreateSwapchain(GLFWwindow *window);
    void CreateImageView(const VkFormat& format);
    
    void CreateFramebuffers();
    void CreateGraphicsPipeline();
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
    void CreateRenderPass(const VkFormat& format);

    void CreateCommandPool();
    void CreateCommandBuffer();
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void CreateSyncObjects();

    static std::vector<char> ReadFile(const std::string& filename);

    void DestroyInstance();
    void DestroySurface();
    void DestroyDevice();
    void DestroySwapchain();
    void DestroyImageView();
    void DestroyFramebuffers();
    void DestroyRenderPass();
    void DestroyPipelineLayout();
    void DestroyPipeline();
    void DestroyCommandPool();
    void DestroySyncObjects();
    QueueFamilyIndices ChooseQueue();
    SwapChainSupportDetails QuerySwapChainSupport();
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window);

  private:
    VkInstance m_vkInstance;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;

    VkSurfaceKHR m_surface;
    VkSwapchainKHR m_swapChain;
    VkExtent2D m_swapChainExtent;
    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;
    std::vector<VkFramebuffer> m_swapChainFramebuffers;

    VkRenderPass m_renderPass;
    VkPipelineLayout m_pipelineLayout;

    VkPipeline m_graphicsPipeline;

    VkCommandPool m_commandPool;
    VkCommandBuffer m_commandBuffer;

    VkSemaphore m_imageAvailableSemaphore;
    VkSemaphore m_renderFinishedSemaphore;
    VkFence m_inFlightFence;
};
}// namespace CookEngine
#endif// RENDERER_H