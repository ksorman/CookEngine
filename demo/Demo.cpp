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
    double deltaMousePosX = m_mousePosX - m_prevMousePosX;
    double deltaMousePosY = m_mousePosY - m_prevMousePosY;

    if (GetInputHandler().IsKeyPressed(CookEngine::InputHandler::Key::KEY_W)
        || GetInputHandler().IsKeyHeld(CookEngine::InputHandler::Key::KEY_W)) {
        glm::vec3 offset = (glm::vec3(0.0f, 0.0f, 1.0f) * deltaRealTime);
        GetScene().GetCamera().MoveReletive(offset);
    }
    if (GetInputHandler().IsKeyPressed(CookEngine::InputHandler::Key::KEY_S)
        || GetInputHandler().IsKeyHeld(CookEngine::InputHandler::Key::KEY_S)) {
        glm::vec3 offset = (glm::vec3(0.0f, 0.0f, -1.0f) * deltaRealTime);
        GetScene().GetCamera().MoveReletive(offset);
    }
    if (GetInputHandler().IsKeyPressed(CookEngine::InputHandler::Key::KEY_A)
        || GetInputHandler().IsKeyHeld(CookEngine::InputHandler::Key::KEY_A)) {
        glm::vec3 offset = (glm::vec3(-1.0f, 0.0f, 0.0f) * deltaRealTime);
        GetScene().GetCamera().MoveReletive(offset);
    }
    if (GetInputHandler().IsKeyPressed(CookEngine::InputHandler::Key::KEY_D)
        || GetInputHandler().IsKeyHeld(CookEngine::InputHandler::Key::KEY_D)) {
        glm::vec3 offset = (glm::vec3(1.0f, 0.0f, 0.0f) * deltaRealTime);
        GetScene().GetCamera().MoveReletive(offset);
    }
    if (GetInputHandler().IsKeyPressed(CookEngine::InputHandler::Key::KEY_Q)
        || GetInputHandler().IsKeyHeld(CookEngine::InputHandler::Key::KEY_Q)) {
        glm::vec3 offset = (glm::vec3(0.0f, 1.0f, 0.0f) * deltaRealTime);
        GetScene().GetCamera().MoveReletive(offset);
    }
    if (GetInputHandler().IsKeyPressed(CookEngine::InputHandler::Key::KEY_E)
        || GetInputHandler().IsKeyHeld(CookEngine::InputHandler::Key::KEY_E)) {
        glm::vec3 offset = (glm::vec3(0.0f, -1.0f, 0.0f) * deltaRealTime);
        GetScene().GetCamera().MoveReletive(offset);
    }


    if (GetInputHandler().IsKeyPressed(CookEngine::InputHandler::MouseButton::KEY_LEFT)
        || GetInputHandler().IsKeyHeld(CookEngine::InputHandler::MouseButton::KEY_LEFT)) {
        glm::vec3 offset = (glm::vec3(deltaMousePosY, deltaMousePosX, 0.0f) * static_cast<float>(m_sensivity) * deltaRealTime);
        GetScene().GetCamera().RotateReletive(offset);
    }

    m_prevMousePosX = m_mousePosX;
    m_prevMousePosY = m_mousePosY;
}

void Demo::MousePositionCallback(double xPos, double yPos)
{
    m_mousePosX = xPos;
    m_mousePosY = yPos;
}