#ifndef PHYSICAL_DEVICE_H
#define PHYSICAL_DEVICE_H

#include "VmaUsage.h"

namespace CookEngine {
class PhysicalDevice
{
  public:
    PhysicalDevice(VkInstance instance);
    operator VkPhysicalDevice();

  private:
    VkPhysicalDevice m_device = VK_NULL_HANDLE;
};
}// namespace CookEngine

#endif