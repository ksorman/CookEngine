#include "Surface.h"
#include "spdlog/spdlog.h"

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3.h"
#include <GLFW/glfw3native.h>

namespace CookEngine {
Surface::Surface(VkInstance instance, GLFWwindow* window) : m_instance(instance)
{
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = glfwGetWin32Window(window);
    createInfo.hinstance = GetModuleHandle(nullptr);
    auto vkResult = vkCreateWin32SurfaceKHR(m_instance, &createInfo, nullptr, &m_surface);
    if (vkResult != VK_SUCCESS) {
        spdlog::error("[Vulkan] Creating surface failed");
    }
}

Surface::~Surface() {
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
}

Surface::operator VkSurfaceKHR()
{
    return m_surface;
}
}// namespace CookEngine