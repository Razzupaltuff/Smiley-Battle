import globals

from abc import ABC, abstractmethod
from OpenGL.GL import *

from vector import *
from vertexdatabuffers import *
from vao import *
from texturehandler import *

# =================================================================================================
# Mesh class definitions for basic mesh information, allowing to pass child classes to functions
# operating with or by meshes
# A mesh is defined by a set of faces, which in turn are defined by vertices, and a set of textures
# The only mesh used in Smiley Battle are ico spheres

class CAbstractMesh:
    @abstractmethod
    def Create (self, quality, texture, textureNames):
        pass

    @abstractmethod
    def Destroy (self):
        pass

    @abstractmethod
    def PushTexture (self, texture):
        pass

    @abstractmethod
    def PopTexture (self):
        pass

    @abstractmethod
    def GetTexture (self):
        pass

    @abstractmethod
    def PushColor (self, texture):
        pass

    @abstractmethod
    def PopColor (self):
        pass

    @abstractmethod
    def GetColor (self):
        pass

    @abstractmethod
    def Render (self):
        pass

# =================================================================================================

class CMesh (CAbstractMesh):
    def __init__ (self, shape, texture = None, textureNames = None, textureType = GL_TEXTURE_2D, color = CVector (1,1,1)):
        self.textures = []
        self.colors = []
        self.vertices = None
        self.normals = None
        self.texCoords = None
        self.vertexColors = None
        self.indices = None
        self.vao = None
        self.shape = shape
        self.shapeSize = self.ShapeSize ()
        self.textures = self.SetupTexture (texture, textureNames, textureType)
        self.PushColor (color)
        self.vertexCount = 0
 

    def Destroy (self):
        pass


    def ShapeSize (self):
        if (self.shape == GL_QUADS):
            return 4
        if (self.shape == GL_TRIANGLES):
            return 3
        if (self.shape == GL_LINES):
            return 2
        return 1


    def CreateVAO (self):
        self.vao = CVAO (self.shape, self.GetTexture (), self.GetColor ())
        self.vao.Enable ()
        if (self.vertices is not None):
            self.vertices.AsArray ()
            self.vao.AddVertexBuffer ("Vertex", self.vertices.buffer, len (self.vertices.buffer) * 4, GL_FLOAT, 3)
        if (self.normals is not None):
            self.normals.AsArray ()
            # in the case of an icosphere, the vertices also are the vertex normals
            self.vao.AddVertexBuffer ("Normal", self.vertices.buffer, len (self.vertices.buffer) * 4, GL_FLOAT, 3)
        if (self.texCoords is not None):
            self.texCoords.AsArray ()
            self.vao.AddVertexBuffer ("TexCoord", self.texCoords.buffer, len (self.texCoords.buffer) * 4, GL_FLOAT, 2)
        if (self.vertexColors is not None):
            self.vertexColors.AsArray ()
            self.vao.AddVertexBuffer ("Color", self.vertexColors.buffer, len (self.vertexColors.buffer) * 4, GL_FLOAT, 4)
        if (self.indices is not None):
            self.indices.AsArray ()
            self.vao.AddIndexBuffer (self.indices.buffer, len (self.indices.buffer) * 4, GL_UNSIGNED_INT)
        self.vao.Disable ()


    def SetupTexture (self, texture, textureNames, textureType):
        if (textureNames is None):
            return [texture]
        return globals.textureHandler.CreateByType (textureNames, textureType)


    def PushTexture (self, texture):
        if (texture is not None):
            self.textures.append (texture)


    def PopTexture (self):
        if (len (self.textures) > 0):
            self.textures.pop (-1)


    def GetTexture (self):
        if (self.textures is not None) and (len (self.textures) > 0):
            return self.textures [-1]
        return None


    def PushColor (self, color):
        self.colors.append (color)


    def PopColor (self):
        self.colors.pop (-1)


    def GetColor (self):
        if (len (self.colors) > 0):
            return self.colors [-1]
        return CVector (1,1,1)


    def EnableTexture (self):
        if (self.GetTexture () is None):
            return False
        self.GetTexture ().Enable ()
        return True


    def DisableTexture (self):
        if (self.GetTexture () is not None):
            self.GetTexture ().Disable ()


    def SetTexture (self):
        self.vao.SetTexture (self.GetTexture ())


    def SetColor (self):
        self.vao.SetColor (self.GetColor ())


    def Render (self):
        if (self.vao is not None):
            self.SetTexture ()
            self.SetColor ()
            self.vao.Render ()

# =================================================================================================
