using System.Threading;
using SDL2;

// =================================================================================================
// Timer functions: Measuring time, delaying program execution, etc.

public class Timer
{
    public int m_time;
    public int m_lapTime;
    public int m_duration;
    public int m_slack;

    public Timer(int duration = 0) { }


    public int Start()
    {
        return m_time = (int) SDL.SDL_GetTicks();
    }



    public int Lap()
    {
        return m_lapTime = (int) SDL.SDL_GetTicks() - m_time;
    }


    public bool HasPassed(int time = 0, bool restart = false)
    {
        Lap();
        if (time == 0)
            time = m_duration;
        if ((m_time > 0) && (m_lapTime < time))
            return false;
        if (restart)
            Start();
        return true;
    }


    public void Delay()
    {
        int t = m_duration - m_slack - Lap();
        if (t > 0)
            Thread.Sleep(t);
        m_slack = Lap() - m_duration;
    }

}

// =================================================================================================
