
import numpy as np

from OpenGL.GL import *
from vector import *
from texcoord import *
from vao import *
from cubemap import *
from mesh import *
from vertexdatabuffers import *

# =================================================================================================
# Basic ico sphere class.
# Ico spheres are created from basic geometric structures with equidistant corners (vertices),
# e.g. cubes, octa- or icosahedrons.
# The faces of the basic structures are subdivided in equally sized child faces. The resulting new
# vertices are normalized. The more iterations this is run through, the finer the resulting mesh
# becomes and the smoother does the sphere look.

class CIcoSphere (CMesh):
    def __init__ (self, shape, texture = None, textureNames = None, color = CVector (1,1,1)):
        super ().__init__ (shape, texture, textureNames, GL_TEXTURE_CUBE_MAP, color)
        #self.vertexColors = CColorBuffer ()
        self.vertexCount = 0
        self.vertices = CVertexBuffer ()
        self.normals = self.vertices
        self.indices = CIndexBuffer (self.shapeSize)  # shapeSize specifies the number of vertices per face (triangle: 3, quad: 4)
        self.faceCount = 0
        self.faceNormals = []


    def Destroy (self):
        pass


    def VertexIndex (self, vertices, indexLookup, i1, i2): # find index pair i1,i2 in 
        if (i1 < i2):
            key = (i1, i2)
        else:
            key = (i2, i1)
        try:
            i = indexLookup [key]
        except KeyError:
            i = self.vertexCount
            indexLookup [key] = i
            p = vertices [i1] + vertices [i2]
            vertices.Append (p.Normalize (0.5))
            self.vertexCount += 1
        return i, vertices, indexLookup
            

    def CreateFaceNormals (self, vertices, faces):
        n = CVector (0,0,0)
        faceNormals = []
        for f in faces:
            faceNormals.append (n.Normal (vertices [f [0]], vertices [f [1]], vertices [f [2]]))
        return faceNormals

# =================================================================================================
# Create an ico sphere based on a shape with triangular faces

class CTriangleIcoSphere (CIcoSphere):
    def __init__ (self, texture = None, textureNames = None, color = CVector (1,1,1)):
        super ().__init__ (GL_TRIANGLES, texture, textureNames, color)


    def Create (self, quality):
        self.vertexCount = 0
        baseVertices, baseIndices = self.CreateBaseMesh (0)
        self.vertices, self.indices, self.faceNormals = self.Refine (baseVertices, baseIndices, quality)
        self.faceCount = len (self.indices)
        self.CreateVAO ()


    def CreateBaseMesh (self, quality = 1):
        if (quality == 0):
            vertices, faces = self.CreateOctahedron ()
        else:
            vertices, faces = self.CreateIcosahedron ()
        for v in vertices:
            v.Normalize (0.5)
        return vertices, faces


    def CreateOctahedron (self):
        X = 0.5
        Y = np.sqrt (0.5)
        Z = 0.5
        vertices = [CVector (-X,0,-Z), CVector (X,0,-Z), CVector (X,0,Z), CVector (-X,0,Z), CVector (0,-Y,0), CVector (0,Y,0)]
        faces = [[0,1,5], [1,2,5], [2,3,5], [3,0,5], [0,1,6], [1,2,6], [2,3,6], [3,0,6]]
        self.vertexCount = len (vertices)
        return CVertexBuffer (vertices), CVertexBuffer (faces)


    def CreateIcosahedron (self):
        X = 0.525731112119133606
        Z = 0.850650808352039932
        N = 0.0
        vertices = [CVector (-X,+N,+Z), CVector (+X,+N,+Z), CVector (-X,+N,-Z), CVector (+X,+N,-Z),
                    CVector (+N,+Z,+X), CVector (+N,+Z,-X), CVector (+N,-Z,+X), CVector (+N,-Z,-X),
                    CVector (+Z,+X,+N), CVector (-Z,+X,+N), CVector (+Z,-X,+N), CVector (-Z,-X,+N)]
        faces = [(0,4,1), (0,9,4), (9,5,4), (4,5,8), (4,8,1), 
                 (8,10,1), (8,3,10), (5,3,8), (5,2,3), (2,7,3), 
                 (7,10,3), (7,6,10), (7,11,6), (11,0,6), (0,1,6), 
                 (6,1,10), (9,0,11), (9,11,2), (9,2,5), (7,2,11)]
        self.vertexCount = len (vertices)                     
        return CVertexBuffer (vertices), CVertexBuffer (faces)


    def SubDivide (self, vertices, faces):
        subFaces = CIndexBuffer (self.shapeSize)
        indexLookup = dict ()
        for f in faces:
            i0, vertices, indexLookup = self.VertexIndex (vertices, indexLookup, f [0], f [1])
            i1, vertices, indexLookup = self.VertexIndex (vertices, indexLookup, f [1], f [2])
            i2, vertices, indexLookup = self.VertexIndex (vertices, indexLookup, f [2], f [0])
            subFaces.Append ((f [0], i0, i2))
            subFaces.Append ((f [1], i1, i2))
            subFaces.Append ((f [2], i2, i1))
            subFaces.Append ((f [0], i1, i2))
        return vertices, subFaces


    def Refine (self, vertices, faces, quality):
        for i in range (0, quality):
            vertices, faces = self.SubDivide (vertices, faces)
        # print ("CTriangleIcoSphere.Create (" + str (quality) + ") created " + str (len (faces)) + " faces.")
        return vertices, faces, self.CreateFaceNormals (vertices, faces)

# =================================================================================================
# Create an ico sphere based on a shape with rectangular faces

class CRectangleIcoSphere (CIcoSphere):
    def __init__ (self, texture = None, textureNames = None, color = CVector (1,1,1)):
        super ().__init__ (GL_QUADS, texture, textureNames, color)


    def Create (self, quality):
        self.vertexCount = 0
        baseVertices, baseIndices = self.CreateBaseMesh (0)
        self.vertices, self.indices, self.faceNormals = self.Refine (baseVertices, baseIndices, quality)
        self.faceCount = self.indices.Length ()
        self.CreateVAO ()


    def Destroy (self):
        if (self.vao is not None):
            self.vao.Destroy ()


    def CreateBaseMesh (self, quality = 1):
        if (quality == 0):
            vertices, faces = self.CreateCube ()
        else:
            vertices, faces = self.CreateIcositetragon ()
        for v in vertices:
            v.Normalize (0.5)
        return vertices, faces


    def CreateCube (self):
        X = 0.5
        Y = 0.5
        Z = 0.5
        vertices = [CVector (-X,-Y,-Z), CVector (+X,-Y,-Z), CVector (+X,+Y,-Z), CVector (-X,+Y,-Z),
                    CVector (-X,-Y,+Z), CVector (+X,-Y,+Z), CVector (+X,+Y,+Z), CVector (-X,+Y,+Z)]
        faces = [(0,1,2,3), (0,4,5,1), (0,3,7,4), (6,2,1,5), (6,7,3,2), (6,5,4,7)]
        # self.vertexCount = len (vertices)
        return CVertexBuffer (vertices), CVertexBuffer (faces)


    def CreateIcositetragon (self):
        X = 0.5
        Y = 0.5
        Z = 0.5
        vertices = [# base cube corner vertices
                    CVector (-X,-Y,-Z), CVector (+X,-Y,-Z), CVector (+X,+Y,-Z), CVector (-X,+Y,-Z), 
                    CVector (-X,+Y,+Z), CVector (-X,-Y,+Z), CVector (+X,+Y,+Z), CVector (+X,-Y,+Z),
                    # base cube face center vertices
                    CVector (0,0,-Z), CVector (-X,0,0), CVector (+X,0,0), CVector (0,0,+Z), CVector (0,+Y,0), CVector (0,-Y,0), 
                    # front face edge center vertices
                    CVector (0,-Y,-Z), CVector (+X,0,-Z), CVector (0,+Y,+Z), CVector (-X,0,-Z), 
                    # left side face edge center vertices
                    CVector (-X,+Y,0), CVector (-X,0,+Z), CVector (-X,-Y,0),
                    # right side face edge center vertices
                    CVector (+X,-Y,0), CVector (+X,0,+Z), CVector (+X,+Y,0),
                    # front and bottom face rear edge center vertices / back face top and bottom edge center vertices
                    CVector (0,+Y,+Z), CVector (0,-Y,+Z)]
        faces = [( 8, 17,  0, 14), ( 8, 14,  1, 15), ( 8, 15,  2, 16), ( 8, 16,  3, 17), 
                 ( 9, 17,  3, 18), ( 9, 18,  4, 19), ( 9, 19,  5, 20), ( 9, 20,  0, 17), 
                 (10, 15,  1, 21), (10, 21,  7, 22), (10, 22,  6, 23), (10, 23,  2, 15), 
                 (11, 22,  7, 25), (11, 25,  5, 19), (11, 19,  4, 24), (11, 24,  6, 22), 
                 (12, 16,  2, 23), (12, 23,  6, 24), (12, 24,  4, 18), (12, 18,  3, 16), 
                 (13, 14,  0, 18), (13, 18,  5, 25), (13, 25,  7, 21), (13, 21,  1, 14)]
        # self.vertexCount = len (vertices)
        return CVertexBuffer (vertices), CIndexBuffer (4, faces)


    # Create 4 child quads per existing quad by evenly subdiving each quad.
    # To subdivide, compute the center of each side of a quad and the center of the quad.
    # Create child quads between the corners and the center of the parent quad.
    # Newly created edge center vertices will be shared with child quads of adjacent parent quads,
    # So store them in a lookup table that is indexed with the vertex indices of the parent edge.
    def SubDivide (self, vertices, faces):
        subFaces = CIndexBuffer (self.shapeSize)
        indexLookup = dict ()
        for f in faces:
            i0, vertices, indexLookup = self.VertexIndex (vertices, indexLookup, f [0], f [1])
            i1, vertices, indexLookup = self.VertexIndex (vertices, indexLookup, f [1], f [2])
            i2, vertices, indexLookup = self.VertexIndex (vertices, indexLookup, f [2], f [3])
            i3, vertices, indexLookup = self.VertexIndex (vertices, indexLookup, f [3], f [0])
            # add new center vertex. No need to remember edges, as these will only be considered during the next iteration
            i4 = self.vertexCount
            p = vertices [i0] + vertices [i1] + vertices [i2] + vertices [i3] 
            vertices.Append (p.Normalize (0.5))
            self.vertexCount += 1
            subFaces.Append ((f [0], i0, i4, i3))
            subFaces.Append ((f [1], i1, i4, i0))
            subFaces.Append ((f [2], i2, i4, i1))
            subFaces.Append ((f [3], i3, i4, i2))
        return vertices, subFaces


    def Refine (self, vertices, faces, quality):
        self.vertexCount = vertices.Length ()
        for i in range (0, quality):
            vertices, faces = self.SubDivide (vertices, faces)
        faceNormals = []
        # print ("CRectangleIcoSphere.Create (" + str (quality) + ") created " + str (len (faces)) + " faces.")
        return vertices, faces, self.CreateFaceNormals (vertices, faces)

# =================================================================================================
