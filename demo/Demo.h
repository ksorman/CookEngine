#ifndef DEMO_H
#define DEMO_H
#include <CookEngine.h>

class Demo : public CookEngine::EngineApi
{
  public:
    void InputCallback(CookEngine::InputHandler::Key key, CookEngine::InputHandler::KeyState action) override;
    void Tick() override;
    void MousePositionCallback(double xPos, double yPos) override;
    private:
      double m_mousePosX = 0.0f;
      double m_mousePosY = 0.0f;
      double m_prevMousePosX = 0.0f;
      double m_prevMousePosY = 0.0f;

      double m_sensivity = 0.1f;
};
#endif