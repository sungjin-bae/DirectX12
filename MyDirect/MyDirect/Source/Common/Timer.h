#pragma once

class Timer
{
public:
    Timer();

    float GetTotalTime() const;  // in seconds
    float GetDeltaTime() const;  // in seconds

    void Reset();  // Call before message loop.
    void Start();  // Call when unpaused.
    void Stop();   // Call when paused.
    void Tick();   // Call every frame.

private:
    double m_seconds_per_count;
    double m_delta_time;

    __int64 m_base_time;
    __int64 m_paused_time;
    __int64 m_stop_time;
    __int64 m_prev_time;
    __int64 m_curr_time;

    bool m_stopped;
};
