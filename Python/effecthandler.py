
import globals

from OpenGL.GL import *

from quad import *
from timer import *

# =================================================================================================

class CEffectHandler:
    def __init__ (self):
        self.screenEffect = CQuad ([CVector (0,0,0), CVector (0,1,0), CVector (1,1,0), CVector (1,0,0)])
        self.screenEffect.Create ()
        self.fadeTimer = CTimer ()
        self.fadeColor = CVector (0,0,0)
        self.fadeOut = False
        self.fade = False


    def StartFade (self, color, duration, fadeOut = False):
        self.fadeColor = color
        self.fadeOut = fadeOut
        self.fadeTimer.duration = duration
        self.fadeTimer.Start ()
        self.fade = True

    def Fade (self):
        if (self.fade):
            glEnable (GL_BLEND)
            glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
            alpha = self.fadeTimer.Lap () / self.fadeTimer.duration
            if (alpha >= 1.0):
                self.fade = False
            if (self.fadeOut):  # fade the effect out (not the scene)
                alpha = 1.0 - alpha
            self.screenEffect.Fill (self.fadeColor, alpha)

# =================================================================================================
