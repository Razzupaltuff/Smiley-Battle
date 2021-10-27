from vector import *
from texcoord import *
from vao import *
from vertexdatabuffers import *

# =================================================================================================

class CQuad:
    def __init__ (self, vertices = [], texture = None, color = CVector (1,1,1)):
        self.vertices = CVertexBuffer (vertices)
        self.texCoords = None
        self.texture = texture
        self.color = color
        self.vao = None


    def Create (self):
        self.texCoords = self.CreateTexCoords ()
        self.CreateVAO ()


    def Destroy (self):
        if (self.vao is not None):
            self.vao.Destroy ()
        if (self.texture is not None):
            self.texture.Destroy ()


    def CreateTexCoords (self):
        texCoords = CTexCoordBuffer ()
        if (self.texture is not None) and (self.texture.wrapMode == GL_REPEAT):
            for v in self.vertices:
                texCoords.Append (CTexCoord (v.x, v.z))
        else:
            texCoords.Append (CTexCoord (0, 0))
            texCoords.Append (CTexCoord (0, 1))
            texCoords.Append (CTexCoord (1, 1))
            texCoords.Append (CTexCoord (1, 0))
        return texCoords


    def CreateVAO (self):
        if (self.vertices.Length () > 0):
            self.vao = CVAO (GL_QUADS, self.texture, self.color)
            self.vertexBuffer = self.vertices.AsArray ()
            self.texCoordBuffer = self.texCoords.AsArray ()
            self.vao.Enable ()
            self.vao.AddVertexBuffer ("Vertex", self.vertexBuffer, len (self.vertexBuffer) * 4, GL_FLOAT, 3)
            self.vao.AddVertexBuffer ("TexCoord", self.texCoordBuffer, len (self.texCoordBuffer) * 4, GL_FLOAT, 2)
            self.vao.Disable ()


    def AddVertex (self, v):
        self.vertices.append (v)


    def SetTexture (self, texture):
        self.texture = texture


    def SetColor (self, color):
        self.color = color


    def Render (self):
        if (self.vao is not None):
            self.vao.SetTexture (self.texture)
            self.vao.SetColor (self.color)
            self.vao.Render (True)


    # fill 2D area defined by x and y components of vertices with color color
    def Fill (self, color, alpha = 1.0):
        # self.vao.SetTexture (None)
        # self.vao.SetColor (color)
        # self.vao.Render ()
        # print ("CQuad.Fill ({}, {}, {})".format (color.x, color.y, color.z))
        glDisable (GL_TEXTURE_2D)
        glBegin (GL_QUADS)
        glColor4f (color.x, color.y, color.z, alpha)
        glVertex2f (self.vertices [0].x, self.vertices [0].y)
        glColor4f (color.x, color.y, color.z, alpha)
        glVertex2f (self.vertices [1].x, self.vertices [1].y)
        glColor4f (color.x, color.y, color.z, alpha)
        glVertex2f (self.vertices [2].x, self.vertices [2].y)
        glColor4f (color.x, color.y, color.z, alpha)
        glVertex2f (self.vertices [3].x, self.vertices [3].y)
        glEnd ()
            

# =================================================================================================
