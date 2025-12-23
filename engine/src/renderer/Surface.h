#ifndef SURFACE_H
#define SURFACE_H

#include "VmaUsage.h"

struct GLFWwindow;

namespace CookEngine {
class Surface
{
  public:
    Surface(VkInstance instance, GLFWwindow* window);
    ~Surface();

    Surface(Surface&) = delete;
    Surface(Surface&&) = delete;
    Surface operator=(Surface&) = delete;
    Surface operator=(Surface&&) = delete;

    operator VkSurfaceKHR();

  private:
    VkInstance m_instance;
    VkSurfaceKHR m_surface;
};
}// namespace CookEngine

#endif