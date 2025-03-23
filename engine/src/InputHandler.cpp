#include "InputHandler.h"
#include "spdlog/spdlog.h"
#include <cstdint>
#include <utility>

namespace CookEngine {

InputHandler::InputHandler()
{
    for (int i; i < m_keys.size(); i++) {
        m_keys[i].key = static_cast<Key>(i);
    }
    for (int i; i < m_mouseButtons.size(); i++) {
        m_mouseButtons[i].key = static_cast<MouseButton>(i);
    }
}

bool InputHandler::IsKeyPressed(Key key) const
{
    return CheckKeyState(key, KeyState::PRESSED);
}

bool InputHandler::IsKeyReleased(Key key) const
{
    return CheckKeyState(key, KeyState::RELEASED);
}

bool InputHandler::IsKeyHeld(Key key) const
{
    return CheckKeyState(key, KeyState::HELD);
}
bool InputHandler::IsKeyPressed(MouseButton key) const
{
    return CheckKeyState(key, KeyState::PRESSED);
}

bool InputHandler::IsKeyReleased(MouseButton key) const
{
    return CheckKeyState(key, KeyState::RELEASED);
}

bool InputHandler::IsKeyHeld(MouseButton key) const
{
    return CheckKeyState(key, KeyState::HELD);
}

void InputHandler::KeyPressed(Key key)
{
    SetKeyState(key, KeyState::PRESSED);
}

void InputHandler::KeyReleased(Key key)
{
    SetKeyState(key, KeyState::RELEASED);
}

void InputHandler::SetKeyCallback(std::function<void(Key, KeyState)>&& keyCallback)
{
    m_externalKeyCallback = std::move(keyCallback);
}

void InputHandler::SetMouseButtonCallback(std::function<void(MouseButton, KeyState)>&& keyCallback)
{
    m_externalMouseButtonCallback = std::move(keyCallback);
}

void InputHandler::SetMousePositionCallback(std::function<void(double, double)>&& positionCallback)
{
    m_externalMousePositionCallback = std::move(positionCallback);
}

void InputHandler::SetKeyState(Key key, KeyState keyState)
{
    if (key == Key::KEY_NONE || static_cast<int16_t>(key) >= m_keys.size()) {
        spdlog::warn("[Input Handler] Wrong key {}, cannot change key state!", static_cast<int16_t>(key));
        return;
    }
    
    m_keys[static_cast<int16_t>(key)].currentState = keyState;

    m_externalKeyCallback(key, keyState);
}

void InputHandler::SetMouseButtonState(MouseButton key, KeyState keyState)
{
    if (key == MouseButton::KEY_NONE || static_cast<int16_t>(key) >= m_mouseButtons.size()) {
        spdlog::warn("[Input Handler] Wrong mouse key {}, cannot change key state!", static_cast<int16_t>(key));
        return;
    }
    
    m_mouseButtons[static_cast<int16_t>(key)].currentState = keyState;
    
    m_externalMouseButtonCallback(key, keyState);
}

void InputHandler::SetMousePositionState(double xpos, double ypos)
{
    m_externalMousePositionCallback(xpos, ypos);
    
    m_oldMousePositionX = xpos;
    m_oldMousePositionY = ypos;
}

bool InputHandler::CheckKeyState(Key key, KeyState keyState) const
{
    if (key == Key::KEY_NONE || static_cast<int16_t>(key) >= m_keys.size()) {
        spdlog::warn("[Input Handler] Wrong key {}!", static_cast<int16_t>(key));
        return false;
    }

    return m_keys[static_cast<int16_t>(key)].currentState == keyState;
}

bool InputHandler::CheckKeyState(MouseButton key, KeyState keyState) const
{
    if (key == MouseButton::KEY_NONE || static_cast<int16_t>(key) >= m_mouseButtons.size()) {
        spdlog::warn("[Input Handler] Wrong key {}!", static_cast<int16_t>(key));
        return false;
    }

    return m_mouseButtons[static_cast<int16_t>(key)].currentState == keyState;
}


}// namespace CookEngine