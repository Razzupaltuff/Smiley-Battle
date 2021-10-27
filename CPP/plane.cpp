#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "plane.h"

// =================================================================================================
// Geometric computations in planes and rectangles in a plane

CPlane::CPlane() {
    m_tolerance = 0.000001f;
    m_refDots[0] = m_refDots[1] = 0.0f;
}

CPlane::CPlane(std::initializer_list<CVector> vertices) {
    Init (vertices);
}


CPlane& CPlane::operator= (std::initializer_list<CVector> vertices) {
    Init (vertices);
    return *this;
}


void CPlane::Init (std::initializer_list<CVector> vertices) {
    m_tolerance = 0.000001f;
    m_vertices = vertices;
    m_center = (m_vertices [0] + m_vertices [1] + m_vertices [2] + m_vertices [3]) / 4;
    m_normal = CVector::Normal (m_vertices [0], m_vertices [1], m_vertices [2]);
    // refEdges and refDots are precomputed for faster "point inside rectangle" tests
    m_refEdges [0] = m_vertices [1] - m_vertices [0];
    m_refEdges [1] = m_vertices [3] - m_vertices [0];
    m_refDots [0] = m_refEdges [0].Dot (m_refEdges [0]) + m_tolerance;
    m_refDots [1] = m_refEdges [1].Dot (m_refEdges [1]) + m_tolerance;
}


// project point p on this plane (i.e. compute a point in the plane 
// so that a vector from that point to p is parallel to the plane's normal)
float CPlane::Project(CVector& p, CVector& vPlane) {
    float d = (p - m_vertices[0]).Dot(m_normal);
    vPlane = p - m_normal * d;
    return d; 
}

// compute the intersection of a vector v between two points with a plane
// Will return None if v parallel to the plane or doesn't intersect with plane 
// (i.e. both points are on the same side of the plane)
bool CPlane::LineIntersection(CVector& vi, CVector& p0, CVector& p1, float r) {
    CVector v = (p0 - p1);
    float l = v.Len();
    float d0 = m_normal.Dot(p0 - m_vertices[0]);
    float d1 = m_normal.Dot(p1 - m_vertices[0]);
    if (d0 * d1 >= 0.0) {
        if ((r > 0.0f) && (-r <= d0) && (d0 <= r)) {    // end point distance to plane <= r
            vi = p0 - m_normal * d0;
            return false;
        }
        vi = CVector(NAN, NAN, NAN);     // both points on the same side of the plane
        return false;
    }
    v /= l;     // normalize v
    float d = m_normal.Dot(v);
    if ((-m_tolerance <= d) && (d <= m_tolerance)) {
        vi = CVector(NAN, NAN, NAN);     
        return false;
    }
    float dv = m_normal.Dot(m_vertices[0]);
    float t = (dv - m_normal.Dot(p0)) / d;
    vi = p0 + v * t;
    return true;  // auto [i, result] = LineIntersection (...)
}


// barycentric method for testing whether a point lies in an arbitrarily shaped triangle
// not needed for rectangular shapes in a plane
bool CPlane::TriangleContains(CVector& p, CVector& a, CVector& b, CVector& c) {
    CVector v0 = c - a;
    CVector v1 = b - a;
    CVector v2 = p - a;
    float dot00 = v0.Dot(v0);
    float dot01 = v0.Dot(v1);
    float dot02 = v0.Dot(v2);
    float dot11 = v1.Dot(v1);
    float dot12 = v1.Dot(v2);
    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    return (u >= 0.0f) && (v >= 0.0f) && (u + v < 1.0f);
}


bool CPlane::Contains(CVector& p, bool barycentric) {
    // barycentric method is rather computation heavy and not needed for rectangles in a plane
    if (barycentric)
        return 
            TriangleContains(p, m_vertices[0], m_vertices[1], m_vertices[2]) ||
            TriangleContains(p, m_vertices[0], m_vertices[2], m_vertices[3]);
    // (0 < AM ⋅ AB < AB ⋅ AB) ∧ (0 < AM ⋅ AD < AD ⋅ AD)
    CVector m = p - m_vertices[0];
    float d = m.Dot(m_refEdges[0]);
    if ((-m_tolerance > d) || (d >= m_refDots[0]))
        return false;
    d = m.Dot(m_refEdges[1]);
    if ((-m_tolerance > d) || (d >= m_refDots[1]))
        return false;
    return true;
}


void CPlane::Translate (CVector t) {
    m_center += t;
    // m_refEdges [0] += t;
    // m_refEdges [1] += t;
    for (auto v : m_vertices)
        *v += t;
}

// =================================================================================================
