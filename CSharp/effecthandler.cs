using System;

// =================================================================================================

public class EffectHandler
{
    public Quad m_screenEffect;
    public Timer m_fadeTimer;
    public Vector m_fadeColor;
    public bool m_fadeOut;
    public bool m_fade;


    public EffectHandler()
    {
        m_screenEffect = new Quad(new Vector[] { new Vector(0, 0, 0), new Vector(0, 1, 0), new Vector(1, 1, 0), new Vector(1, 0, 0) });
        m_screenEffect.Create();
        m_fadeTimer = new Timer();
        m_fadeColor = new Vector(0, 0, 0);
        m_fadeOut = false;
        m_fade = false;
    }


    public void StartFade(Vector color, int duration, bool fadeOut = false)
    {
        m_fadeColor = color;
        m_fadeOut = fadeOut;
        m_fadeTimer.m_duration = duration;
        m_fadeTimer.Start();
        m_fade = true;
    }

    public bool Fade()
    {
        if (m_fade)
        {
            GL.Enable(GL.BLEND);
            GL.BlendFunc(GL.SRC_ALPHA, GL.ONE_MINUS_SRC_ALPHA);
            float alpha = (float)m_fadeTimer.Lap() / (float)m_fadeTimer.m_duration;
            if (alpha >= 1.0)
                m_fade = false;
            if (m_fadeOut)  // fade the effect out (not the scene)
                alpha = 1.0f - alpha;
            m_screenEffect.Fill(m_fadeColor, alpha);
        }
        return m_fade;
    }

}

// =================================================================================================
