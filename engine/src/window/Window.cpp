#include "Window.h"

#include <spdlog/spdlog.h>

namespace CookEngine {
Window::Window()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    spdlog::info("GLFW Window was initialized");
}

bool Window::IsWindowShouldClose() { return glfwWindowShouldClose(m_window); }

void Window::Events() { glfwPollEvents(); }

void Window::Loop()
{
    while (!IsWindowShouldClose()) { Events(); }
}

GLFWwindow *Window::GetHWND() { return m_window; }

Window::~Window()
{
    glfwDestroyWindow(m_window);

    glfwTerminate();
    spdlog::info("GLFW Window was destroyed");
}
}// namespace CookEngine