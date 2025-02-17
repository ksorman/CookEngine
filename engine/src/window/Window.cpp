#include "Window.h"
#include "GLFW/glfw3.h"

#include <spdlog/spdlog.h>

namespace CookEngine {
Window::Window()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

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

void Window::SetUserDataPtr(bool& data) 
{
    glfwSetWindowUserPointer(m_window, &data);
    glfwSetFramebufferSizeCallback(m_window, ResizeHandel);
}

void Window::ResizeHandel(GLFWwindow *window, int width, int height)
{
    bool* userData = reinterpret_cast<bool*>(glfwGetWindowUserPointer(window));
    *userData = true;
}

Window::~Window()
{
    glfwDestroyWindow(m_window);

    glfwTerminate();
    spdlog::info("GLFW Window was destroyed");
}
}// namespace CookEngine