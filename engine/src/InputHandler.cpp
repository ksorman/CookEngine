#include "InputHandler.h"
#include "spdlog/spdlog.h"
#include <cstddef>
#include <cstdint>
#include <utility>

namespace CookEngine {

InputHandler::InputHandler()
{
    for (int i; i < m_keys.size(); i++) {
        m_keys[i].key = static_cast<Key>(i);
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

bool InputHandler::CheckKeyState(Key key, KeyState keyState) const
{
    if (key == Key::KEY_NONE || static_cast<int16_t>(key) >= m_keys.size()) {
        spdlog::warn("[Input Handler] Wrong key {}!", static_cast<int16_t>(key));
        return false;
    }

    return m_keys[static_cast<int16_t>(key)].currentState == keyState;
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


}// namespace CookEngine