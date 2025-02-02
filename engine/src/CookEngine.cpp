
#include "CookEngine.h"
#include "window/window.h"
#include <memory>
namespace CookEngine {

void EngineApi::Init() {
  m_window = std::make_unique<Window>();
  m_renderer = std::make_unique<Renderer>();
  m_renderer->Init();
}

void EngineApi::Deinit() { m_renderer->Deinit(); }

bool EngineApi::Loop() {
  while (!m_window->IsWindowShouldClose()) {
    m_window->Loop();
  }
  return true;
}
} // namespace CookEngine