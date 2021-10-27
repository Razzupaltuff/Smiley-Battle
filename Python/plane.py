import numpy as np
from vector import *

# =================================================================================================
# Geometric computations in planes and rectangles in a plane

class CPlane (CVector):
    def __init__ (self, vertices):
        self.vertices = vertices
        self.tolerance = 0.000001
        self.center = self.vertices [0] + self.vertices [1] + self.vertices [2] + self.vertices [3]
        self.center.Scale (0.25)
        self.normal = self.Normal (self.vertices [0], self.vertices [1], self.vertices [2])
        # refEdges and refDots are precomputed for faster "point inside rectangle" tests
        self.refEdges = [self.vertices [1] - self.vertices [0], self.vertices [3] - self.vertices [0]]
        self.refDots = [self.refEdges [0].Dot (self.refEdges [0]) + self.tolerance, self.refEdges [1].Dot (self.refEdges [1]) + self.tolerance]


    # distance to plane (v0, v1, v2)
    def Distance (self, p):
        return np.abs (p.Dot (p - self.vertices [0], self.normal))


    # project point p on this plane (i.e. compute a point in the plane 
    # so that a vector from that point to p is parallel to the plane's normal)
    def Project (self, p):
        v = p - self.vertices [0]
        d = v.Dot (self.normal)
        return p - self.normal.ScaleCopy (d), d


    # compute the intersection of a vector v between two points with a plane
    # Will return None if v parallel to the plane or doesn't intersect with plane 
    # (i.e. both points are on the same side of the plane)
    def LineIntersection (self, p, r = 0):
        v = (p [0] - p [1])
        l = v.Length ()
        d0 = self.normal.Dot (p [0] - self.vertices [0])
        d1 = self.normal.Dot (p [1] - self.vertices [0])
        if (d0 * d1 >= 0.0):
            if (r > 0) and (-r <= d0) and (d0 <= r):   # end point distance to plane <= r
                return p [0] - self.normal.ScaleCopy (d0), False
            return None, False     # both points on the same side of the plane
        v.Scale (1.0 / l)          # normalize v
        d = self.normal.Dot (v)
        if ((-self.tolerance <= d) and (d <= self.tolerance)):
            return None, False
        dv = self.normal.Dot (self.vertices [0])
        t = (dv - self.normal.Dot (p [0])) / d
        return p [0] + v.Scale (t), True


    # barycentric method for testing whether a point lies in an arbitrarily shaped triangle
    # not needed for rectangular shapes in a plane
    def TriangleContains (self, p, a, b, c):
        v0 = c - a
        v1 = b - a
        v2 = p - a
        dot00 = v0.Dot (v0)
        dot01 = v0.Dot (v1)
        dot02 = v0.Dot (v2)
        dot11 = v1.Dot (v1)
        dot12 = v1.Dot (v2)
        invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01)
        u = (dot11 * dot02 - dot01 * dot12) * invDenom
        v = (dot00 * dot12 - dot01 * dot02) * invDenom
        return (u >= 0.0) and (v >= 0.0) and (u + v < 1.0)


    def Contains (self, p, barycentric = False):
        # barycentric method is rather computation heavy and not needed for rectangles in a plane
        if (barycentric):
            return self.TriangleContains (p, self.vertices [0], self.vertices [1], self.vertices [2]) or \
                   self.TriangleContains (p, self.vertices [0], self.vertices [2], self.vertices [3])
        # (0 < AM ⋅ AB < AB ⋅ AB) ∧ (0 < AM ⋅ AD < AD ⋅ AD)
        m = p - self.vertices [0]
        d = m.Dot (self.refEdges [0])
        if (-self.tolerance > d) or (d >= self.refDots [0]):
            return False
        d = m.Dot (self.refEdges [1])
        if (-self.tolerance > d) or (d >= self.refDots [1]):
            return False
        return True

# =================================================================================================
