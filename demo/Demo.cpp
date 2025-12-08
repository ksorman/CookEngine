#include "Demo.h"
#include "glm/fwd.hpp"

void Demo::LoadWorld()
{
   m_scene->AddModel(m_modelLoader->GetModel(PATH_TO_ASSETS_FOLDER + "/spider.glb"));
}

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

    glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.0f);

    if (GetInputHandler().IsKeyPressed(CookEngine::InputHandler::Key::KEY_W)
        || GetInputHandler().IsKeyHeld(CookEngine::InputHandler::Key::KEY_W)) {
        offset += (glm::vec3(0.0f, 0.0f, 1.0f) * deltaRealTime);
    }
    if (GetInputHandler().IsKeyPressed(CookEngine::InputHandler::Key::KEY_S)
        || GetInputHandler().IsKeyHeld(CookEngine::InputHandler::Key::KEY_S)) {
        offset += (glm::vec3(0.0f, 0.0f, -1.0f) * deltaRealTime);
    }
    if (GetInputHandler().IsKeyPressed(CookEngine::InputHandler::Key::KEY_A)
        || GetInputHandler().IsKeyHeld(CookEngine::InputHandler::Key::KEY_A)) {
        offset += (glm::vec3(-1.0f, 0.0f, 0.0f) * deltaRealTime);
    }
    if (GetInputHandler().IsKeyPressed(CookEngine::InputHandler::Key::KEY_D)
        || GetInputHandler().IsKeyHeld(CookEngine::InputHandler::Key::KEY_D)) {
        offset += (glm::vec3(1.0f, 0.0f, 0.0f) * deltaRealTime);
    }
    if (GetInputHandler().IsKeyPressed(CookEngine::InputHandler::Key::KEY_Q)
        || GetInputHandler().IsKeyHeld(CookEngine::InputHandler::Key::KEY_Q)) {
        offset += (glm::vec3(0.0f, 1.0f, 0.0f) * deltaRealTime);
    }
    if (GetInputHandler().IsKeyPressed(CookEngine::InputHandler::Key::KEY_E)
        || GetInputHandler().IsKeyHeld(CookEngine::InputHandler::Key::KEY_E)) {
        offset += (glm::vec3(0.0f, -1.0f, 0.0f) * deltaRealTime);
    }
    
    GetScene().GetCamera().MoveReletive(offset);

    if (GetInputHandler().IsKeyPressed(CookEngine::InputHandler::MouseButton::KEY_LEFT)
        || GetInputHandler().IsKeyHeld(CookEngine::InputHandler::MouseButton::KEY_LEFT)) {
        glm::vec3 rotation = (glm::vec3(deltaMousePosY, deltaMousePosX, 0.0f) * static_cast<float>(m_sensivity) * deltaRealTime);
        GetScene().GetCamera().RotateReletive(rotation);
    }

    m_prevMousePosX = m_mousePosX;
    m_prevMousePosY = m_mousePosY;
}

void Demo::MousePositionCallback(double xPos, double yPos)
{
    m_mousePosX = xPos;
    m_mousePosY = yPos;
}