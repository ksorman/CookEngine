#include "Renderer.h"
#include <cstdint>
#include <spdlog/spdlog.h>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace CookEngine {

void Renderer::Init() {
  spdlog::info("Reneder Init");
  CreateInstance(&m_vkInstance);
  CreatePhysicalDevice(m_vkInstance, &m_physicalDevice);
  CreateDevice(m_physicalDevice, &m_device);
}

void Renderer::Deinit() {
  spdlog::info("Reneder Deinit");
  DestrouyDevice(m_device);
  DestrouyInstance(m_vkInstance);
}

void Renderer::CreateInstance(VkInstance *pInstance) {

  VkApplicationInfo appInfo{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                            .pNext = nullptr,
                            .pApplicationName = "Demo",
                            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                            .pEngineName = "CookEngine",
                            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                            .apiVersion = VK_API_VERSION_1_0};

  std::vector<const char *> layers = {"VK_LAYER_KHRONOS_validation"};

  VkInstanceCreateInfo insatnceCreateInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = static_cast<uint32_t>(layers.size()),
      .ppEnabledLayerNames = layers.data(),
      .enabledExtensionCount = 0,
      .ppEnabledExtensionNames = nullptr};

  auto vkResult = vkCreateInstance(&insatnceCreateInfo, nullptr, pInstance);
  if (vkResult == VK_SUCCESS) {
    spdlog::info("Instance was created successfully");
  }
}

void Renderer::CreatePhysicalDevice(VkInstance instance,
                                    VkPhysicalDevice *pPhysicalDevice) {
  uint32_t numDevices = 0;
  auto vkResult = vkEnumeratePhysicalDevices(instance, &numDevices, nullptr);
  if (vkResult != VK_SUCCESS) {
    spdlog::error("Devices not found");
  }

  if (numDevices > 0) {
    std::vector<VkPhysicalDevice> devices(numDevices);
    auto vkResult =
        vkEnumeratePhysicalDevices(instance, &numDevices, devices.data());
    if (vkResult == VK_SUCCESS) {
      for (const auto &device : devices) {
        VkPhysicalDeviceProperties property;
        vkGetPhysicalDeviceProperties(device, &property);
        spdlog::info("Device: {}", property.deviceName);
      }
    }

    *pPhysicalDevice = devices[0];
  }
}

void Renderer::ChooseQueue(VkPhysicalDevice physicalDevice,
                           VkQueueFamilyProperties *pQueue) {
  uint32_t numQueues = 0;

  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueues, nullptr);

  if (numQueues > 0) {
    std::vector<VkQueueFamilyProperties> queue(numQueues);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueues,
                                             queue.data());
    *pQueue = queue[0];
  }
}

void Renderer::CreateDevice(VkPhysicalDevice physicalDevice,
                            VkDevice *pDevice) {

  VkPhysicalDeviceFeatures supportedFeatures;
  vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

  std::vector<VkDeviceQueueCreateInfo> queueInfos{
      {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
       .pNext = nullptr,
       .flags = 0,
       .queueFamilyIndex = 0,
       .queueCount = 1,
       .pQueuePriorities = &m_quequePriority}};

  VkDeviceCreateInfo deviceInfo{.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                .pNext = nullptr,
                                .queueCreateInfoCount = 1,
                                .pQueueCreateInfos = queueInfos.data(),
                                .enabledLayerCount = 0,
                                .ppEnabledLayerNames = nullptr,
                                .enabledExtensionCount = 0,
                                .ppEnabledExtensionNames = nullptr,
                                .pEnabledFeatures = &supportedFeatures};

  auto vkResult = vkCreateDevice(physicalDevice, &deviceInfo, nullptr, pDevice);
  if (vkResult == VK_SUCCESS) {
    spdlog::info("vkDevice was created successfully");
  }
}

void Renderer::DestrouyInstance(VkInstance instance) {
  vkDestroyInstance(instance, nullptr);
}

void Renderer::DestrouyDevice(VkDevice device) {
  vkDestroyDevice(device, nullptr);
}
} // namespace CookEngine
