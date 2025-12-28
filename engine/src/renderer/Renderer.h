#ifndef RENDERER_H
#define RENDERER_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Pipeline.h"
#include "RHIBuffer.h"
#include "RHIImage.h"
#include "ShaderLoader.h"
#include "Swapchain.h"
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

class Renderer
{
  public:
    void Init(GLFWwindow* window);
    void DrawFrame(Scene& scene);
    void Deinit();

    bool& RefToBoolForResize();

    RHI* GetRHICmdList();
  private:
    void UpdateUniformBuffer(const Camera& camera, uint32_t currentFrame);

    void CreateUniformBuffers();

    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline();
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
    void CreateRenderPass();
    
    void CreateCommandBuffers();
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, Scene& scene);

    void CreateDescriptorPool();
    void CreateDescriptorSets();

    void CreateSyncObjects();

    void CreateTextureImage();
    void CreateTextureImageView();

    void CreateTextureSampler();

    void DestroyRenderPass();
    void DestroyPipelineLayout();
    void DestroyDescriptorSetLayout();
    void DestroySyncObjects();
    void DestroyTextureImage();
    void DestroyUniformBuffer();
    void DestroyDescriptorPool();
    void DestroyTextureImageView();
    void DestroyTextureSampler();


    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void InitScene(Scene& scene);

  private:
    std::unique_ptr<Swapchain> m_swapchain;

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

    RHIImage m_textureImage;
    VkImageView m_textureImageView;

    VkSampler m_textureSampler;

    ShaderLoader m_shaderLoader;

    std::unique_ptr<RHI> m_RHICmdList;

    const int MAX_FRAMES_IN_FLIGHT = 2;

    const std::wstring PATH_TO_SHADER_FOLDER = L"./../engine/shaders";
    const std::string PATH_TO_ASSETS_FOLDER = "./../engine/assets";
};
}// namespace CookEngine
#endif// RENDERER_H
