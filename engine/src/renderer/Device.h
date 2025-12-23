#ifndef DEVICE_H
#define DEVICE_H

#include "VmaUsage.h"

namespace CookEngine {
class Device
{
  public:
    Device(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    ~Device();

    Device(Device&) = delete;
    Device(Device&&) = delete;
    Device operator=(Device&) = delete;
    Device operator=(Device&&) = delete;

    operator VkDevice();

  private:
    VkDevice m_device;
};
}// namespace CookEngine

#endif