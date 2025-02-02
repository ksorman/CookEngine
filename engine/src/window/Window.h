#ifndef WINDOW_H
#define WINDOW_H

#include <GLFW/glfw3.h>

namespace CookEngine {
class Window {
public:
  Window();
  ~Window();

  bool IsWindowShouldClose();
  void Events();
  void Loop();
  static const uint32_t WIDTH = 800;
  static const uint32_t HEIGHT = 600;

private:
  GLFWwindow *window;
};
} // namespace CookEngine

#endif // WINDOW_H