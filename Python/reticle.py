import numpy as np
import globals

from OpenGL.GL import *
from quad import *
from renderer import *

# =================================================================================================
# Render a reticle on the scene. Requires an orthogonal (not perspective) projection.
# Reticle renderer needs to be called after the 3D renderer is done.

class CReticle (CQuad):
    def __init__ (self):
        super ().__init__ ([CVector (-0.125, -0.125, 0.0), CVector (-0.125, +0.125, 0.0), CVector (+0.125, +0.125, 0.0), CVector (+0.125, -0.125, 0.0)])
        self.textureNames = ["reticle-darkgreen.png", "reticle-lightgreen.png"]
        self.textures = self.CreateTextures ()


    def Create (self):
        super ().Create ()
        self.CreateTextures ()


    def Destroy (self):
        super ().Destroy ()
        self.DestroyTextures ()


    def CreateTextures (self):
        return globals.textureHandler.CreateTextures (self.textureNames)


    def DestroyTextures (self):
        pass    # textures will be globally tracked and destroyed by the textureHandler
        # for t in self.textures:
        #     t.Destroy ()


    def Render (self):
        # global renderer
        if (self.textures is None):
            return
        self.SetTexture (self.textures [globals.gameItems.viewer.ReadyToFire ()])
        glDepthFunc (GL_ALWAYS)
        glDisable (GL_CULL_FACE)
        glPushMatrix ()
        glTranslatef (0.5, 0.5, 0)
        glScalef (0.125 / globals.renderer.aspectRatio, 0.125, 1.0)
        super ().Render ()
        glPopMatrix ()
        glEnable (GL_CULL_FACE)
        glDepthFunc (GL_LESS)

# =================================================================================================
