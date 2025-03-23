#ifndef WINDOW_H
#define WINDOW_H

#include "InputHandler.h"
#include <GLFW/glfw3.h>


namespace CookEngine {

class Camera;

struct UserResizeHandler
{
  bool* isResized;
  Camera* camera;
};

struct UserInputHandler
{
  InputHandler* inputHandler;
};

struct UserHandler
{
  UserResizeHandler resizeHandler;
  UserInputHandler inputHandler;
};

  class Window
{
  public:
    Window();
    ~Window();

    bool IsWindowShouldClose();
    void Events();
    void Loop();
    GLFWwindow *GetHWND();

    void SetUserDataPtr(UserHandler userHandler);
    static void ResizeCallback(GLFWwindow *window, int width, int height);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void MousePositionCallback(GLFWwindow* window, double xpos, double ypos);

  public:
    static const uint32_t WIDTH = 1400;
    static const uint32_t HEIGHT = 900;

  private:
    GLFWwindow *m_window;
    UserHandler m_userHandler;
};
}// namespace CookEngine

#endif// WINDOW_H
