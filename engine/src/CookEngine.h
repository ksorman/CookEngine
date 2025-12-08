#ifndef COOK_ENGINE_H
#define COOK_ENGINE_H

#include "InputHandler.h"
#include "ModelLoader.h"
#include "renderer/Renderer.h"
#include "renderer/Scene.h"
#include "utils/Timer.h"
#include "window/Window.h"
#include <memory>
#include <spdlog/spdlog.h>

namespace CookEngine {
class EngineApi
{
  public:
    void Init();
    void Deinit();
    bool Loop();
    virtual void LoadWorld();
    virtual void Tick();
    virtual void InputCallback(InputHandler::Key key, InputHandler::KeyState action);
    virtual void MouseInputCallback(InputHandler::MouseButton key, InputHandler::KeyState action);
    virtual void MousePositionCallback(double xPos, double yPos);

    Scene& GetScene();
    const Utils::EngineTimer& GetTimer() const;
    const InputHandler& GetInputHandler() const;

  protected:
    std::unique_ptr<ModelLoader> m_modelLoader;
    std::unique_ptr<Scene> m_scene;
    
  private:
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Window> m_window;
    Utils::EngineTimer m_timer;
    InputHandler m_inputHandler;
};
}// namespace CookEngine
#endif// COOK_ENGINE_H
