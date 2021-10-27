import numpy as np

# =================================================================================================
# Vector math for 3D rendering

class CVector:
    def __init__ (self, x, y, z, w = 0.0):
        self.x = x
        self.y = y
        self.z = z
        self.w = w


    def __assign__ (self, other):
        self.x = other.x
        self.y = other.y
        self.z = other.z
        self.w = other.w


    def __add__ (self, other):
        return CVector (self.x + other.x, self.y + other.y, self.z + other.z)


    def __sub__ (self, other):
        return CVector (self.x - other.x, self.y - other.y, self.z - other.z)


    def __iadd__ (self, other):
        self.x += other.x
        self.y += other.y
        self.z += other.z
        return self


    def __isub__ (self, other):
        self.x -= other.x
        self.y -= other.y
        self.z -= other.z
        return self


    def __mul__ (self, other):
        return CVector (self.x * other, self.y * other, self.z * other)


    def __div__ (self, other):
        return CVector (self.x / other, self.y / other, self.z / other)


    def __neg__ (self):
        return CVector (-self.x, -self.y, -self.z)


    def __eq__ (self, other):
       return (self.x == other.x) and (self.y == other.y) and (self.z == other.z)


    def Negate (self):
        self.x = -self.x
        self.y = -self.y
        self.z = -self.z
        return self


    def CopyTo (self, other):
        other.x = self.x
        other.y = self.y
        other.z = self.z
        other.w = self.w
        return other


    # dot product of self and other
    def Dot (self, other):
        return self.x * other.x + self.y * other.y + self.z * other.z


    # cross product of self and other
    def Cross (self, other):
        return CVector (self.y * other.z - self.z * other.y, self.z * other.x - self.x * other.z, self.x * other.y - self.y * other.x)


    def Length (self):
        return np.sqrt (self.Dot (self))


    def Normalize (self, scale = 1.0):
        m = self.Length () / scale
        if m and (m != 1.0):
            self.x /= m
            self.y /= m
            self.z /= m
        return self


    # perpendicular vector to plane (v0, v1, v2)
    @staticmethod
    def Perpendicular (v0, v1, v2):
        return (v1 - v0).Cross (v2 - v1)


    # normal of plane (v0, v1, v2)
    @staticmethod
    def Normal (v0, v1, v2):
        return CVector.Perpendicular (v0, v1, v2).Normalize ()


    # distance to plane (v0, v1, v2)
    def DistanceToPlane (self, v0, v1, v2):
        return np.abs (self.Dot (self - v0, self.Normal (v0, v1, v2)))


    def ProjectToPlane (self, v0, v1, v2):
        v = self - v0
        n = self.Normal (v0, v1, v2)
        d = v.Dot (n)
        return self - n.Scale (d)


    # (0 < AM ⋅ AB < AB ⋅ AB) ∧ (0 < AM ⋅ AD < AD ⋅ AD)
    def IsInRectangle (p, v0, v1, v2):
        m = p - v0
        v = v1 - v0
        d = m.Dot (v)
        if (0.0 < d) and (d < v.Dot (v)):
            return True
        v = v2 - v0
        d = m.Dot (v)
        if (0.0 < d) and (d < v.Dot (v)):
            return True
        return False


    def Scale (self, n):
        self.x *= n
        self.y *= n
        self.z *= n
        return self


    def ScaleCopy (self, n):
        return CVector (self.x * n, self.y * n, self.z * n)


    def AsArray (self):
        return np.array ([self.x, self.y, self.z, self.w], np.float32)


    def AsVector (self, a):
        return CVector (a [0], a [1], a [2], a [3])


    def MinCoord (self, c1, c2):
        if (c1 < c2):
            return c1
        return c2


    def MaxCoord (self, c1, c2):
        if (c1 > c2):
            return c1
        return c2


    def Clone (self):
        return CVector (self.x, self.y, self.z, self.w)


    def MinDimension (self):
        return self.MinCoord (self.x, self.MinCoord (self.y, self.z))


    def MaxDimension (self):
        return self.MaxCoord (self.x, self.MaxCoord (self.y, self.z))


    def Minimize (self, other):
        if (self.x > other.x):
            self.x = other.x
        if (self.y > other.y):
            self.y = other.y
        if (self.z > other.z):
            self.z = other.z


    def Maximize (self, other):
        if (self.x < other.x):
            self.x = other.x
        if (self.y < other.y):
            self.y = other.y
        if (self.z < other.z):
            self.z = other.z


    def IsValid (self):
        return (self.x == self.x) and (self.y == self.y) and (self.z == self.z) # x == x will fail if x is NaN


    def RGBA (self):
        return CVector (self.x / 255.0, self.y / 255.0, self.z / 255.0, self.w / 255.0)

# =================================================================================================
