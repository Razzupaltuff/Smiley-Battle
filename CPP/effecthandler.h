#pragma once

#include "glew.h"
#include "quad.h"
#include "timer.h"

// =================================================================================================

class CEffectHandler {
public:
    CQuad       m_screenEffect;
    CTimer      m_fadeTimer;
    CVector     m_fadeColor;
    bool        m_fadeOut;
    bool        m_fade;

    CEffectHandler ();

    void StartFade (CVector color, int duration, bool fadeOut = false);

    bool Fade (void);

};

extern CEffectHandler* effectHandler;

// =================================================================================================
