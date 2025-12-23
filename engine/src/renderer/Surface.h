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
    operator VkSurfaceKHR();

  private:
    VkInstance m_instance;
    VkSurfaceKHR m_surface;
};
}// namespace CookEngine

#endif