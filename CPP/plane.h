# pragma once

#include <initializer_list>
#include <string.h>
#include <math.h>
#include "carray.h"
#include "vector.h"

// =================================================================================================
// Geometric computations in planes and rectangles in a plane

class CPlane : public CVector {

    public:
        CArray<CVector> m_vertices;
        CVector m_normal;
        CVector m_center;
        CVector m_refEdges[2];
        float   m_refDots[2];
        float   m_tolerance;

        CPlane ();

        CPlane(std::initializer_list<CVector> vertices);

        CPlane& operator= (std::initializer_list<CVector> vertices);

        void Init (std::initializer_list<CVector> vertices);

        // distance to plane (v0, v1, v2)
        inline float Distance(CVector& p) {
            return float (fabs((p - m_vertices[0]).Dot(m_normal)));
        }

        // project point p on this plane (i.e. compute a point in the plane 
        // so that a vector from that point to p is parallel to the plane's normal)
        float Project(CVector& p, CVector& vPlane);

        // compute the intersection of a vector v between two points with a plane
        // Will return None if v parallel to the plane or doesn't intersect with plane 
        // (i.e. both points are on the same side of the plane)
        bool LineIntersection(CVector& vi, CVector& p0, CVector& p1, float r = 0.0f);

        // barycentric method for testing whether a point lies in an arbitrarily shaped triangle
        // not needed for rectangular shapes in a plane
        bool TriangleContains(CVector& p, CVector& a, CVector& b, CVector& c);

        bool Contains(CVector& p, bool barycentric = false);

        void Translate (CVector t);

    };

// =================================================================================================
