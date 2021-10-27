
#include "effectHandler.h"

// =================================================================================================

CEffectHandler::CEffectHandler () {
    m_screenEffect = CQuad ({ CVector (0,0,0), CVector (0,1,0), CVector (1,1,0), CVector (1,0,0) });
    m_screenEffect.Create ();
    m_fadeTimer = CTimer ();
    m_fadeColor = CVector (0, 0, 0);
    m_fadeOut = false;
    m_fade = false;
}


void CEffectHandler::StartFade (CVector color, int duration, bool fadeOut) {
    m_fadeColor = color;
    m_fadeOut = fadeOut;
    m_fadeTimer.m_duration = duration;
    m_fadeTimer.Start ();
    m_fade = true;
}

bool CEffectHandler::Fade (void) {
    if (m_fade) {
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        float alpha = float (m_fadeTimer.Lap ()) / float (m_fadeTimer.m_duration);
        if (alpha >= 1.0)
            m_fade = false;
        if (m_fadeOut)  // fade the effect out (not the scene)
            alpha = 1.0f - alpha;
        m_screenEffect.Fill (m_fadeColor, alpha);
    }
    return m_fade;
}

CEffectHandler* effectHandler = nullptr;

// =================================================================================================
