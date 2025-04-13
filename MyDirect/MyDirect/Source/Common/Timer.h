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

    long m_base_time;
    long m_paused_time;
    long m_stop_time;
    long m_prev_time;
    long m_curr_time;

    bool m_stopped;
};
