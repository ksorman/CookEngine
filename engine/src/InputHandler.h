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

    struct KeyDescriptor
    {
        Key key = Key::KEY_NONE;
        KeyState currentState = KeyState::NONE;
        KeyState prevState = KeyState::NONE;
    };

    InputHandler();
    bool IsKeyPressed(Key key) const;
    bool IsKeyReleased(Key key) const;
    bool IsKeyHeld(Key key) const;
    void KeyPressed(Key key);
    void KeyReleased(Key key);
    void SetKeyState(Key key, KeyState keyState);

    void SetKeyCallback(std::function<void(Key, KeyState)>&& keyCallback);

  private:
    bool CheckKeyState(Key key, KeyState keyState) const;
    std::array<KeyDescriptor, 6> m_keys;
    std::function<void(Key, KeyState)> m_externalKeyCallback = [](Key key, KeyState keyState){};
};

}// namespace CookEngine

#endif
