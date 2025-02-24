#ifndef RENDERER_H
#define RENDERER_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <array>
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "glm/glm.hpp"
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>


namespace CookEngine {

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
};

struct UniformBufferObject {
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
    void Init(GLFWwindow *window);
    void DrawFrame();
    void Deinit();

    bool& RefToBoolForResize();

  private:
    void UpdateUniformBuffer(uint32_t currentFrame);

    void CreateInstance();
    void CreatePhysicalDevice();
    void CreateLogicalDevice();
    void CreateSurface(GLFWwindow *window);
    VkFormat CreateSwapchain(GLFWwindow *window);
    void CleanupSwapchain();
    void RecreateSwapchain();
    void CreateImageView(const VkFormat& format);

    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CreateUniformBuffers();
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    
    void CreateFramebuffers();
    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline();
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
    void CreateRenderPass(const VkFormat& format);

    void CreateCommandPool();
    void CreateCommandBuffers();
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    VkCommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    void CreateDescriptorPool();
    void CreateDescriptorSets();

    void CreateSyncObjects();

    void CreateTextureImage();
    void CreateImage(uint32_t width,
      uint32_t height,
      VkFormat format,
      VkImageTiling tiling,
      VkImageUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VkImage &image,
      VkDeviceMemory &imageMemory);
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    void CreateTextureImageView();
    VkImageView CreateImageView(VkImage image, VkFormat format);

    void CreateTextureSampler();
    static std::vector<char> ReadFile(const std::string &filename);

    void DestroyInstance();
    void DestroySurface();
    void DestroyDevice();
    void DestroySwapchain();
    void DestroyImageView();
    void DestroyFramebuffers();
    void DestroyRenderPass();
    void DestroyPipelineLayout();
    void DestroyDescriptorSetLayout();
    void DestroyPipeline();
    void DestroyCommandPool();
    void DestroySyncObjects();
    void DestroyTextureImage();
    void DestroyVertexBuffer();
    void DestroyIndexBuffer();
    void DestroyUniformBuffer();
    void DestroyDescriptorPool();
    void DestroyDescriptorSets();
    void DestroyTextureImageView();
    void DestroyTextureSampler();
    QueueFamilyIndices ChooseQueue();
    SwapChainSupportDetails QuerySwapChainSupport();
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window);
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

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

    VkPipeline m_graphicsPipeline;

    VkCommandPool m_commandPool;

    std::vector<VkCommandBuffer> m_commandBuffers;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;

    bool framebufferResized = false;

    int32_t m_currentFrame;

    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vertexBufferMemory;
    VkBuffer m_indexBuffer;
    VkDeviceMemory m_indexBufferMemory;

    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets;

    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;
    std::vector<void *> m_uniformBuffersMapped;

    VkImage m_textureImage;
    VkDeviceMemory m_textureImageMemory;
    VkImageView m_textureImageView;

    VkSampler m_textureSampler;

    const int MAX_FRAMES_IN_FLIGHT = 2;
};
}// namespace CookEngine
#endif// RENDERER_H