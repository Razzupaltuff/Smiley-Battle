import numpy as np

# =================================================================================================
# Texture coordinate representation

class CTexCoord:
    def __init__ (self, u, v):
        self.u = u
        self.v = v

    def __assign__ (self, value):
        self.u = value.u
        self.v = value.v
 
    def __add__ (self, other):
        return CTexCoord (self.u + other.u, self.v + other.v)

    def __sub__ (self, other):
        return CTexCoord (self.u - other.u, self.v - other.v)

    def __iadd__ (self, other):
        self.u += other.u
        self.v += other.v
        self.z += other.z
        return self

    def __isub__ (self, other):
        self.u -= other.u
        self.v -= other.v
        self.z -= other.z
        return self

    def __neg__ (self, other):
        return CTexCoord (-other.u, -other.v)

    def Negate (self):
        self.u = -self.u
        self.v = -self.v
        return self

    def Scale (self, n):
        self.u *= n
        self.v *= n
        return self
		
    def AsArray (self):
        return np.array ([self.u, self.v], np.float32)

# =================================================================================================
