#pragma once

#include <windows.h>
#include <stdio.h>
#include "SDL.h"

// =================================================================================================
// Timer functions: Measuring time, delaying program execution, etc.

class CTimer {
public:
    int  m_time;
    int  m_lapTime;
    int  m_duration;
    int  m_slack;

    CTimer(int duration = 0) : m_time(0), m_lapTime(0), m_duration(duration), m_slack(0) {}


    int Start(void) {
        return m_time = SDL_GetTicks();
    }



    int Lap(void) {
        return m_lapTime = SDL_GetTicks() - m_time;
    }


    bool HasPassed(int time = 0, bool restart = false) {
        Lap();
        if (time == 0)
            time = m_duration;
        if ((m_time > 0) && (m_lapTime < time))
            return false;
        if (restart)
            Start();
        return true;
    }


    void Delay(void) {
        int t = m_duration - m_slack - Lap();
        if (t > 0)
            Sleep(DWORD (t));
        m_slack = Lap() - m_duration;
    }
};

// =================================================================================================
