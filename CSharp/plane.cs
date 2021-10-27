using System;

/// <summary>
/// Summary description for Class1
/// </summary>
// =================================================================================================
// Geometric computations in planes and rectangles in a plane

public class Plane : Vector
{

    Vector[] m_refEdges;
    float[] m_refDots;
    float m_tolerance;
    public Vector[] m_vertices;
    public Vector m_center;
    public Vector m_normal;

    public Plane()
    {
        m_refDots = new float[2];
        m_refEdges = new Vector[2];
    }

    public Plane(Vector[] vertices)
    {
        m_refDots = new float[2];
        m_refEdges = new Vector[2];
        Init(vertices);
    }

public Plane Init (Vector[] vertices = null)
    {
        m_tolerance = 0.000001f;
        m_refDots[0] = 0.0f;
        m_refDots[1] = 0.0f;
        m_vertices = vertices;
        m_center = (m_vertices[0] + m_vertices[1] + m_vertices[2] + m_vertices[3]) / 4;
        m_normal = Vector.Normal(m_vertices[0], m_vertices[1], m_vertices[2]);
        // refEdges and refDots are precomputed for faster "point inside rectangle" tests
        m_refEdges[0] = m_vertices[1] - m_vertices[0];
        m_refEdges[1] = m_vertices[3] - m_vertices[0];
        m_refDots[0] = m_refEdges[0].Dot(m_refEdges[0]) + m_tolerance;
        m_refDots[1] = m_refEdges[1].Dot(m_refEdges[1]) + m_tolerance;
        return this;
    }

        // distance to plane (v0, v1, v2)
public float Distance(Vector p)
{
    return (float) Math.Abs((p - m_vertices[0]).Dot(m_normal));
}

// project point p on this plane (i.e. compute a point in the plane 
// so that a vector from that point to p is parallel to the plane's normal)
public float Project(Vector p, out Vector vPlane)
    {
        float d = (p - m_vertices[0]).Dot(m_normal);
        vPlane = p - m_normal * d;
        return d;
    }

    // compute the intersection of a vector v between two points with a plane
    // Will return None if v parallel to the plane or doesn't intersect with plane 
    // (i.e. both points are on the same side of the plane)
public bool LineIntersection(out Vector vi, Vector p0, Vector p1, float r = 0.0f)
    {
        Vector v = (p0 - p1);
        float l = v.Len();
        float d0 = m_normal.Dot(p0 - m_vertices[0]);
        float d1 = m_normal.Dot(p1 - m_vertices[0]);
        if (d0 * d1 >= 0.0f)
        {
            if ((r > 0.0f) && (-r <= d0) && (d0 <= r))
            {    // end point distance to plane <= r
                vi = p0 - m_normal * d0;
                return false;
            }
            vi = Vector.None;     // both points on the same side of the plane
            return false;
        }
        v /= l;     // normalize v
        float d = m_normal.Dot(v);
        if ((-m_tolerance <= d) && (d <= m_tolerance)) {
            vi = Vector.None;
            return false;
        }
        float dv = m_normal.Dot(m_vertices[0]);
        float t = (dv - m_normal.Dot(p0)) / d;
        vi = p0 + v * t;
        return true;  // auto [i, result] = LineIntersection (...)

    }

    // barycentric method for testing whether a point lies in an arbitrarily shaped triangle
    // not needed for rectangular shapes in a plane
    public bool TriangleContains(Vector p, Vector a, Vector b, Vector c)
    {
        Vector v0 = c - a;
        Vector v1 = b - a;
        Vector v2 = p - a;
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

    public bool Contains(Vector p, bool barycentric = false)
    {
        if (barycentric)
            return
                TriangleContains(p, m_vertices[0], m_vertices[1], m_vertices[2]) ||
                TriangleContains(p, m_vertices[0], m_vertices[2], m_vertices[3]);
        // (0 < AM ⋅ AB < AB ⋅ AB) ∧ (0 < AM ⋅ AD < AD ⋅ AD)
        Vector m = p - m_vertices[0];
        float d = m.Dot(m_refEdges[0]);
        if ((-m_tolerance > d) || (d >= m_refDots[0]))
            return false;
        d = m.Dot(m_refEdges[1]);
        if ((-m_tolerance > d) || (d >= m_refDots[1]))
            return false;
        return true;

    }

    public void Translate(Vector t)
    {
        m_center += t;
        // m_refEdges [0] += t;
        // m_refEdges [1] += t;
        //for (int i = 0; i < 4; i++)
        //    m_vertices [i] += t;
    }

}

// =================================================================================================
