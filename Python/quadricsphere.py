from OpenGL.GL import *
from OpenGL.GLU import *

from texture import *

# =================================================================================================
# Sphere rendering using quadric spheres from the GLU library
# didn't work to well for me, only kept as reference

class CQuadricSphere:
    def __init__ (self):
        self.sphere = None
        self.texture = None


    def __del__ (self):
        self.Destroy ()


    def Create (self, quality):
        self.Destroy ()
        quadric = gluNewQuadric ()
        gluQuadricDrawStyle (quadric, GLU_FILL)
        gluQuadricTexture (quadric, True)
        gluQuadricNormals (quadric, GLU_SMOOTH)
        self.sphere = glGenLists (1)
        glNewList (self.sphere, GL_COMPILE)
        gluSphere (quadric, 0.5, quality, quality)
        glEndList ()
        gluDeleteQuadric (quadric)
        return self


    def Destroy (self):
        if (self.texture is not None):
            self.texture.Destroy ()
            self.texture = None


    def SetTexture (self, texture):
        self.texture = texture


    def Destroy (self):
        if (self.sphere is not None):
            glDeleteLists (1, self.sphere)
            self.sphere = None


    def Render (self):
        if (self.sphere is not None):
            if (self.texture is not None):
                self.texture.Enable ()
            glCallList (self.sphere)
            if (self.texture is not None):
                self.texture.Disable ()

# =================================================================================================
