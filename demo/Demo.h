#ifndef DEMO_H
#define DEMO_H
#include <CookEngine.h>

class Demo : public CookEngine::EngineApi
{
  public:
    void InputCallback(CookEngine::InputHandler::Key key, CookEngine::InputHandler::KeyState action) override;
    void Tick() override;
};
#endif