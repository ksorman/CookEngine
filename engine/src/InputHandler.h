#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <array>
#include <cstdint>
#include <functional>
namespace CookEngine {

class InputHandler
{
  public:
    enum class KeyState { NONE, RELEASED, PRESSED, HELD };

    enum class Key : int16_t { KEY_NONE = -1, KEY_A = 0, KEY_W, KEY_S, KEY_D, KEY_Q, KEY_E };
    enum class MouseButton : int16_t { KEY_NONE = -1, KEY_RIGHT = 0, KEY_MIDDLE, KEY_LEFT };

    struct KeyDescriptor
    {
        Key key = Key::KEY_NONE;
        KeyState currentState = KeyState::NONE;
        KeyState prevState = KeyState::NONE;
    };

    struct MouseButtonDescriptor
    {
        MouseButton key = MouseButton::KEY_NONE;
        KeyState currentState = KeyState::NONE;
        KeyState prevState = KeyState::NONE;
    };

    InputHandler();
    bool IsKeyPressed(Key key) const;
    bool IsKeyReleased(Key key) const;
    bool IsKeyHeld(Key key) const;
    bool IsKeyPressed(MouseButton key) const;
    bool IsKeyReleased(MouseButton key) const;
    bool IsKeyHeld(MouseButton key) const;
    void KeyPressed(Key key);
    void KeyReleased(Key key);
    void SetKeyState(Key key, KeyState keyState);
    void SetMouseButtonState(MouseButton key, KeyState keyState);
    void SetMousePositionState(double xpos, double ypos);

    void SetKeyCallback(std::function<void(Key, KeyState)>&& keyCallback);
    void SetMouseButtonCallback(std::function<void(MouseButton, KeyState)>&& keyCallback);
    void SetMousePositionCallback(std::function<void(double, double)>&& positionCallback);

  private:
    bool CheckKeyState(Key key, KeyState keyState) const;
    bool CheckKeyState(MouseButton key, KeyState keyState) const;
    std::array<KeyDescriptor, 6> m_keys;
    std::array<MouseButtonDescriptor, 3> m_mouseButtons;
    std::function<void(Key, KeyState)> m_externalKeyCallback = [](Key key, KeyState keyState){};
    std::function<void(MouseButton, KeyState)> m_externalMouseButtonCallback = [](MouseButton key, KeyState keyState){};
    std::function<void(double, double)> m_externalMousePositionCallback = [](double xpos, double ypos){};

    double m_oldMousePositionX = 0;
    double m_oldMousePositionY = 0;
};

}// namespace CookEngine

#endif
