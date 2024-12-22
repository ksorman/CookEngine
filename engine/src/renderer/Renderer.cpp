#include "Renderer.h"
#include <cstdint>
#include <iostream>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace CookEngine {

void Renderer::Init() {
  std::cout << "Reneder Init" << std::endl;
  CreateInstance(&m_vkInstance);
}

void Renderer::Deinit() {
  std::cout << "Reneder Deinit" << std::endl;
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
    std::cout << "Instance was created successfully" << std::endl;
  }
}

void Renderer::DestrouyInstance(VkInstance instance) {
  vkDestroyInstance(instance, nullptr);
}
} // namespace CookEngine
