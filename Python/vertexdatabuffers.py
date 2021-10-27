from vector import *
from texcoord import *
import copy

# =================================================================================================
# Iterator for vertex data buffers

class CVertexDataIterator:
   def __init__(self, buffer):
       self._buffer = buffer.data
       self._bufLen = len (self._buffer)
       self._index = 0


   def __next__ (self):
       if (self._index < self._bufLen):
           value = self._buffer [self._index]
           self._index += 1
           return value
       raise StopIteration

# =================================================================================================
# Data buffer handling as support for vertex buffer operations.
# Interface classes between python and OpenGL representations of rendering data
# Supplies iterators, assignment and indexing operatores and transparent data conversion to OpenGL
# ready format (AsArray() method)

class CVertexDataBuffer:
    def __init__ (self, componentCount, data = []):
        # data is the high level Python data
        self.data = data [:]    # need to create a copy of the buffer to avoid aliasing of successively referenced empty buffers ("[]")
        # buffer is a packed numpy array with the values from data, suitable for passing to OpenGL as bytearray
        self.buffer = []
        self.bufferLength = 0
        self.componentCount = componentCount


    def __iter__ (self):
        return CVertexDataIterator (self)


    def __assign__ (self, data):
        self.data = data


    def __getitem__ (self, index):
        return self.data [index]


    def __setitem__ (self, index, value):
        self.data [index] = value


    def __len__ (self):
        return len (self.data)


    def Append (self, value):
        self.data.append (value)


    def Buffer (self):
        return self.buffer


    def Length (self):
        return len (self.data)

# =================================================================================================
# Buffer for vertex data (4D xyzw vector of type numpy.float32). Also used for normal data.
# A pre-populated data buffer can be passed to the constructor

class CVertexBuffer (CVertexDataBuffer):
    def __init__ (self, data = []):
        super ().__init__ (3, data)

    # Create a densely packed numpy array from the vertex data
    def AsArray (self):
        self.buffer = np.empty (len (self.data) * 3, np.float32)
        n = 0
        for v in self.data:
            self.buffer [n] = v.x
            n += 1
            self.buffer [n] = v.y
            n += 1
            self.buffer [n] = v.z
            n += 1
        self.bufferLength = n
        return self.buffer

# =================================================================================================
# Buffer for texture coordinate data (2D uv vector). Also used for color information
# A pre-populated data buffer can be passed to the constructor

class CTexCoordBuffer (CVertexDataBuffer):
    def __init__ (self, data = []):
        super ().__init__ (2, data)

    def AsArray (self):
        self.buffer = np.empty (len (self.data) * 2, np.float32)
        n = 0
        for c in self.data:
            self.buffer [n] = c.u
            n += 1
            self.buffer [n] = c.v
            n += 1
        self.bufferLength = n
        return self.buffer

# =================================================================================================
# Buffer for color data (4D rgba vector of type numpy.float32). 
# A pre-populated data buffer can be passed to the constructor

class CColorBuffer (CVertexDataBuffer):
    def __init__ (self, data = []):
        super ().__init__ (4, data)

    def AsArray (self):
        self.buffer = np.empty (len (self.data) * 4, np.float32)
        n = 0
        for v in self.data:
            self.buffer [n] = v.x
            n += 1
            self.buffer [n] = v.y
            n += 1
            self.buffer [n] = v.z
            n += 1
            self.buffer [n] = 1.0
            n += 1
        self.bufferLength = n
        return self.buffer

# =================================================================================================
# Buffer for color data (4D rgba vector of type numpy.float32). 
# A pre-populated data buffer can be passed to the constructor
# Requires an additional componentCount parameter, as index count depends on the vertex count of the 
# primitive being rendered (quad: 4, triangle: 3, line: 2, point: 1)

class CIndexBuffer (CVertexDataBuffer):
    def __init__ (self, componentCount, data = []):
        super ().__init__ (componentCount, data)

    def AsArray (self):
        self.buffer = np.empty (len (self.data) * self.componentCount, np.uint32)
        n = 0
        for f in self.data:
            for i in range (self.componentCount):
                self.buffer [n] = f [i]
                n += 1
        self.bufferLength = n
        return self.buffer

# =================================================================================================
