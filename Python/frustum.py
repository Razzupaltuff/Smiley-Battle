import numpy as np

from vector import *

# =================================================================================================
# compute frustum data (not used in Smiley Battle)

class CFrustum:
    def __init__ (self):
        corners = np.zeros (8, CVector)
        centers = np.zeros (6, CVector)
        normals = np.zeros (6, CVector)
        planeVerts = np.array ([[0,1,2,3], [0,1,5,4], [1,2,6,5], [2,3,7,6], [0,3,7,4], [4,5,6,7]])
        normRefs = np.array ([[1,5], [4,7], [5,4], [7,4], [4,5], [5,1]])

    def Create (self, fov, aspectRatio, zNear, zFar):
        h = np.tan (fov * np.pi / 360.0)
        w = h * aspectRatio
        n = zNear
        f = zFar
        m = f * 0.5
        r = f / n
        ln = -w
        rn = +w
        tn = +h
        bn = -h
        lf = ln * r
        rf = rn * r
        tf = tn * r
        bf = bn * r
        self.corners = np.array ([CVector (0.0, 0.0, 0.0), CVector (0.0, 0.0, 0.0), CVector (0.0, 0.0, 0.0), CVector (0.0, 0.0, 0.0)],
                                  CVector (lf, bf, f), CVector (lf, tf, f), CVector (rf ,tf, f), CVector (rf, bf, f)])
        self.centers = np.array ([CVector (0.0, 0.0, 0.0), CVector (lf * 0.5, 0.0, m), CVector (0.0, tf * 0.5, m), 
                                  CVector (rf * 0.5, 0.0, m), CVector (0.0, bf * 0.5, m), CVector (0.0, 0.0, f)])
        self.normals [0] = CVector (0.0, 0.0, 1.0)
        for i in range (1,6):
            v = self.normals [i].Normal (self.corners [self.planeVerts [i][1]], self.corners [self.planeVerts [i][2]], self.corners [self.planeVerts [i][3]])
            if (v.Dot (self.normals [i]) < 0):
                self.normals [i].Negate ()

# =================================================================================================
