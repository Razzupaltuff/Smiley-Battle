import globals

from OpenGL import *
from OpenGL.GL import *
from shaders import *
from vbo import *
from vector import *
from texcoord import *

# =================================================================================================
# "Premium version of" OpenGL vertex array objects. CVAO instances offer methods to convert python
# lists into the corresponding lists of OpenGL items (vertices, normals, texture coordinates, etc)
# The current implementation requires a fixed order of array buffer creation to comply with the 
# corresponding layout positions in the shaders implemented here.
# Currently offers shaders for cubemap and regular (2D) texturing.
# Implements loading of varying textures, so an application item derived from or using a CVAO instance
# (e.g. an ico sphere) can be reused by different other application items that require different 
# texturing. This implementation basically allows for reusing one single ico sphere instance whereever
# a sphere is needed.
# Supports indexed and non indexed vertex buffer objects.
#
# # Due to the current shader implementation (fixed position layout), buffers need to be passed in a
# fixed sequence: vertices, colors, ...
# See also https://qastack.com.de/programming/8704801/glvertexattribpointer-clarification

class CVAO:
    def __init__ (self, shape, texture = None, color = CVector (1,1,1)):
        self.dataBuffers = []
        self.indexBuffer = None
        self.handle = glGenVertexArrays (1)
        self.shape = shape
        self.texture = texture
        self.color = color
        self.minBrightness = 0


    def Destroy (self):
        for vbo in self.dataBuffers:
            vbo.Destroy ()
        if (self.handle > 0):
            self.Disable ()
            glDeleteVertexArrays (1, [self.handle])
            self.handle = 0


    def Enable (self):
        glBindVertexArray (self.handle)


    def Disable (self):
        glBindVertexArray (0)


    # add a vertex or index data buffer
    def AddBuffer (self, type, data, dataSize, componentType, componentCount = 0):
        if (type == "Index"):
            self.AddIndexBuffer (data, dataSize, componentType)
        else:
            self.AddVertexBuffer (type, data, dataSize, componentType, componentCount)


    def AddVertexBuffer (self, type, data, dataSize, componentType, componentCount):
        buffer = CVBO (type, GL_ARRAY_BUFFER)
        buffer.Create (len (self.dataBuffers), data, dataSize, componentType, componentCount)
        self.dataBuffers.append (buffer)


    def AddIndexBuffer (self, data, dataSize, componentType):
        self.indexBuffer = CVBO ("Index", GL_ELEMENT_ARRAY_BUFFER)
        self.indexBuffer.Create (-1, data, dataSize, componentType)


    def SetTexture (self, texture):
        self.texture = texture


    def SetColor (self, color):
        self.color = color


    def SetMinBrightness (self, minBrightness):
        self.minBrightness = minBrightness


    def ResetMinBrightness (self):
        self.minBrightness = 0


    def EnableTexture (self):
        if (self.texture is None):
            return None
        self.texture.Enable ()
        return self.texture


    def DisableTexture (self):
        if (self.texture is not None):
            self.texture.Disable ()


    def Render (self, useShader = True):
        shaderId = globals.shaderHandler.SelectShader (useShader, self.EnableTexture ())
        globals.shaderHandler.shaders [shaderId].SetUniformVector ("fillColor", self.color)
        if (shaderId == 0):
            globals.shaderHandler.shaders [shaderId].SetUniformFloat ("minBrightness", self.minBrightness)
        self.Enable ()
        # glColor3f (self.color.x, self.color.y, self.color.z)
        if (self.indexBuffer is None):
            glDrawArrays (self.shape, 0, self.dataBuffers [0].itemCount) # draw non indexed arrays
        else:
            glDrawElements (self.shape, self.indexBuffer.size, self.indexBuffer.componentType, None) # draw using an index buffer
            self.Disable ()
        if (shaderId > -1):
            globals.shaderHandler.shaders [shaderId].Disable ()
        glUseProgram (0)
        self.DisableTexture ()


# =================================================================================================
