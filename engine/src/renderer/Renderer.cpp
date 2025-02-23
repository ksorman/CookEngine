#include "Renderer.h"

#include <cstdint>
#include <fstream>
#include <set>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <chrono>

namespace CookEngine {
const std::vector<Vertex> vertices = { { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
    { { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
    { { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },
    { { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } } };

const std::vector<uint16_t> indices = { 0, 1, 2, 2, 3, 0 };

void Renderer::Init(GLFWwindow *window)
{
    m_window = window;

    spdlog::info("Reneder Init");
    CreateInstance();
    CreateSurface(m_window);
    CreatePhysicalDevice();
    CreateLogicalDevice();
    auto format = CreateSwapchain(m_window);
    CreateImageView(format);
    CreateRenderPass(format);
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateCommandPool();
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateUniformBuffers();
    CreateTextureImage();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateCommandBuffers();
    CreateSyncObjects();

    m_currentFrame = 0;
}

void Renderer::DrawFrame()
{
    vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
      m_device, m_swapChain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || framebufferResized) {
        framebufferResized = false;
        RecreateSwapchain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        spdlog::error("Failed to acquire swap chain image!");
        return;
    }

    vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);

    vkResetCommandBuffer(m_commandBuffers[m_currentFrame], 0);

    UpdateUniformBuffer(m_currentFrame);
    RecordCommandBuffer(m_commandBuffers[m_currentFrame], imageIndex);

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


    if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
        spdlog::error("Failed to submit draw command buffer!");
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

    result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        RecreateSwapchain();
    } else if (result != VK_SUCCESS) {
        spdlog::error("Failed to present swap chain image!");
    }
    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::Deinit()
{
    vkDeviceWaitIdle(m_device);

    spdlog::info("Reneder Deinit");
    DestroySyncObjects();
    DestroyDescriptorSetLayout();
    DestroyDescriptorPool();
    DestroyTextureImage();
    DestroyUniformBuffer();
    DestroyIndexBuffer();
    DestroyVertexBuffer();
    DestroyCommandPool();
    DestroyFramebuffers();
    DestroyPipeline();
    DestroyRenderPass();
    DestroyDescriptorSetLayout();
    DestroyPipelineLayout();
    DestroyImageView();
    DestroySwapchain();
    DestroyDevice();
    DestroySurface();
    DestroyInstance();
}

bool &Renderer::RefToBoolForResize() { return framebufferResized; }

void Renderer::UpdateUniformBuffer(uint32_t currentFrame)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(
      glm::radians(45.0f), m_swapChainExtent.width / static_cast<float>(m_swapChainExtent.height), 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    memcpy(m_uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
}

void Renderer::CreateInstance()
{

    VkApplicationInfo appInfo{ .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "Demo",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "CookEngine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0 };

    std::vector<const char *> layers = { "VK_LAYER_KHRONOS_validation" };

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    VkInstanceCreateInfo insatnceCreateInfo{ .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = glfwExtensionCount,
        .ppEnabledExtensionNames = glfwExtensions };

    auto vkResult = vkCreateInstance(&insatnceCreateInfo, nullptr, &m_vkInstance);
    if (vkResult == VK_SUCCESS) {
        spdlog::info("Instance was created successfully");
    } else {
        spdlog::error("Instance wasn't created");
    }
}

void Renderer::CreatePhysicalDevice()
{
    uint32_t numDevices = 0;
    auto vkResult = vkEnumeratePhysicalDevices(m_vkInstance, &numDevices, nullptr);
    if (vkResult != VK_SUCCESS) { spdlog::error("Devices not found"); }

    if (numDevices > 0) {
        std::vector<VkPhysicalDevice> devices(numDevices);
        auto vkResult = vkEnumeratePhysicalDevices(m_vkInstance, &numDevices, devices.data());
        if (vkResult == VK_SUCCESS) {
            for (const auto &device : devices) {
                VkPhysicalDeviceProperties property;
                vkGetPhysicalDeviceProperties(device, &property);
                spdlog::info("Device: {}", property.deviceName);
            }
        }

        m_physicalDevice = devices[0];
    }
}

QueueFamilyIndices Renderer::ChooseQueue()
{
    QueueFamilyIndices indexes;

    uint32_t numQueues = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &numQueues, nullptr);

    if (numQueues > 0) {
        std::vector<VkQueueFamilyProperties> queuesFamilies(numQueues);
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &numQueues, queuesFamilies.data());
        uint32_t i = 0;
        for (const auto &queueFamily : queuesFamilies) {
            VkBool32 presentSupport = false;
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) { indexes.graphicsFamily = i; }

            vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &presentSupport);
            if (presentSupport) { indexes.presentFamily = i; }
            i++;
        }
    }
    return indexes;
}

SwapChainSupportDetails Renderer::QuerySwapChainSupport()
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
          m_physicalDevice, m_surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR Renderer::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for (const auto &availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
            && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    spdlog::warn("Needed format didnt found!");
    return availableFormats[0];
}

VkPresentModeKHR Renderer::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Renderer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window)
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
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    spdlog::error("Failed to find suitable memory type!");
    return 0;
}

void Renderer::CreateSurface(GLFWwindow *window)
{
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = glfwGetWin32Window(window);
    createInfo.hinstance = GetModuleHandle(nullptr);
    auto vkResult = vkCreateWin32SurfaceKHR(m_vkInstance, &createInfo, nullptr, &m_surface);
    if (vkResult != VK_SUCCESS) { spdlog::error("Creating surface failed"); }
}

VkFormat Renderer::CreateSwapchain(GLFWwindow *window)
{
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport();

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    m_swapChainExtent = ChooseSwapExtent(swapChainSupport.capabilities, window);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    VkSwapchainCreateInfoKHR createInfo{ .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = m_surface,
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

    auto result = vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain);

    if (result != VK_SUCCESS) {
        spdlog::error("Failed to create swap chain!");
        return surfaceFormat.format;
    }

    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
    m_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());

    spdlog::info("Swapchain created successfully");

    return surfaceFormat.format;
}

void Renderer::CleanupSwapchain()
{
    DestroyFramebuffers();
    DestroyImageView();
    DestroySwapchain();
}

void Renderer::RecreateSwapchain()
{
    vkDeviceWaitIdle(m_device);

    CleanupSwapchain();

    auto format = CreateSwapchain(m_window);
    CreateImageView(format);
    CreateFramebuffers();
}

void Renderer::CreateImageView(const VkFormat &format)
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
        VkImageViewCreateInfo imageViewInfo{ .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .image = m_swapChainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .components = { VK_COMPONENT_SWIZZLE_IDENTITY,
              VK_COMPONENT_SWIZZLE_IDENTITY,
              VK_COMPONENT_SWIZZLE_IDENTITY,
              VK_COMPONENT_SWIZZLE_IDENTITY },
            .subresourceRange = subresourceRange };

        if (vkCreateImageView(m_device, &imageViewInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS) {
            spdlog::error("ImageView for swapchain didnt created!");
            return;
        }
    }
    spdlog::info("ImageView for swapchain successfully created");
}

void Renderer::CreateVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      stagingBuffer,
      stagingBufferMemory);

    void *data;
    vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    CreateBuffer(bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      m_vertexBuffer,
      m_vertexBufferMemory);

    CopyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void Renderer::CreateIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      stagingBuffer,
      stagingBufferMemory);

    void *data;
    vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    CreateBuffer(bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      m_indexBuffer,
      m_indexBufferMemory);

    CopyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void Renderer::CreateUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    m_uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    m_uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        CreateBuffer(bufferSize,
          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
          m_uniformBuffers[i],
          m_uniformBuffersMemory[i]);

        vkMapMemory(m_device, m_uniformBuffersMemory[i], 0, bufferSize, 0, &m_uniformBuffersMapped[i]);
    }
}

void Renderer::CreateBuffer(VkDeviceSize size,
  VkBufferUsageFlags usage,
  VkMemoryPropertyFlags properties,
  VkBuffer &buffer,
  VkDeviceMemory &bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
}

void Renderer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    EndSingleTimeCommands(commandBuffer);
}

void Renderer::CreateFramebuffers()
{
    m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

    for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
        VkImageView attachments[] = { m_swapChainImageViews[i] };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_swapChainExtent.width;
        framebufferInfo.height = m_swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) == VK_SUCCESS) {
            spdlog::info("Framebuffer crated successfully");
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

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
        spdlog::error("Failed to create descriptor set layout!");
    }
}

void Renderer::CreateGraphicsPipeline()
{
    // TODO(Change path) Change those paths
    auto vertShaderCode = ReadFile("D:/CookEngine/engine/src/renderer/shaders/triangleVS.spv");
    auto pixelShaderCode = ReadFile("D:/CookEngine/engine/src/renderer/shaders/trianglePS.spv");

    VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
    VkShaderModule pixelShaderModule = CreateShaderModule(pixelShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShaderModule,
        .pName = "main",
        .pSpecializationInfo = nullptr };

    VkPipelineShaderStageCreateInfo pixelShaderStageInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = pixelShaderModule,
        .pName = "main",
        .pSpecializationInfo = nullptr };

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, pixelShaderStageInfo };

    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

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

    if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        spdlog::error("Failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;// Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;// Optional
    pipelineInfo.basePipelineIndex = -1;// Optional

    if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline)
        != VK_SUCCESS) {
        spdlog::error("Failed to create graphics pipeline!");
    } else {
        spdlog::info("Graphics pipeline created successfully!");
    }

    vkDestroyShaderModule(m_device, pixelShaderModule, nullptr);
    vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
}

VkShaderModule Renderer::CreateShaderModule(const std::vector<char> &code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        spdlog::error("Failed to create shader module!");
    }

    return shaderModule;
}

void Renderer::CreateRenderPass(const VkFormat &format)
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

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
        spdlog::error("Failed to create render pass!");
    }
}

void Renderer::CreateCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = ChooseQueue();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) == VK_SUCCESS) {
        spdlog::info("Command pool created successfully");
    }
}

void Renderer::CreateCommandBuffers()
{
    m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    if (vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers.data()) == VK_SUCCESS) {
        spdlog::info("Command buffers created successfully");
    }
}

void Renderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;// Optional
    beginInfo.pInheritanceInfo = nullptr;// Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        spdlog::error("Failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_swapChainExtent;

    VkClearValue clearColor = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

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

    VkBuffer vertexBuffers[] = { m_vertexBuffer };
    VkDeviceSize offsetsVertex[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsetsVertex);

    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(commandBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      m_pipelineLayout,
      0,
      1,
      &m_descriptorSets[m_currentFrame],
      0,
      nullptr);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) { spdlog::error("Failed to record command buffer!"); }
}

VkCommandBuffer Renderer::BeginSingleTimeCommands()
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

void Renderer::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphicsQueue);

    vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
}

void Renderer::CreateDescriptorPool()
{
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        spdlog::error("Failed to create descriptor pool!");
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
    if (vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
        spdlog::error("Failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr;// Optional
        descriptorWrite.pTexelBufferView = nullptr;// Optional
        vkUpdateDescriptorSets(m_device, 1, &descriptorWrite, 0, nullptr);
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
        if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS
            || vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS
            || vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
            spdlog::error("Failed to create semaphores!");
        }
    }
}

void Renderer::CreateTextureImage()
{
    int texWidth;
    int texHeight;
    int texChannels;

    //TODO(Kostia) Fix hard code
    stbi_uc* pixels = stbi_load("D:/CookEngine/engine/assets/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        spdlog::error("Failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(imageSize,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      stagingBuffer,
      stagingBufferMemory);

    void *data;
    vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_device, stagingBufferMemory);

    stbi_image_free(pixels);

    CreateImage(texWidth,
      texHeight,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_SHARING_MODE_EXCLUSIVE,
      m_textureImage,
      m_textureImageMemory);

    TransitionImageLayout(
      m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    CopyBufferToImage(stagingBuffer, m_textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    TransitionImageLayout(m_textureImage,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void Renderer::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
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
    } else {
        spdlog::error("Unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    EndSingleTimeCommands(commandBuffer);
}

void Renderer::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
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

void Renderer::CreateImage(uint32_t width,
  uint32_t height,
  VkFormat format,
  VkImageTiling tiling,
  VkImageUsageFlags usage,
  VkMemoryPropertyFlags properties,
  VkImage &image,
  VkDeviceMemory &imageMemory)
{
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

    if (vkCreateImage(m_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        spdlog::error("Failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        spdlog::error("Failed to allocate image memory!");
    }

    vkBindImageMemory(m_device, image, imageMemory, 0);
}

std::vector<char> Renderer::ReadFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        spdlog::error("Failed to open file: {}", filename);
        return {};
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

void Renderer::CreateLogicalDevice()
{

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(m_physicalDevice, &supportedFeatures);

    auto queueFamilyIndices = ChooseQueue();
    if (!queueFamilyIndices.isComplete()) {
        spdlog::error("Choosing Queue is faild!");
        return;
    }

    float queuePriority = 1.0f;
    std::set<uint32_t> uniqueQueueFamilies = { queueFamilyIndices.graphicsFamily.value(),
        queueFamilyIndices.presentFamily.value() };
    std::vector<VkDeviceQueueCreateInfo> queueInfos;

    for (uint32_t uniqueQueueFamilie : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueInfo{ .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = uniqueQueueFamilie,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority };

        queueInfos.push_back(queueInfo);
    }

    const std::vector<const char *> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo deviceInfo{ .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size()),
        .pQueueCreateInfos = queueInfos.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &supportedFeatures };

    auto vkResult = vkCreateDevice(m_physicalDevice, &deviceInfo, nullptr, &m_device);
    if (vkResult == VK_SUCCESS) { spdlog::info("vkDevice was created successfully"); }

    vkGetDeviceQueue(m_device, queueFamilyIndices.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, queueFamilyIndices.presentFamily.value(), 0, &m_presentQueue);
}

void Renderer::DestroyInstance() { vkDestroyInstance(m_vkInstance, nullptr); }

void Renderer::DestroySurface() { vkDestroySurfaceKHR(m_vkInstance, m_surface, nullptr); }

void Renderer::DestroyDevice() { vkDestroyDevice(m_device, nullptr); }

void Renderer::DestroySwapchain() { vkDestroySwapchainKHR(m_device, m_swapChain, nullptr); }

void Renderer::DestroyImageView()
{
    for (auto imageView : m_swapChainImageViews) { vkDestroyImageView(m_device, imageView, nullptr); }
}
void Renderer::DestroyFramebuffers()
{
    for (auto framebuffer : m_swapChainFramebuffers) { vkDestroyFramebuffer(m_device, framebuffer, nullptr); }
}

void Renderer::DestroyPipelineLayout() { vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr); }

void Renderer::DestroyDescriptorSetLayout() { vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr); }

void Renderer::DestroyPipeline() { vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr); }

void Renderer::DestroyRenderPass() { vkDestroyRenderPass(m_device, m_renderPass, nullptr); }

void Renderer::DestroyCommandPool() { vkDestroyCommandPool(m_device, m_commandPool, nullptr); }

void Renderer::DestroySyncObjects()
{
    for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
    }
}

void Renderer::DestroyTextureImage()
{
    vkDestroyImage(m_device, m_textureImage, nullptr);
    vkFreeMemory(m_device, m_textureImageMemory, nullptr);
}

void Renderer::DestroyVertexBuffer()
{
    vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
    vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);
}

void Renderer::DestroyIndexBuffer()
{
    vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
    vkFreeMemory(m_device, m_indexBufferMemory, nullptr);
}

void Renderer::DestroyUniformBuffer()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(m_device, m_uniformBuffers[i], nullptr);
        vkFreeMemory(m_device, m_uniformBuffersMemory[i], nullptr);
    }
}

void Renderer::DestroyDescriptorPool() { vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr); }

void Renderer::DestroyDescriptorSets() { vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr); }
}// namespace CookEngine
