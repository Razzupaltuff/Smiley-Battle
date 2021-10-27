from OpenGL import *
from OpenGL.GL import *

# =================================================================================================
# OpenGL vertex buffer handling: Creation, sending attributes to OpenGL, binding for rendering

class CVBO:
    def __init__ (self, type, bufferType = GL_ARRAY_BUFFER):
        self.index = -1
        self.type = type
        self.bufferType = bufferType
        self.data = None
        self.renderData = bytearray ()
        self.handle = glGenBuffers (1)
        self.size = 0
        self.itemSize = 0
        self.componentCount = 0
        self.componentType = 0


    def Bind (self):
        glBindBuffer (self.bufferType, self.handle)


    def Release (self):
        glBindBuffer (self.bufferType, 0)


    def EnableAttribs (self):
        if (self.index > -1):
            glEnableVertexAttribArray (self.index)


    def DisableAttribs (self):
        if (self.index > -1):
            glDisableVertexAttribArray (self.index)


    def Describe (self):
        if (self.index > -1):
            self.EnableAttribs ()
            glVertexAttribPointer (self.index, self.componentCount, self.componentType, False, 0, None)


    # data: buffer with OpenGL data (float or unsigned int)
    # dataSize: buffer size in bytes
    # componentType: OpenGL type of OpenGL data components (GL_FLOAT or GL_UNSIGNED_INT)
    # componentCount: Number of components of the primitives represented by the render data (3 for 3D vectors, 2 for texture coords, 4 for color values, ...)
    def Create (self, index, data, dataSize, componentType, componentCount = 1):
        self.index = index
        self.size = len (data)
        self.data = data
        self.renderData = bytearray (data)
        self.componentType = componentType
        self.componentCount = componentCount
        self.itemSize = dataSize // self.size * self.componentCount
        self.itemCount = dataSize // self.itemSize
        self.Bind ()
        glBufferData (self.bufferType, dataSize, self.renderData, GL_STATIC_DRAW)
        self.Describe ()


    def Destroy (self):
        if (self.handle > 0):
            self.Release ()
            glDeleteBuffers (1, [self.handle])
            self.handle = 0

# =================================================================================================
