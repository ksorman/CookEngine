#include "Demo.h"
#include "glm/fwd.hpp"

void Demo::InputCallback(CookEngine::InputHandler::Key key, CookEngine::InputHandler::KeyState action)
{
    switch (key) {
    case CookEngine::InputHandler::Key::KEY_NONE:
    case CookEngine::InputHandler::Key::KEY_A:
    case CookEngine::InputHandler::Key::KEY_W:
    case CookEngine::InputHandler::Key::KEY_S:
    case CookEngine::InputHandler::Key::KEY_D:
    case CookEngine::InputHandler::Key::KEY_Q:
    case CookEngine::InputHandler::Key::KEY_E:
        break;
    }
}

void Demo::Tick()
{
    float deltaRealTime = GetTimer().GetRealDeltaTime();

    if (GetInputHandler().IsKeyPressed(CookEngine::InputHandler::Key::KEY_W)
        || GetInputHandler().IsKeyHeld(CookEngine::InputHandler::Key::KEY_W)) {
            auto camPosition = GetScene().GetCamera().GetPosition();
            camPosition += (glm::vec3(0.0f, 0.0f, 1.0f) * deltaRealTime);
            GetScene().GetCamera().SetPosition(camPosition);
    }
    if (GetInputHandler().IsKeyPressed(CookEngine::InputHandler::Key::KEY_S)
        || GetInputHandler().IsKeyHeld(CookEngine::InputHandler::Key::KEY_S)) {
            auto camPosition = GetScene().GetCamera().GetPosition();
            camPosition += (glm::vec3(0.0f, 0.0f, -1.0f) * deltaRealTime);
            GetScene().GetCamera().SetPosition(camPosition);
    }
    if (GetInputHandler().IsKeyPressed(CookEngine::InputHandler::Key::KEY_A)
        || GetInputHandler().IsKeyHeld(CookEngine::InputHandler::Key::KEY_A)) {
            auto camPosition = GetScene().GetCamera().GetPosition();
            camPosition += (glm::vec3(-1.0f, 0.0f, 0.0f) * deltaRealTime);
            GetScene().GetCamera().SetPosition(camPosition);
    }
    if (GetInputHandler().IsKeyPressed(CookEngine::InputHandler::Key::KEY_D)
        || GetInputHandler().IsKeyHeld(CookEngine::InputHandler::Key::KEY_D)) {
            auto camPosition = GetScene().GetCamera().GetPosition();
            camPosition += (glm::vec3(1.0f, 0.0f, 0.0f) * deltaRealTime);
            GetScene().GetCamera().SetPosition(camPosition);
    }
    if (GetInputHandler().IsKeyPressed(CookEngine::InputHandler::Key::KEY_Q)
        || GetInputHandler().IsKeyHeld(CookEngine::InputHandler::Key::KEY_Q)) {
            auto camPosition = GetScene().GetCamera().GetPosition();
            camPosition += (glm::vec3(0.0f, 1.0f, 0.0f) * deltaRealTime);
            GetScene().GetCamera().SetPosition(camPosition);
    }
    if (GetInputHandler().IsKeyPressed(CookEngine::InputHandler::Key::KEY_E)
        || GetInputHandler().IsKeyHeld(CookEngine::InputHandler::Key::KEY_E)) {
            auto camPosition = GetScene().GetCamera().GetPosition();
            camPosition += (glm::vec3(0.0f, -1.0f, 0.0f) * deltaRealTime);
            GetScene().GetCamera().SetPosition(camPosition);
    }
}