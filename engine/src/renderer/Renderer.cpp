#include "Renderer.h"
#include "Camera.h"
#include "GPUBuffer.h"
#include "RHI.h"
#include "RHIBuffer.h"
#include "Scene.h"
#include "VmaUsage.h"
#include "glm/ext/matrix_transform.hpp"
#include "utils/GeometryPrimitives.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <set>
#include <spdlog/spdlog.h>
#include <vector>
#include <vulkan/vulkan_core.h>

#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
namespace CookEngine {

void Renderer::Init(GLFWwindow* window)
{
    m_window = window;

    spdlog::info("[Vulkan] Reneder Init");
    m_RHICmdList = std::make_unique<RHI>(m_window);
    m_shaderLoader.Init(m_RHICmdList->GetDevice());
    auto format = CreateSwapchain(m_window);
    CreateImageView(format);
    CreateRenderPass(format);
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateDepthBuffer();
    CreateDepthBufferView();
    CreateFramebuffers();
    CreateUniformBuffers();
    CreateTextureImage();
    CreateTextureImageView();
    CreateTextureSampler();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateCommandBuffers();
    CreateSyncObjects();

    m_currentFrame = 0;
}

void Renderer::DrawFrame(Scene& scene)
{
    InitScene(scene);

    vkWaitForFences(m_RHICmdList->GetDevice(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;

    VkResult result = vkAcquireNextImageKHR(
      m_RHICmdList->GetDevice(), m_swapChain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        spdlog::info("[Vulkan] Framebuffer was resized: {}", framebufferResized);
        RecreateSwapchain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        spdlog::error("[Vulkan] Failed to acquire swap chain image!");
        return;
    }

    vkResetFences(m_RHICmdList->GetDevice(), 1, &m_inFlightFences[m_currentFrame]);

    vkResetCommandBuffer(m_commandBuffers[m_currentFrame], 0);

    UpdateUniformBuffer(scene.GetCamera(), m_currentFrame);
    RecordCommandBuffer(m_commandBuffers[m_currentFrame], imageIndex, scene);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[m_currentFrame];

    VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;


    if (vkQueueSubmit(m_RHICmdList->GetQueue().GetGraphicQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
        spdlog::error("[Vulkan] Failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { m_swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;// Optional

    result = vkQueuePresentKHR(m_RHICmdList->GetQueue().GetPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        spdlog::info("[Vulkan] Framebuffer was resized: {}", framebufferResized);
        framebufferResized = false;
        RecreateSwapchain();
    } else if (result != VK_SUCCESS) {
        spdlog::error("[Vulkan] Failed to present swap chain image!");
    }
    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::Deinit()
{
    vkDeviceWaitIdle(m_RHICmdList->GetDevice());

    spdlog::info("[Vulkan] Reneder Deinit");
    DestroySyncObjects();
    DestroyDescriptorPool();
    DestroyTextureSampler();
    DestroyTextureImageView();
    DestroyTextureImage();
    DestroyUniformBuffer();
    DestroyFramebuffers();
    m_graphicsPipeline.DestroyPipeline();
    DestroyRenderPass();
    DestroyDescriptorSetLayout();
    DestroyPipelineLayout();
    DestroyDepthBufferView();
    DestroyDepthBuffer();
    DestroyImageView();
    DestroySwapchain();
    m_RHICmdList.reset();
}

bool& Renderer::RefToBoolForResize()
{
    return framebufferResized;
}

void Renderer::UpdateUniformBuffer(const Camera& camera, uint32_t currentFrame)
{
    UniformBufferObject ubo{};
    ubo.model = glm::mat4(0.01f);
    ubo.model[3][3] = 1.0f;
    ubo.model = glm::rotate(ubo.model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.model = glm::translate(ubo.model, glm::vec3(0.0f, 0.0f, 50.0f));
    ubo.view = camera.GetView();
    ubo.proj = camera.GetProj();

    memcpy(m_uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
}

SwapChainSupportDetails Renderer::QuerySwapChainSupport()
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_RHICmdList->GetPhysicalDevice(), m_RHICmdList->GetSurface(), &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_RHICmdList->GetPhysicalDevice(), m_RHICmdList->GetSurface(), &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_RHICmdList->GetPhysicalDevice(), m_RHICmdList->GetSurface(), &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_RHICmdList->GetPhysicalDevice(), m_RHICmdList->GetSurface(), &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
          m_RHICmdList->GetPhysicalDevice(), m_RHICmdList->GetSurface(), &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR Renderer::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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

VkPresentModeKHR Renderer::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Renderer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
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

uint32_t Renderer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_RHICmdList->GetPhysicalDevice(), &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    spdlog::error("[Vulkan] Failed to find suitable memory type!");
    return 0;
}

VkFormat Renderer::CreateSwapchain(GLFWwindow* window)
{
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport();

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    m_swapChainExtent = ChooseSwapExtent(swapChainSupport.capabilities, window);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    VkSwapchainCreateInfoKHR createInfo{ .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = m_RHICmdList->GetSurface(),
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = m_swapChainExtent,
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

    auto result = vkCreateSwapchainKHR(m_RHICmdList->GetDevice(), &createInfo, nullptr, &m_swapChain);

    if (result != VK_SUCCESS) {
        spdlog::error("[Vulkan] Failed to create swap chain!");
        return surfaceFormat.format;
    }

    vkGetSwapchainImagesKHR(m_RHICmdList->GetDevice(), m_swapChain, &imageCount, nullptr);
    m_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_RHICmdList->GetDevice(), m_swapChain, &imageCount, m_swapChainImages.data());

    spdlog::info("[Vulkan] Swapchain created successfully");

    return surfaceFormat.format;
}

void Renderer::CleanupSwapchain()
{
    DestroyFramebuffers();
    DestroyImageView();
    DestroySwapchain();

    DestroyDepthBufferView();
    DestroyDepthBuffer();
}

void Renderer::RecreateSwapchain()
{
    vkDeviceWaitIdle(m_RHICmdList->GetDevice());

    CleanupSwapchain();

    auto format = CreateSwapchain(m_window);
    CreateImageView(format);
    CreateDepthBuffer();
    CreateDepthBufferView();
    CreateFramebuffers();
}

void Renderer::CreateImageView(const VkFormat& format)
{
    m_swapChainImageViews.resize(m_swapChainImages.size());

    VkImageSubresourceRange subresourceRange{
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    for (size_t i = 0; i < m_swapChainImages.size(); i++) {
        m_swapChainImageViews[i] = CreateImageView(m_swapChainImages[i], format, VK_IMAGE_ASPECT_COLOR_BIT);
    }
    spdlog::info("[Vulkan] ImageView for swapchain successfully created");
}

VkFormat Renderer::FindSupportedFormat(const std::vector<VkFormat>& candidates,
  VkImageTiling tiling,
  VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_RHICmdList->GetPhysicalDevice(), format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    spdlog::error("[Vulkan] Failed to find supported format!");
    return VK_FORMAT_UNDEFINED;
}

VkFormat Renderer::FindDepthFormat()
{
    return FindSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void Renderer::CreateUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    m_uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_uniformBuffers[i] = m_RHICmdList->CreateBuffer(bufferSize,
          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
          VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
        vmaMapMemory(m_RHICmdList->GetAllocator(), m_uniformBuffers[i].bufferAllocation, &m_uniformBuffersMapped[i]);
    }
}

RHI* Renderer::GetRHICmdList()
{
    return m_RHICmdList.get();
}

void Renderer::CreateFramebuffers()
{
    m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

    for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
        VkImageView attachments[] = { m_swapChainImageViews[i], m_depthBufferView };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_swapChainExtent.width;
        framebufferInfo.height = m_swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_RHICmdList->GetDevice(), &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) == VK_SUCCESS) {
            spdlog::info("[Vulkan] Framebuffer crated successfully");
        }
    }
}

void Renderer::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_RHICmdList->GetDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
        spdlog::error("[Vulkan] Failed to create descriptor set layout!");
    }
}

void Renderer::CreateGraphicsPipeline()
{
    Shader vertShader = m_shaderLoader.LoadShader(PATH_TO_SHADER_FOLDER + L"/triangle.vert.hlsl");
    Shader fragShader = m_shaderLoader.LoadShader(PATH_TO_SHADER_FOLDER + L"/triangle.frag.hlsl");

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShader, fragShader };

    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    auto bindingDescription = Vertex::GetBindingDescription();
    auto attributeDescriptions = Vertex::GetAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapChainExtent.width);
    viewport.height = static_cast<float>(m_swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = m_swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;// Optional
    rasterizer.depthBiasClamp = 0.0f;// Optional
    rasterizer.depthBiasSlopeFactor = 0.0f;// Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;// Optional
    multisampling.pSampleMask = nullptr;// Optional
    multisampling.alphaToCoverageEnable = VK_FALSE;// Optional
    multisampling.alphaToOneEnable = VK_FALSE;// Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;// Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;// Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;// Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;// Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;// Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;// Optional

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;// Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;// Optional
    colorBlending.blendConstants[1] = 0.0f;// Optional
    colorBlending.blendConstants[2] = 0.0f;// Optional
    colorBlending.blendConstants[3] = 0.0f;// Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;// Optional
    pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;// Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0;// Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr;// Optional

    if (vkCreatePipelineLayout(m_RHICmdList->GetDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        spdlog::error("[Vulkan] Failed to create pipeline layout!");
    }

    VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
    depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilCreateInfo.depthTestEnable = VK_TRUE;
    depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
    depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilCreateInfo.stencilTestEnable = VK_FALSE;
    depthStencilCreateInfo.minDepthBounds = 0.0f;
    depthStencilCreateInfo.maxDepthBounds = 1.0f;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencilCreateInfo;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;// Optional
    pipelineInfo.basePipelineIndex = -1;// Optional

    m_graphicsPipeline.CreatePipeline(m_RHICmdList->GetDevice(), &pipelineInfo);
}

VkShaderModule Renderer::CreateShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_RHICmdList->GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        spdlog::error("[Vulkan] Failed to create shader module!");
    }

    return shaderModule;
}

void Renderer::CreateRenderPass(const VkFormat& format)
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = FindDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_RHICmdList->GetDevice(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
        spdlog::error("[Vulkan] Failed to create render pass!");
    } else {
        spdlog::info("[Vulkan] Render pass created successfully!");
    }
}

void Renderer::CreateCommandBuffers()
{
    m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_RHICmdList->GetCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    if (vkAllocateCommandBuffers(m_RHICmdList->GetDevice(), &allocInfo, m_commandBuffers.data()) == VK_SUCCESS) {
        spdlog::info("[Vulkan] Command buffers created successfully");
    }
}

void Renderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, Scene& scene)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;// Optional
    beginInfo.pInheritanceInfo = nullptr;// Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        spdlog::error("[Vulkan] Failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_swapChainExtent;

    VkClearValue clearValue[2];
    clearValue[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValue[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValue;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapChainExtent.width);
    viewport.height = static_cast<float>(m_swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = m_swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    {
        auto view = scene.GetRegistry().view<std::shared_ptr<Model>>();
        Model* model = view->begin()->get();

        VkBuffer vertexBuffers[] = { model->meshes[0].vertexBuffer->GetBuffer().buffer };
        VkDeviceSize offsetsVertex[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsetsVertex);

        vkCmdBindIndexBuffer(commandBuffer, model->meshes[0].indexBuffer->GetBuffer().buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer,
          VK_PIPELINE_BIND_POINT_GRAPHICS,
          m_pipelineLayout,
          0,
          1,
          &m_descriptorSets[m_currentFrame],
          0,
          nullptr);

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model->meshes[0].indeces.size()), 1, 0, 0, 0);
    }
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        spdlog::error("[Vulkan] Failed to record command buffer!");
    }
}

void Renderer::CreateDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(m_RHICmdList->GetDevice(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        spdlog::error("[Vulkan] Failed to create descriptor pool!");
    }
}

void Renderer::CreateDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    m_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(m_RHICmdList->GetDevice(), &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
        spdlog::error("[Vulkan] Failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_uniformBuffers[i].buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_textureImageView;
        imageInfo.sampler = m_textureSampler;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(
          m_RHICmdList->GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void Renderer::CreateSyncObjects()
{
    m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(m_RHICmdList->GetDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS
            || vkCreateSemaphore(m_RHICmdList->GetDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS
            || vkCreateFence(m_RHICmdList->GetDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
            spdlog::error("[Vulkan] Failed to create semaphores!");
        }
    }
}

void Renderer::CreateDepthBuffer()
{
    CreateImage(m_swapChainExtent.width,
      m_swapChainExtent.height,
      FindDepthFormat(),
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      m_depthBuffer,
      m_depthBufferMemory,
      0);

    TransitionImageLayout(
      m_depthBuffer, FindDepthFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void Renderer::CreateTextureImage()
{
    int texWidth;
    int texHeight;
    int texChannels;

    const std::string PATH_TO_IMAGE = PATH_TO_ASSETS_FOLDER + "/texture.jpg";
    stbi_uc* pixels = stbi_load(PATH_TO_IMAGE.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth) * texHeight * 4;

    if (!pixels) {
        spdlog::error("[Vulkan] Failed to load texture image!");
    }

    RHIBuffer stagingBuffer = m_RHICmdList->CreateBuffer(imageSize,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    void* data;

    vmaMapMemory(m_RHICmdList->GetAllocator(), stagingBuffer.bufferAllocation, &data);
    memcpy(data, pixels, (size_t)imageSize);
    vmaUnmapMemory(m_RHICmdList->GetAllocator(), stagingBuffer.bufferAllocation);

    stbi_image_free(pixels);

    CreateImage(texWidth,
      texHeight,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_SHARING_MODE_EXCLUSIVE,
      m_textureImage,
      m_textureImageMemory,
      0);

    TransitionImageLayout(
      m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    CopyBufferToImage(
      stagingBuffer.buffer, m_textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    TransitionImageLayout(m_textureImage,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vmaDestroyBuffer(m_RHICmdList->GetAllocator(), stagingBuffer.buffer, stagingBuffer.bufferAllocation);
}

bool Renderer::HasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void Renderer::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = m_RHICmdList->BeginSingleTimeCommands();
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
    m_RHICmdList->EndSingleTimeCommands(commandBuffer);
}

void Renderer::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = m_RHICmdList->BeginSingleTimeCommands();
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
    m_RHICmdList->EndSingleTimeCommands(commandBuffer);
}

void Renderer::CreateDepthBufferView()
{
    m_depthBufferView = CreateImageView(m_depthBuffer, FindDepthFormat(), VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Renderer::CreateTextureImageView()
{
    m_textureImageView = CreateImageView(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

VkImageView Renderer::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectMask)
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
    if (vkCreateImageView(m_RHICmdList->GetDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        spdlog::error("[Vulkan] Failed to create texture image view!");
    }

    return imageView;
}

void Renderer::CreateTextureSampler()
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_RHICmdList->GetPhysicalDevice(), &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(m_RHICmdList->GetDevice(), &samplerInfo, nullptr, &m_textureSampler) != VK_SUCCESS) {
        spdlog::error("[Vulkan] Failed to create texture sampler!");
    }
}

bool Renderer::CreateImage(uint32_t width,
  uint32_t height,
  VkFormat format,
  VkImageTiling tiling,
  VkImageUsageFlags usage,
  VkMemoryPropertyFlags properties,
  VkImage& image,
  VmaAllocation& imageMemory,
  VmaAllocationCreateFlags vmaFlags)
{
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

    if (vmaCreateImage(m_RHICmdList->GetAllocator(), &imageInfo, &allocInfo, &image, &imageMemory, nullptr) != VK_SUCCESS) {
        spdlog::error("[Vulkan] Failed to create image!");
        return false;
    }
    return true;
}

void Renderer::DestroySwapchain()
{
    vkDestroySwapchainKHR(m_RHICmdList->GetDevice(), m_swapChain, nullptr);
}

void Renderer::DestroyImageView()
{
    for (auto imageView : m_swapChainImageViews) {
        vkDestroyImageView(m_RHICmdList->GetDevice(), imageView, nullptr);
    }
}
void Renderer::DestroyFramebuffers()
{
    for (auto framebuffer : m_swapChainFramebuffers) {
        vkDestroyFramebuffer(m_RHICmdList->GetDevice(), framebuffer, nullptr);
    }
}

void Renderer::DestroyPipelineLayout()
{
    vkDestroyPipelineLayout(m_RHICmdList->GetDevice(), m_pipelineLayout, nullptr);
}

void Renderer::DestroyDescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(m_RHICmdList->GetDevice(), m_descriptorSetLayout, nullptr);
}

void Renderer::DestroyRenderPass()
{
    vkDestroyRenderPass(m_RHICmdList->GetDevice(), m_renderPass, nullptr);
}

void Renderer::DestroySyncObjects()
{
    for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(m_RHICmdList->GetDevice(), m_imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(m_RHICmdList->GetDevice(), m_renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(m_RHICmdList->GetDevice(), m_inFlightFences[i], nullptr);
    }
}

void Renderer::DestroyTextureImage()
{
    vmaDestroyImage(m_RHICmdList->GetAllocator(), m_textureImage, m_textureImageMemory);
}

void Renderer::DestroyUniformBuffer()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vmaUnmapMemory(m_RHICmdList->GetAllocator(), m_uniformBuffers[i].bufferAllocation);
        vmaDestroyBuffer(m_RHICmdList->GetAllocator(), m_uniformBuffers[i].buffer, m_uniformBuffers[i].bufferAllocation);
    }
}

void Renderer::DestroyDescriptorPool()
{
    vkDestroyDescriptorPool(m_RHICmdList->GetDevice(), m_descriptorPool, nullptr);
}

void Renderer::DestroyTextureImageView()
{
    vkDestroyImageView(m_RHICmdList->GetDevice(), m_textureImageView, nullptr);
}

void Renderer::DestroyDepthBufferView()
{
    vkDestroyImageView(m_RHICmdList->GetDevice(), m_depthBufferView, nullptr);
}

void Renderer::DestroyTextureSampler()
{
    vkDestroySampler(m_RHICmdList->GetDevice(), m_textureSampler, nullptr);
}

void Renderer::DestroyDepthBuffer()
{
    vmaDestroyImage(m_RHICmdList->GetAllocator(), m_depthBuffer, m_depthBufferMemory);
}

void Renderer::InitScene(Scene& scene)
{
    auto view = scene.GetRegistry().view<std::shared_ptr<Model>>();
    view.each([this](std::shared_ptr<Model>& model) {
        for (auto& mesh : model->meshes) {
            if (!mesh.renderInitilized) {
                mesh.vertexBuffer = std::make_unique<GPUBuffer>(*m_RHICmdList);
                mesh.vertexBuffer->InitBuffer(*this, mesh.verteces, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

                mesh.indexBuffer = std::make_unique<GPUBuffer>(*m_RHICmdList);
                mesh.indexBuffer->InitBuffer(*this, mesh.indeces, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

                mesh.renderInitilized = true;
            }
        }
    });
}
}// namespace CookEngine
