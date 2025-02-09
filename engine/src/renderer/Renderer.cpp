#include "Renderer.h"

#include <cstdint>
#include <set>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

namespace CookEngine {

void Renderer::Init(GLFWwindow *window)
{
  spdlog::info("Reneder Init");
  CreateInstance();
  CreateSurface(window);
  CreatePhysicalDevice();
  CreateLogicalDevice();
  CreateSwapchain(window);
}

void Renderer::Deinit()
{
  spdlog::info("Reneder Deinit");
  DestroySwapchain();
  DestroyDevice();
  DestroySurface();
  DestroyInstance();
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

void Renderer::CreateSurface(GLFWwindow *window)
{
  VkWin32SurfaceCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  createInfo.hwnd = glfwGetWin32Window(window);
  createInfo.hinstance = GetModuleHandle(nullptr);
  auto vkResult = vkCreateWin32SurfaceKHR(m_vkInstance, &createInfo, nullptr, &m_surface);
  if (vkResult != VK_SUCCESS) { spdlog::error("Creating surface failed"); }
}

void Renderer::CreateSwapchain(GLFWwindow *window)
{
  spdlog::info("Creating Swapchain!");

  SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport();

  VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, window);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

  VkSwapchainCreateInfoKHR createInfo{ .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = m_surface,
    .minImageCount = imageCount,
    .imageFormat = surfaceFormat.format,
    .imageColorSpace = surfaceFormat.colorSpace,
    .imageExtent = extent,
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
    return;
  }

  vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
  m_swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());
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
}// namespace CookEngine
