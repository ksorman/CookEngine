#include "Timer.h"

namespace Utils {
EngineTimer::EngineTimer() : m_startTime(Clock::now()), m_previousTime(m_startTime) {}

void EngineTimer::Tick()
{
    if (m_isPaused) {
        m_deltaTime = 0.0f;
        return;
    }

    const auto currentTime = Clock::now();
    m_realDelatTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTime - m_previousTime).count();

    m_previousTime = currentTime;
    m_deltaTime = std::min(m_realDelatTime, m_maxDelta) * m_timeScale;
    m_totalTime += m_deltaTime;
}

float EngineTimer::GetDeltaTime() const
{
    return m_deltaTime;
}

float EngineTimer::GetRealDeltaTime() const
{
    return m_realDelatTime;
}

float EngineTimer::GetTotalTime() const
{
    return m_totalTime;
}

float EngineTimer::GetRealTime() const
{
    return std::chrono::duration_cast<std::chrono::duration<float>>(Clock::now() - m_startTime).count();
}

void EngineTimer::Pause()
{
    m_isPaused = true;
}

void EngineTimer::Resume()
{
    m_isPaused = false;
}

void EngineTimer::SetTimeScale(float scale)
{
    m_timeScale = std::max(scale, 0.0f);
}

float EngineTimer::GetTimeScale() const
{
    return m_timeScale;
}

bool EngineTimer::IsPaused() const
{
    return m_isPaused;
}

void EngineTimer::Reset()
{
    m_startTime = Clock::now();
    m_previousTime = m_startTime;
    m_totalTime = 0.0f;
    m_deltaTime = 0.0f;
}
}// namespace Utils