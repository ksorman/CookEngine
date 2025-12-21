#ifndef RENDERER_H
#define RENDERER_H

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Pipeline.h"
#include "RHIBuffer.h"
#include "ShaderLoader.h"
#include "VmaUsage.h"
#include "utils/GeometryPrimitives.h"
#include "RHI.h"

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3.h"
#include "utils/MathUsage.h"
#include <GLFW/glfw3native.h>


namespace CookEngine {

class Camera;
class Scene;

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

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
    void Init(GLFWwindow* window);
    void DrawFrame(Scene& scene);
    void Deinit();

    bool& RefToBoolForResize();

    VkDevice& GetDevice();
    RHI* GetRHICmdList();
  private:
    void UpdateUniformBuffer(const Camera& camera, uint32_t currentFrame);

    void CreateInstance();
    void CreatePhysicalDevice();
    void CreateLogicalDevice();
    bool CreateVMAAllocator();
    void CreateSurface(GLFWwindow* window);
    VkFormat CreateSwapchain(GLFWwindow* window);
    void CleanupSwapchain();
    void RecreateSwapchain();
    void CreateImageView(const VkFormat& format);

    VkFormat
      FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat FindDepthFormat();

    void CreateUniformBuffers();

    void CreateFramebuffers();
    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline();
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
    void CreateRenderPass(const VkFormat& format);
    
    void CreateCommandBuffers();
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, Scene& scene);

    void CreateDescriptorPool();
    void CreateDescriptorSets();

    void CreateSyncObjects();

    void CreateDepthBuffer();
    void CreateTextureImage();
    bool CreateImage(uint32_t width,
      uint32_t height,
      VkFormat format,
      VkImageTiling tiling,
      VkImageUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VkImage& image,
      VmaAllocation& imageMemory,
      VmaAllocationCreateFlags vmaFlags);
    bool HasStencilComponent(VkFormat format);
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    void CreateDepthBufferView();
    void CreateTextureImageView();
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectMask);

    void CreateTextureSampler();

    void DestroyInstance();
    void DestroySurface();
    void DestroyDevice();
    void DestroySwapchain();
    void DestroyImageView();
    void DestroyFramebuffers();
    void DestroyRenderPass();
    void DestroyPipelineLayout();
    void DestroyDescriptorSetLayout();
    void DestroySyncObjects();
    void DestroyTextureImage();
    void DestroyUniformBuffer();
    void DestroyDescriptorPool();
    void DestroyTextureImageView();
    void DestroyDepthBufferView();
    void DestroyTextureSampler();
    void DestroyDepthBuffer();

    void DestroyVMAAllocator();

    QueueFamilyIndices ChooseQueue();
    SwapChainSupportDetails QuerySwapChainSupport();
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void InitScene(Scene& scene);

  private:
    GLFWwindow* m_window;

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
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkPipelineLayout m_pipelineLayout;

    Pipeline m_graphicsPipeline;

    std::vector<VkCommandBuffer> m_commandBuffers;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;

    bool framebufferResized = false;

    int32_t m_currentFrame;

    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets;

    std::vector<RHIBuffer> m_uniformBuffers;
    std::vector<void*> m_uniformBuffersMapped;

    VkImage m_textureImage;
    VmaAllocation m_textureImageMemory;
    VkImageView m_textureImageView;

    VkSampler m_textureSampler;

    VkImage m_depthBuffer;
    VmaAllocation m_depthBufferMemory;
    VkImageView m_depthBufferView;

    VmaAllocator m_vmaAllocator;
    ShaderLoader m_shaderLoader;

    std::unique_ptr<RHI> m_RHICmdList;

    const int MAX_FRAMES_IN_FLIGHT = 2;

    const std::wstring PATH_TO_SHADER_FOLDER = L"./../engine/shaders";
    const std::string PATH_TO_ASSETS_FOLDER = "./../engine/assets";
};
}// namespace CookEngine
#endif// RENDERER_H
