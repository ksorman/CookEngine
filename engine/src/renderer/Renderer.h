#ifndef RENDERER_H
#define RENDERER_H

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace CookEngine {
class Renderer {
public:
  void Init();
  void Deinit();

private:
  void CreateInstance(VkInstance *instance);
  void CreatePhysicalDevice(VkInstance instance,
                            VkPhysicalDevice *pPhysicalDevice);
  void ChooseQueue(VkPhysicalDevice physicalDevice,
                   VkQueueFamilyProperties *pQueue);
  void CreateDevice(VkPhysicalDevice physicalDevice, VkDevice *pDevice);
  void DestrouyInstance(VkInstance instance);
  void DestrouyDevice(VkDevice device);

private:
  VkInstance m_vkInstance;
  VkPhysicalDevice m_physicalDevice;
  VkQueueFamilyProperties m_queue;
  VkDevice m_device;

  float m_quequePriority = 1.0f;
};
} // namespace CookEngine
#endif // RENDERER_H