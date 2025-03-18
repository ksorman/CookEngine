#ifndef TIMER_H
#define TIMER_H

#include <chrono>

namespace Utils {

class EngineTimer
{
  public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration = Clock::duration;

    EngineTimer();

    void Tick();

    float GetDeltaTime() const;

    float GetRealDeltaTime() const;

    float GetTotalTime() const;

    float GetRealTime() const;

    void Pause();
    void Resume();
    void SetTimeScale(float scale);
    float GetTimeScale() const;
    bool IsPaused() const;

    void Reset();

  private:
    TimePoint m_startTime;
    TimePoint m_previousTime;
    float m_timeScale = 1.0f;
    float m_deltaTime = 0.0f;
    float m_realDelatTime = 0.0f;
    float m_totalTime = 0.0f;
    bool m_isPaused = false;
    float m_maxDelta = 0.1f;
};
}// namespace Utils

#endif// TIMER_H