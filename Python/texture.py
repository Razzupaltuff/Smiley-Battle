import pygame

from abc import abstractmethod
from OpenGL.GL import *

# =================================================================================================
# texture handling classes

class CAbstractTexture:
    @abstractmethod
    def Create (self, fileNames = [], flipVertically = True):
        pass

    @abstractmethod
    def Destroy (self):
        pass

    @abstractmethod
    def Available (self):
        pass

    @abstractmethod
    def Bind (self, tmu = 0):
        pass

    @abstractmethod
    def Release (self):
        pass

    @abstractmethod
    def SetParams (self):
        pass

    @abstractmethod
    def Deploy (self, bufferIndex = 0):
        pass

    @abstractmethod
    def Enable (self):
        pass

    @abstractmethod
    def Disable (self):
        pass

    @abstractmethod
    def Load (self, fileNames, flipVertically = True):
        pass

# =================================================================================================
# texture data buffer handling

class CTextureBuffer:
    def __init__ (self, width = 0, height = 0):
        self.width = width
        self.height = height
        self.data = None

# =================================================================================================
# texture handling: Loading from file, parameterization and sending to OpenGL driver, 
# enabling for rendering
# Base class for higher level textures (e.g. cubemaps)

class CTexture (CAbstractTexture):
    def __init__ (self, type = GL_TEXTURE_2D, wrapMode = GL_CLAMP_TO_EDGE, child = None):
        self.handle = 0
        self.buffers = []
        self.fileNames = []
        self.type = type
        self.wrapMode = wrapMode
        self.useMipMaps = False
        self.child = child


    def __del__ (self):
        self.Destroy ()


    def LoadFunc (self):
        if (self.child is None):
            return self.Load
        return self.child.Load


    def DeployFunc (self):
        if (self.child is None):
            return self.Deploy
        return self.child.Deploy


    def Create (self):
        self.Destroy ()
        self.handle = glGenTextures (1)
        if (self.handle > 0):
            return True
        return False


    def Destroy (self):
        if (self.handle != 0):
            self.Release ()
            glDeleteTextures (1, [self.handle])
            self.handle = 0


    def TextureCount (self):
        return len (self.buffers)


    def Available (self):
        return (self.handle > 0) and (len (self.buffers) > 0)


    def Bind (self):
        if self.Available ():
            glBindTexture (self.type, self.handle)


    def Release (self):
        if self.Available ():
            glBindTexture (self.type, 0)


    def SetParams (self):
        if self.useMipMaps:
            glTexParameteri (self.type, GL_GENERATE_MIPMAP, GL_TRUE)
            glTexParameteri (self.type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR)
            glTexParameteri (self.type, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        else:
            glTexParameteri (self.type, GL_GENERATE_MIPMAP, GL_FALSE)
            glTexParameteri (self.type, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
            glTexParameteri (self.type, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE)


    def Wrap (self):
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, self.wrapMode)
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, self.wrapMode)


    def Enable (self, tmu = 0):
        glActiveTexture (GL_TEXTURE0 + tmu)
        glEnable (self.type)
        self.Bind ()
        self.SetParams ()
        self.Wrap ()


    def Disable (self):
        self.Release ()
        glDisable (self.type)


    def Deploy (self, bufferIndex = 0):
        if self.Available ():
            self.Bind ()
            self.SetParams ()
            texBuf = self.buffers [bufferIndex]
            glTexImage2D (self.type, 0, GL_RGBA, texBuf.width, texBuf.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texBuf.data)
            self.Release ()


    # Load loads textures from file. The texture filenames are given in filenames
    # An empty filename ("") means that the previously loaded texture should be used here as well
    # This makes sense e.g. for cubemaps if several of its faces share the same texture, like e.g. spherical smileys,
    # which have a face on one side and flat color on the remaining five sides of the cubemap used to texture them.
    # So a smiley cubemap texture list could be specified here like this: ("skin.png", "", "", "", "", "face.png")
    # This would cause the skin texture to be reused for in the place of the texture data buffers at positions 2, 3, 4
    # and 5. You could also do something like ("skin.png", "", "back.png", "", "face.png") or just ("color.png", "", "", "", "", "")
    # for a uniformly textured sphere. The latter case will however be also taken into regard by the cubemap class.
    # It allows to pass a single texture which it will use for all faces of the cubemap
    def Load (self, fileNames, flipVertically = True):
        # load texture from file
        self.fileNames = fileNames
        self.textureCount = 0
        texBuf = None
        for fileName in fileNames:
            if (len (fileName) > 0):
                try:
                    image = pygame.image.load (fileName)
                except FileNotFoundError:
                    print ("Couldn't find '{0}'".format (fileName))
                    return False
                else:
                    texBuf = CTextureBuffer ()
                    texBuf.width, texBuf.height = image.get_rect ().size
                    texBuf.data = pygame.image.tostring (image, "RGBA", flipVertically)
            elif (texBuf is None):  # must have a texture at start
                return False
            self.buffers.append (texBuf)
        return True


    def CreateFromFile (self, fileNames = [], flipVertically = True):
        if not self.Create ():
            return False
        if (len (fileNames) == 0):
            return True
        if not self.Load (fileNames, flipVertically):
            return False
        self.DeployFunc () ()
        return True


    def CreateFromSurface (self, surface):
        if not self.Create ():
            return False
        texBuf = CTextureBuffer ()
        texBuf.width, texBuf.height = surface.get_rect ().size
        texBuf.data = pygame.image.tostring (surface, "RGBA", True)
        self.buffers.append (texBuf)
        return True


    def GetWidth (self, i = 0):
        return self.buffers [i].width


    def GetHeight (self, i = 0):
        return self.buffers [i].height

# =================================================================================================
