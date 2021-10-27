import numpy as np

from vector import *

# =================================================================================================
# Matrix math. Just the usual 3D stuff.

class CMatrix:
    def __init__ (self):
        self.data = self.Identity ()


    def __getitem__ (self, index):
        return self.data [index]


    def __setitem__ (self, index, value):
        self.data [index] = value


    def Identity (self):
        return np.array ([CVector (1.0, 0.0, 0.0, 0.0), CVector (0.0, 1.0, 0.0, 0.0), CVector (0.0, 0.0, 1.0, 0.0), CVector (0.0, 0.0, 0.0, 1.0)])


    def Create (self, r, u, f):
        self.data = np.array ([r, u, f, CVector (0.0, 0.0, 0.0, 1.0)])
        return self


    def CreateFromAngles (self, sinX, cosX, sinY, cosY, sinZ, cosZ):
        return self.Create (CVector (cosY * cosZ, -cosY * sinZ, sinY),
                            CVector (sinX * sinY * cosZ + cosX * sinZ, -sinX * sinY * sinZ + cosX * cosZ, -sinX * cosY),
                            CVector (-cosX * sinY * cosZ + sinX * sinZ, cosX * sinY * sinZ + sinX * cosZ, cosX * cosY))
                        # CVector (cosX * cosY, cosX * sinY * sinC - sinX * cosZ, cosX * sinY * cosZ + sinX * sinZ), 
                        # CVector (sinX * cosY, sinX * sinY * sinC + cosX * cosZ, sinX * sinY * cosZ - cosX * sinZ), 
                        # CVector (-sinY, cosY * sinZ, cosY * cosZ))


    def R (self):
        return CVector (self [0].x, self [0].y, self [0].z)


    def U (self):
        return CVector (self [1].x, self [1].y, self [1].z)


    def F (self):
        return CVector (self [2].x, self [2].y, self [2].z)


    def __mul__ (self,  other):
        # m = np.matmul (self.AsArray (), other.AsArray ())
        # = CMatrix (CVector (m [0][0])
        m = CMatrix ()
        v = CVector (self [0].x, self [1].x, self [2].x)
        m [0].x = v.Dot (other [0])
        m [1].x = v.Dot (other [1])
        m [2].x = v.Dot (other [2])
        v = CVector (self [0].y, self [1].y, self [2].y)
        m [0].y = v.Dot (other [0])
        m [1].y = v.Dot (other [1])
        m [2].y = v.Dot (other [2])
        v = CVector (self [0].z, self [1].z, self [2].z)
        m [0].z = v.Dot (other [0])
        m [1].z = v.Dot (other [1])
        m [2].z = v.Dot (other [2])
        return m


    def __imul__ (self, other):
        v = CVector (self [0].x, self [1].x, self [2].x)
        self [0].x = v.Dot (other [0])
        self [1].x = v.Dot (other [1])
        self [2].x = v.Dot (other [2])
        v = CVector (self [0].y, self [1].y, self [2].y)
        self [0].y = v.Dot (other [0])
        self [1].y = v.Dot (other [1])
        self [2].y = v.Dot (other [2])
        v = CVector (self [0].z, self [1].z, self [2].z)
        self [0].z = v.Dot (other [0])
        self [1].z = v.Dot (other [1])
        self [2].z = v.Dot (other [2])
        return self


    def Determinant (self):
        return \
            self [0].x * (self [1].y * self [2].z - self [1].z * self [2].y) + \
	        self [0].y * (self [1].z * self [2].x - self [1].x * self [2].z) + \
		    self [0].z * (self [1].x * self [2].y - self [1].y * self [2].x)


    def Inverse (self):
        det = self.Determinant ()
        m = CMatrix ()
        m [0] = CVector ((self [1].y * self [2].z - self [1].z * self [2].y) / det,
                         (self [0].z * self [2].y - self [0].y * self [2].z) / det,
                         (self [0].y * self [1].z - self [0].z * self [1].y) / det)
        m [1] = CVector ((self [1].z * self [2].x - self [1].x * self [2].z) / det,
                         (self [0].x * self [2].z - self [0].z * self [2].x) / det,
                         (self [0].z * self [1].x - self [0].x * self [1].z) / det)
        m [2] = CVector ((self [1].x * self [2].y - self [1].y * self [2].x) / det,
                         (self [0].y * self [2].x - self [0].x * self [2].y) / det,
                         (self [0].x * self [1].y - self [0].y * self [1].x) / det)


    def Transpose (self, this):
        m = CMatrix ()
        return m.Create (CVector (this [0].x, this [1].x, this [2].x),
                         CVector (this [0].y, this [1].y, this [2].y),
                         CVector (this [0].z, this [1].z, this [2].z))


    def Rotate (self, v):
        t = self.Transpose (self)
        return CVector (v.Dot (t [0]), v.Dot (t [1]), v.Dot (t [2]))


    def Unrotate (self, v):
        return CVector (v.Dot (self [0]), v.Dot (self [1]), v.Dot (self [2]))


    def AsArray (self):
        return np.array ([
            self [0].x, self [0].y, self [0].z, self [0].w, 
            self [1].x, self [1].y, self [1].z, self [1].w, 
            self [2].x, self [2].y, self [2].z, self [2].w, 
            self [3].x, self [3].y, self [3].z, self [3].w
        ])


    def AsMatrix (self, a):
        return CMatrix (CVector.AsArray (a [0]), CVector.AsArray (a [1]), CVector.AsArray (a [2]), CVector.AsArray (a [3]))


    def Clone (self):
        m = CMatrix ()
        m.data = self.data.copy ()
        return m

# =================================================================================================
