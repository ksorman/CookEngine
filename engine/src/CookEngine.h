#ifndef COOK_ENGINE_H
#define COOK_ENGINE_H

#include "renderer/Renderer.h"
#include "window/Window.h"
#include <memory>

namespace CookEngine {
class EngineApi {
public:
  void Init();
  void Deinit();
  bool Loop();

private:
  std::unique_ptr<Renderer> m_renderer;
  std::unique_ptr<Window> m_window;
};
} // namespace CookEngine
#endif // COOK_ENGINE_H
