
import globals

from OpenGL.GL import *
from texture import *
from cubemap import *

# =================================================================================================
# Very simply class for texture tracking
# Main purpose is to keep track of all texture objects in the game and return them to OpenGL in
# a well defined and controlled way at program termination without having to bother about releasing
# textures at a dozen places in the game

class CTextureHandler:
    def __init__ (self):
        self.textures = []


    def Destroy (self):
        for t in self.textures:
            t.Destroy ()


    def GetTexture (self):
        self.textures.append (CTexture ())
        return self.textures [-1]


    def GetCubemap (self):
        self.textures.append (CCubemap ())
        return self.textures [-1]


    def Create (self, textureNames, getter):
        textures = []
        for n in textureNames:
            t = getter ()
            textures.append (t)
            if not t.CreateFromFile ([globals.gameData.textureFolder + n]):
                return None
        return textures


    def CreateTextures (self, textureNames):
        return self.Create (textureNames, self.GetTexture)


    def CreateCubemaps (self, textureNames):
        return self.Create (textureNames, self.GetCubemap)


    def CreateByType (self, textureNames, textureType):
        if (textureType == GL_TEXTURE_CUBE_MAP):
            return self.CreateCubemaps (textureNames)
        return self.CreateTextures (textureNames)

# =================================================================================================
