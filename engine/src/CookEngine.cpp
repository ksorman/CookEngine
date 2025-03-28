
#include "CookEngine.h"
#include "renderer/Scene.h"
#include "window/Window.h"
#include <memory>
namespace CookEngine {

void EngineApi::Init()
{
    m_timer.Tick();
    m_window = std::make_unique<Window>();
    m_renderer = std::make_unique<Renderer>();
    m_scene = std::make_unique<Scene>();
    m_renderer->Init(m_window->GetHWND());
    m_scene->GetCamera().SetAspectRatio(static_cast<float>(Window::WIDTH) / Window::HEIGHT);
    m_window->SetUserDataPtr({ { &(m_renderer->RefToBoolForResize()), &(m_scene->GetCamera()) }, &m_inputHandler });
    m_inputHandler.SetKeyCallback(
      [this](InputHandler::Key key, InputHandler::KeyState action) { InputCallback(key, action); });
    m_inputHandler.SetMouseButtonCallback(
      [this](InputHandler::MouseButton key, InputHandler::KeyState action) { MouseInputCallback(key, action); });
    m_inputHandler.SetMousePositionCallback([this](double xPos, double yPos) { MousePositionCallback(xPos, yPos); });
}

void EngineApi::Deinit()
{
    m_renderer->Deinit();
}

bool EngineApi::Loop()
{
    while (!m_window->IsWindowShouldClose()) {
        m_timer.Tick();
        Tick();
        m_renderer->DrawFrame(*m_scene);

        m_window->Events();
    }
    return true;
}

void EngineApi::Tick() {}

void EngineApi::InputCallback(InputHandler::Key key, InputHandler::KeyState action) {}

void EngineApi::MouseInputCallback(InputHandler::MouseButton key, InputHandler::KeyState action) {}

void EngineApi::MousePositionCallback(double xPos, double yPos) {}

Scene& EngineApi::GetScene()
{
    return *m_scene;
}

const Utils::EngineTimer& EngineApi::GetTimer() const
{
    return m_timer;
}

const InputHandler& EngineApi::GetInputHandler() const
{
    return m_inputHandler;
}

}// namespace CookEngine