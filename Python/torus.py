
import globals
import numpy as np

from vector import *
from texcoord import *
from vertexdatabuffers import *
from mesh import *

# =================================================================================================

class CTorus (CMesh):
    def __init__ (self, texture = None, textureNames = None, color = CVector (1,1,1)):
        super ().__init__ (GL_QUADS, texture, textureNames, GL_TEXTURE_2D, color)
        self.vertices = CVertexBuffer ()
        self.texCoords = CTexCoordBuffer ()
        self.indices = CIndexBuffer (4)


    def Create (self, quality, width, height):
        self.quadTexCoords = [CTexCoord (0,0), CTexCoord (1,0), CTexCoord (1,1), CTexCoord (0,1)]
        self.vertexCount = self.CreateVertices (quality, width, height)
        # we now have four vertex rings of self.vertexCount vertices each in the vertex buffer:
        # @ 0: lower outer vertices
        # @ vertexCount: lower inner vertices
        # @ 2 * vertexCount: upper outer vertices
        # @ 3 * vertexCount: upper inner vertices
        # now create quad indices from that. We will need four quad stripes:
        # outer vertical, inner vertical, lower horizontal, upper horizontal
        self.CreateIndex ()
        self.CreateVAO ()


    # create four vertex rings. Resolution depends on quality ()
    def CreateVertices (self, quality, width, height):
        # compute lower outer vertex ring
        vertexCount = self.CreateCircle (4 * 2 ** quality, 0.5, -height / 2)
        # compute lower inner vertex ring
        r = 1.0 - width
        for i in range (vertexCount):
            v = self.vertices [i]
            self.vertices.Append (CVector (v.x * r, v.y, v.z * r))
        # compute upper vertex rings by copying the lower vertex rings and replacing their y coordinates
        y = height / 2
        for i in range (2 * vertexCount):
            v = self.vertices [i]
            self.vertices.Append (CVector (v.x, y, v.z))
        return vertexCount


    # create circular vertex coordinates    
    def CreateCircle (self, vertexCount, r, y):
        # Rad = lambda a : a / 180.0 * np.pi
        for i in range (0, vertexCount):
            a = i / vertexCount * 2 * np.pi
            self.vertices.Append (CVector (np.cos (a) * r, y, np.sin (a) * r))
        return vertexCount


    def CreateIndex (self):
        # create quads between lower and upper outer rings
        self.CreateQuadIndex (0, self.vertexCount * 2)
        # create quads between lower and upper inner rings
        self.CreateQuadIndex (self.vertexCount, self.vertexCount * 3)
        # create quads between lower outer and inner rings
        self.CreateQuadIndex (0, self.vertexCount)
        # create quads between upper outer and inner rings
        self.CreateQuadIndex (self.vertexCount  * 2, self.vertexCount * 3)


    # construct vertex indices for a quad stripe
    # This fully depends on the sequence of vertices in the vertex buffer
    def CreateQuadIndex (self, i, j):
        k = 0
        while True:
            h = k
            k = (h + 1) % self.vertexCount
            self.indices.Append ((h + i, h + j, k + j, k + i))
            self.texCoords.data += self.quadTexCoords
            if (k == 0):
                break                

# =================================================================================================
