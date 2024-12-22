#include <iostream>

#include "CookEngine.h"
namespace CookEngine{

void EngineApi::Init()
{
	m_renderer = std::make_unique<Renderer>();
	m_renderer->Init();
}

void EngineApi::Deinit()
{
	m_renderer->Deinit();
}
}