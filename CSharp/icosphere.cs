using System;
using System.Collections.Generic;

// =================================================================================================
// Basic ico sphere class.
// Ico spheres are created from basic geometric structures with equidistant corners (vertices},
// e.g. cubes, octa- or icosahedrons.
// The faces of the basic structures are subdivided in equally sized child faces. The resulting new
// vertices are normalized. The more iterations this is run through, the finer the resulting mesh
// becomes and the smoother does the sphere look.

public class IcoSphere : Mesh
{
    public ushort m_vertexCount;
    public ushort m_faceCount;
    public List<Vector> m_faceNormals;


    public IcoSphere() : base(GL.TRIANGLES) 
    {
        base.Init(GL.TRIANGLES);
    }

    public IcoSphere(uint shape) : base(shape) 
    {
        base.Init(shape);
    }


    public IcoSphere(uint shape, Texture texture, String[] textureNames, Vector color = null) : base()
    {
        base.Init(shape, texture, textureNames, GL.TEXTURE_CUBE_MAP, color);
    }

    protected void SetupBaseData (Vector[] vertices, ushort[][] indices)
    {
        foreach (Vector v in vertices)
            m_vertices.Append(v);
        foreach (ushort[] f in indices)
            m_indices.Append(f);
    }

    protected ushort VertexIndex(SortedDictionary<uint, ushort> indexLookup, ushort i1, ushort i2)
    { // find index pair i1,i2 in 
        uint key = (i1 < i2) ? ((uint)i1 << 16) | i2 : ((uint)i2 << 16) | i1;
        if (indexLookup.ContainsKey(key))
            return indexLookup[key];
        indexLookup.Add(key, (ushort) m_vertexCount);
        Vector p = m_vertices.m_appData[(int) i1] + m_vertices.m_appData[(int)i2];
        p.Normalize();
        p.Scale (0.5f);
        m_vertices.Append(p);
        return m_vertexCount++;
    }


    protected List<Vector> CreateFaceNormals(VertexBuffer vertices, List<ushort[]> faces)
    {
        List<Vector> faceNormals = new List<Vector> ();
        foreach (ushort[] f in faces)
            faceNormals.Add(Vector.Normal(m_vertices.m_appData [(int)f[0]], m_vertices.m_appData[(int)f[1]], vertices.m_appData[(int) f[2]]));
        return faceNormals;
    }

}

// =================================================================================================
// Create an ico sphere based on a shape with triangular faces

public class TriangleIcoSphere : IcoSphere
{
    public TriangleIcoSphere() : base(GL.TRIANGLES) { }

    public TriangleIcoSphere(Texture texture, String[] textureNames, Vector color = null)
            : base(GL.TRIANGLES, texture, textureNames, color)
    { }

    public void Create(int quality)
    {
        CreateBaseMesh(0);
        m_vertexCount = (ushort)m_vertices.AppDataLength;
        m_indices.m_appData = Refine(m_indices.m_appData, quality);
        m_faceCount = (ushort) m_indices.AppDataLength;
        CreateVAO();
    }


    void CreateBaseMesh(int quality)
    {
        if (quality == 0)
            CreateOctahedron();
        else
            CreateIcosahedron();
        //for (int i = 0; i < m_vertices.AppDataLength; i++)
        foreach (Vector v in m_vertices.m_appData)
        {
            v.Normalize();
            v.Scale(0.5f);
            //m_vertices.m_appData[i].Normalize();
            //m_vertices.m_appData[i].Scale (0.5f);
        }
    }



    protected void CreateOctahedron()
    {
        float X = 0.5f;
        float Y = (float) Math.Sqrt(0.5);
        float Z = 0.5f;
        Vector[] vertices =
        {
            new Vector(-X, 0, -Z), new Vector(X, 0, -Z), new Vector(X, 0, Z), 
            new Vector(-X, 0, Z), new Vector(0, -Y, 0), new Vector(0, Y, 0)
        };
        ushort[][] indices = 
        {
            new ushort[] { 0,1,5}, new ushort[] { 1,2,5}, new ushort[] { 2,3,5}, new ushort[] { 3,0,5},
            new ushort[] { 0,1,6}, new ushort[] { 1,2,6}, new ushort[] { 2,3,6}, new ushort[] { 3,0,6}
        };
        SetupBaseData(vertices, indices);
    }


    protected void CreateIcosahedron()
    {
        float X = 0.525731112119133606f;
        float Z = 0.850650808352039932f;
        float N = 0.0f;
        Vector[] vertices =
        {
            new Vector(-X, +N, +Z), new Vector(+X, +N, +Z), new Vector(-X, +N, -Z), new Vector(+X, +N, -Z),
            new Vector(+N, +Z, +X), new Vector(+N, +Z, -X), new Vector(+N, -Z, +X), new Vector(+N, -Z, -X),
            new Vector(+Z, +X, +N), new Vector(-Z, +X, +N), new Vector(+Z, -X, +N), new Vector(-Z, -X, +N)
        };
        ushort[][] indices = 
        {
             new ushort[] { 0, 4,1},  new ushort[] { 0,9, 4},  new ushort[] { 9, 5,4},  new ushort[] { 4,5,8},  new ushort[] { 4,8, 1},
             new ushort[] { 8,10,1},  new ushort[] { 8,3,10},  new ushort[] { 5, 3,8},  new ushort[] { 5,2,3},  new ushort[] { 2,7, 3},
             new ushort[] { 7,10,3},  new ushort[] { 7,6,10},  new ushort[] { 7,11,6},  new ushort[] {11,0,6},  new ushort[] { 0,1, 6},
             new ushort[] { 6,1,10},  new ushort[] { 9,0,11},  new ushort[] { 9,11,2},  new ushort[] { 9,2,5},  new ushort[] { 7,2,11}
        };
        SetupBaseData(vertices, indices);
    }


    List<ushort[]> SubDivide(List<ushort[]> faces)
    {
        List<ushort[]> subFaces = new List<ushort[]> ();
        SortedDictionary<uint, ushort> indexLookup = new SortedDictionary<uint, ushort> ();
        foreach (ushort[] f in faces)
        {
            ushort i0 = VertexIndex(indexLookup, f[0], f[1]);
            ushort i1 = VertexIndex(indexLookup, f[1], f[2]);
            ushort i2 = VertexIndex(indexLookup, f[2], f[0]);
            subFaces.Add(new ushort[] { f[0], i0, i2 });
            subFaces.Add(new ushort[] { f[1], i1, i2 });
            subFaces.Add(new ushort[] { f[2], i2, i1 });
            subFaces.Add(new ushort[] { f[0], i1, i2 });
        }
        return subFaces;
    }


    List<ushort[]> Refine(List<ushort[]> faces, int quality)
    {
        while (0 < quality--)
            faces = SubDivide(faces);
        return faces;
    }

}

// =================================================================================================
// Create an ico sphere based on a shape with rectangular faces

public class RectangleIcoSphere : IcoSphere
{
    public RectangleIcoSphere() : base(GL.QUADS) { }

    public RectangleIcoSphere(Texture texture, String[] textureNames, Vector color = null)
        : base(GL.QUADS, texture, textureNames, color)
    { }

    public void Create(int quality)
    {
        CreateBaseMesh(0);
        m_vertexCount = (ushort)m_vertices.AppDataLength;
        m_indices.m_appData = Refine(m_indices.m_appData, quality);
        m_faceCount = (ushort)m_indices.AppDataLength;
        m_normals = m_vertices;
        CreateVAO();
    }


    void CreateBaseMesh(int quality = 1)
    {
        if (quality == 0)
            CreateCube();
        else
            CreateIcositetragon();
        //for (int i = 0; i < m_vertices.AppDataLength; i++)
        foreach (Vector v in m_vertices.m_appData)
        {
            v.Normalize();
            v.Scale (0.5f);
            //m_vertices.m_appData [i].Normalize();
            //m_vertices.m_appData [i] *= 0.5f;
        }
    }


    void CreateCube()
    {
        float X = 0.5f;
        float Y = 0.5f;
        float Z = 0.5f;
        Vector[] vertices =
        {
            new Vector(-X, -Y, -Z), new Vector(+X, -Y, -Z), new Vector(+X, +Y, -Z), new Vector(-X, +Y, -Z),
            new Vector(-X, -Y, +Z), new Vector(+X, -Y, +Z), new Vector(+X, +Y, +Z), new Vector(-X, +Y, +Z)
        };
        ushort[][] indices =
        {
            new ushort[] { 0,1,2,3},  new ushort[] { 0,4,5,1},  new ushort[] { 0,3,7,4},
            new ushort[] { 6,2,1,5},  new ushort[] { 6,7,3,2},  new ushort[] { 6,5,4,7}
        };
        SetupBaseData(vertices, indices);
    }


    void CreateIcositetragon()
    {
        float X = 0.5f;
        float Y = 0.5f;
        float Z = 0.5f;
        Vector[] vertices =
        {
            // base cube corner vertices
            new Vector(-X, -Y, -Z), new Vector(+X, -Y, -Z), new Vector(+X, +Y, -Z), new Vector(-X, +Y, -Z),
            new Vector(-X, +Y, +Z), new Vector(-X, -Y, +Z), new Vector(+X, +Y, +Z), new Vector(+X, -Y, +Z),
            // base cube face center vertices
            new Vector(0, 0, -Z), new Vector(-X, 0, 0), new Vector(+X, 0, 0), new Vector(0, 0, +Z), new Vector(0, +Y, 0), new Vector(0, -Y, 0),
            // front face edge center vertices
            new Vector(0, -Y, -Z), new Vector(+X, 0, -Z), new Vector(0, +Y, +Z), new Vector(-X, 0, -Z),
            // left side face edge center vertices
            new Vector(-X, +Y, 0), new Vector(-X, 0, +Z), new Vector(-X, -Y, 0),
            // right side face edge center vertices
            new Vector(+X, -Y, 0), new Vector(+X, 0, +Z), new Vector(+X, +Y, 0),
            // front and bottom face rear edge center vertices / back face top and bottom edge center vertices
            new Vector(0, +Y, +Z), new Vector(0, -Y, +Z)
        };
        ushort[][] indices =
        {
            new ushort[] { 8, 17,   0, 14},  new ushort[] {  8, 14,  1, 15},  new ushort[] {  8, 15,  2, 16},  new ushort[] {  8, 16,  3, 17},
            new ushort[] { 9, 17,   3, 18},  new ushort[] {  9, 18,  4, 19},  new ushort[] {  9, 19,  5, 20},  new ushort[] {  9, 20,  0, 17},
            new ushort[] { 10, 15,  1, 21},  new ushort[] { 10, 21,  7, 22},  new ushort[] { 10, 22,  6, 23},  new ushort[] { 10, 23,  2, 15},
            new ushort[] { 11, 22,  7, 25},  new ushort[] { 11, 25,  5, 19},  new ushort[] { 11, 19,  4, 24},  new ushort[] { 11, 24,  6, 22},
            new ushort[] { 12, 16,  2, 23},  new ushort[] { 12, 23,  6, 24},  new ushort[] { 12, 24,  4, 18},  new ushort[] { 12, 18,  3, 16},
            new ushort[] { 13, 14,  0, 18},  new ushort[] { 13, 18,  5, 25},  new ushort[] { 13, 25,  7, 21},  new ushort[] { 13, 21,  1, 14}
        };
        SetupBaseData(vertices, indices);
    }


    // Create 4 child quads per existing quad by evenly subdiving each quad.
    // To subdivide, compute the center of each side of a quad and the center of the quad.
    // Create child quads between the corners and the center of the parent quad.
    // Newly created edge center vertices will be shared with child quads of adjacent parent quads,
    // So store them in a lookup table that is indexed with the vertex indices of the parent edge.
    List<ushort[]> SubDivide(List<ushort[]> faces)
    {
        List<ushort[]> subFaces = new List<ushort[]>();
        SortedDictionary<uint, ushort> indexLookup = new SortedDictionary<uint, ushort>();
        foreach (ushort[] f in faces)
        {
            ushort i0 = VertexIndex(indexLookup, f [0], f [1]);
            ushort i1 = VertexIndex(indexLookup, f [1], f [2]);
            ushort i2 = VertexIndex(indexLookup, f [2], f [3]);
            ushort i3 = VertexIndex(indexLookup, f [3], f [0]);
            ushort i4 = m_vertexCount++;
            Vector p = m_vertices.m_appData[(int)i0] + m_vertices.m_appData[(int)i1] + m_vertices.m_appData[(int)i2] + m_vertices.m_appData[(int)i3];
            p.Normalize();
            p.Scale (0.5f);
            m_vertices.Append(p);
            subFaces.Add(new ushort[] { f [0], i0, i4, i3 });
            subFaces.Add(new ushort[] { f [1], i1, i4, i0 });
            subFaces.Add(new ushort[] { f [2], i2, i4, i1 });
            subFaces.Add(new ushort[] { f [3], i3, i4, i2 });
        }
        return subFaces;
    }


    List<ushort[]> Refine(List<ushort[]> faces, int quality)
    {
        while (0 < quality--)
            faces = SubDivide(faces);
        return faces;
    }

}

// =================================================================================================
