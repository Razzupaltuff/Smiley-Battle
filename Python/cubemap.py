import pygame
import numpy as np

from OpenGL.GL import *
from texture import *

# =================================================================================================
# Load cubemap textures from file and generate an OpenGL cubemap 

class CCubemap (CTexture):
    def __init__ (self):
        super ().__init__ (GL_TEXTURE_CUBE_MAP, child = self)


    def Destroy (self):
        super ().Destroy ()


    def SetParams (self):
        glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)
        glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE)
        glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE)  


    def Deploy (self):
        if self.Available ():
            self.Bind ()
            self.SetParams ()
            l = len (self.buffers)
            # put the available textures on the cubemap as far as possible and put the last texture on any remaining cubemap faces
            # Reguar case six textures: One texture for each cubemap face
            # Special case one textures: all cubemap faces bear the same texture
            # Special case two textures: first texture goes to first 5 cubemap faces, 2nd texture goes to 6th cubemap face. Special case for smileys with a uniform skin and a face                    
            for i in range (l):
                texBuf = self.buffers [i]
                glTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, texBuf.width, texBuf.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texBuf.data)
            for i in range (l, 6):
                glTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, texBuf.width, texBuf.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texBuf.data)
            self.Release ()

# =================================================================================================
