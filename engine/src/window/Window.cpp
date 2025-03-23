#include "Window.h"
#include "GLFW/glfw3.h"
#include "InputHandler.h"
#include "renderer/Camera.h"

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

bool Window::IsWindowShouldClose()
{
    return glfwWindowShouldClose(m_window);
}

void Window::Events()
{
    glfwPollEvents();
}

void Window::Loop()
{
    while (!IsWindowShouldClose()) {
        Events();
    }
}

GLFWwindow* Window::GetHWND()
{
    return m_window;
}

void Window::SetUserDataPtr(UserHandler userHandler)
{
    if (!userHandler.inputHandler.inputHandler || !userHandler.resizeHandler.isResized
        || !userHandler.resizeHandler.camera) {
        spdlog::error("[GLFW] Cannot set user handler!");
        return;
    }

    m_userHandler = userHandler;
    glfwSetWindowUserPointer(m_window, &m_userHandler);
    glfwSetFramebufferSizeCallback(m_window, ResizeCallback);
    glfwSetKeyCallback(m_window, KeyCallback);
    glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
    glfwSetCursorPosCallback(m_window, MousePositionCallback);
}

void Window::ResizeCallback(GLFWwindow* window, int width, int height)
{
    auto userHandel = reinterpret_cast<UserHandler*>(glfwGetWindowUserPointer(window));
    if (!userHandel || !userHandel->resizeHandler.isResized || !userHandel->resizeHandler.camera) {
        spdlog::error("[GLFW] ResizeCallback error!");
        return;
    }
    *(userHandel->resizeHandler.isResized) = true;
    userHandel->resizeHandler.camera->SetAspectRatio(static_cast<float>(width) / static_cast<float>(height));
}

void Window::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto userHandel = reinterpret_cast<UserHandler*>(glfwGetWindowUserPointer(window));
    if (!userHandel || !userHandel->inputHandler.inputHandler) {
        spdlog::error("[GLFW] KeyCallback error!");
        return;
    }
    InputHandler::KeyState keyAction = InputHandler::KeyState::NONE;
    switch (action) {
    case GLFW_PRESS:
        keyAction = InputHandler::KeyState::PRESSED;
        break;
    case GLFW_RELEASE:
        keyAction = InputHandler::KeyState::RELEASED;
        break;
    case GLFW_REPEAT:
        keyAction = InputHandler::KeyState::HELD;
        break;
    }

    InputHandler::Key inputKey = InputHandler::Key::KEY_NONE;

    switch (key) {
    case GLFW_KEY_A:
        inputKey = InputHandler::Key::KEY_A;
        break;
    case GLFW_KEY_D:
        inputKey = InputHandler::Key::KEY_D;
        break;
    case GLFW_KEY_W:
        inputKey = InputHandler::Key::KEY_W;
        break;
    case GLFW_KEY_S:
        inputKey = InputHandler::Key::KEY_S;
        break;
    case GLFW_KEY_Q:
        inputKey = InputHandler::Key::KEY_Q;
        break;
    case GLFW_KEY_E:
        inputKey = InputHandler::Key::KEY_E;
        break;
    }

    userHandel->inputHandler.inputHandler->SetKeyState(inputKey, keyAction);
}

void Window::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    auto userHandel = reinterpret_cast<UserHandler*>(glfwGetWindowUserPointer(window));
    if (!userHandel || !userHandel->inputHandler.inputHandler) {
        spdlog::error("[GLFW] MouseButtonCallback error!");
        return;
    }
    InputHandler::KeyState keyAction = InputHandler::KeyState::NONE;
    switch (action) {
    case GLFW_PRESS:
        keyAction = InputHandler::KeyState::PRESSED;
        break;
    case GLFW_RELEASE:
        keyAction = InputHandler::KeyState::RELEASED;
        break;
    case GLFW_REPEAT:
        keyAction = InputHandler::KeyState::HELD;
        break;
    }

    InputHandler::MouseButton inputKey = InputHandler::MouseButton::KEY_NONE;

    switch (button) {
    case GLFW_MOUSE_BUTTON_RIGHT:
        inputKey = InputHandler::MouseButton::KEY_RIGHT;
        break;
    case GLFW_MOUSE_BUTTON_MIDDLE:
        inputKey = InputHandler::MouseButton::KEY_MIDDLE;
        break;
    case GLFW_MOUSE_BUTTON_LEFT:
        inputKey = InputHandler::MouseButton::KEY_LEFT;
        break;
    }

    userHandel->inputHandler.inputHandler->SetMouseButtonState(inputKey, keyAction);
}

void Window::MousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    auto userHandel = reinterpret_cast<UserHandler*>(glfwGetWindowUserPointer(window));
    if (!userHandel || !userHandel->inputHandler.inputHandler) {
        spdlog::error("[GLFW] MousePositionCallback error!");
        return;
    }

    userHandel->inputHandler.inputHandler->SetMousePositionState(xpos, ypos);
}

Window::~Window()
{
    glfwDestroyWindow(m_window);

    glfwTerminate();
    spdlog::info("GLFW Window was destroyed");
}
}// namespace CookEngine