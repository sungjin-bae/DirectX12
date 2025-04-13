#include "Timer.h"
#include <Windows.h>

Timer::Timer()
    : m_seconds_per_count(0.0)
    , m_delta_time(-1.0)
    , m_base_time(0)
    , m_paused_time(0)
    , m_stop_time(0)
    , m_prev_time(0)
    , m_curr_time(0)
    , m_stopped(false)
{
    __int64 counts_per_sec;
    QueryPerformanceFrequency((LARGE_INTEGER*)&counts_per_sec);
    m_seconds_per_count = 1.0 / (double)counts_per_sec;
}

float Timer::GetTotalTime() const
{
    // If we are stopped, do not count the time that has passed since we stopped.
    // Moreover, if we previously already had a pause, the distance 
    // mStopTime - mBaseTime includes paused time, which we do not want to count.
    // To correct this, we can subtract the paused time from mStopTime:  
    //
    //                     |<--paused time-->|
    // ----*---------------*-----------------*------------*------------*------> time
    //  mBaseTime       mStopTime        startTime     mStopTime    mCurrTime

    if (m_stopped)
    {
        return (float)(((m_stop_time - m_paused_time) - m_base_time) * m_seconds_per_count);
    }

    // The distance mCurrTime - mBaseTime includes paused time,
    // which we do not want to count.  To correct this, we can subtract 
    // the paused time from mCurrTime:  
    //
    //  (mCurrTime - mPausedTime) - mBaseTime 
    //
    //                     |<--paused time-->|
    // ----*---------------*-----------------*------------*------> time
    //  mBaseTime       mStopTime        startTime     mCurrTime

    else
    {
        return (float)(((m_curr_time - m_paused_time) - m_base_time) * m_seconds_per_count);
    }
}

float Timer::GetDeltaTime() const
{
    return (float)m_delta_time;
}

void Timer::Reset()
{
    __int64 curr_time;
    QueryPerformanceCounter((LARGE_INTEGER*)&curr_time);

    m_base_time = curr_time;
    m_prev_time = curr_time;
    m_stop_time = 0;
    m_stopped = false;
}

void Timer::Start()
{
    __int64 start_time;
    QueryPerformanceCounter((LARGE_INTEGER*)&start_time);

    // Accumulate the time elapsed between stop and start pairs.
    //
    //                     |<-------d------->|
    // ----*---------------*-----------------*------------> time
    //  mBaseTime       mStopTime        startTime     

    if (m_stopped)
    {
        m_paused_time += (start_time - m_stop_time);

        m_prev_time = start_time;
        m_stop_time = 0;
        m_stopped = false;
    }
}

void Timer::Stop()
{
    if (!m_stopped)
    {
        __int64 curr_time;
        QueryPerformanceCounter((LARGE_INTEGER*)&curr_time);

        m_stop_time = curr_time;
        m_stopped = true;
    }
}

void Timer::Tick()
{
    if (m_stopped)
    {
        m_delta_time = 0.0;
        return;
    }

    __int64 curr_time;
    QueryPerformanceCounter((LARGE_INTEGER*)&curr_time);
    m_curr_time = curr_time;

    // Time difference between this frame and the previous.
    m_delta_time = (m_curr_time - m_prev_time) * m_seconds_per_count;

    // Prepare for next frame.
    m_prev_time = m_curr_time;

    // Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
    // processor goes into a power save mode or we get shuffled to another
    // processor, then m_delta_time can be negative.
    if (m_delta_time < 0.0)
    {
        m_delta_time = 0.0;
    }
}
