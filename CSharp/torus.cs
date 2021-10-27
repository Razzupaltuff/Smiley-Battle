using System;
using System.Collections.Generic;

// =================================================================================================

public class Torus : Mesh
{
    public List<TexCoord> m_quadTexCoords;
    int m_vertexCount;

    public Torus(Texture texture = null, String[] textureNames = null, Vector color = null) : base()
    {
        m_vertexCount = 0;
        TexCoord[] quadTexCoords = { new TexCoord(0, 0), new TexCoord(1, 0), new TexCoord(1, 1), new TexCoord(0, 1) };
        m_quadTexCoords = new List<TexCoord>();
        foreach (TexCoord tc in quadTexCoords)
            m_quadTexCoords.Add(tc);
        Init(GL.QUADS, texture, textureNames, GL.TEXTURE_2D, color);
    }

    public void Create(ushort quality, float width, float height)
    {
        m_vertexCount = CreateVertices(quality, width, height);
        // we now have four vertex rings of m_vertexCount vertices each in the vertex buffer:
        // @ 0: lower outer vertices
        // @ vertexCount: lower inner vertices
        // @ 2 * vertexCount: upper outer vertices
        // @ 3 * vertexCount: upper inner vertices
        // now create quad indices #include "that. We will need four quad stripes:
        // outer vertical, inner vertical, lower horizontal, upper horizontal
        CreateIndex();
        CreateVAO();
    }


    // create four vertex rings. Resolution depends on quality ()
    int CreateVertices(ushort quality, float width, float height)
    {
        // compute lower outer vertex ring
        int vertexCount = CreateCircle(4 * (int) Math.Pow(2, quality), 0.5f, -height / 2.0f);
        // compute lower inner vertex ring
        float r = 1.0f - width;
        for (int i = 0; i < vertexCount; i++)
        {
            Vector v = m_vertices.m_appData[i];
            m_vertices.Append(new Vector(v.X * r, v.Y, v.Z * r));
        }
        // compute upper vertex rings by copying the lower vertex rings and replacing their y coordinates
        float y = height / 2.0f;
        for (int i = 0; i < 2 * vertexCount; i++)
        {
            Vector v = m_vertices.m_appData[i];
            m_vertices.Append(new Vector(v.X, y, v.Z));
        }
        return vertexCount;
    }


    // create circular vertex coordinates    
    int CreateCircle(int vertexCount, float r, float y)
    {
        // Rad = lambda a : a / 180.0 * np.pi
        for (int i = 0; i < vertexCount; i++)
        {
            float a = (float)i / (float)vertexCount * 2.0f * (float)Math.PI;
            m_vertices.Append(new Vector((float)Math.Cos(a) * r, y, (float)Math.Sin(a) * r));
        }
        return vertexCount;
    }


    void CreateIndex()
    {
        // create quads between lower and upper outer rings
        CreateQuadIndex(0, m_vertexCount * 2);
        // create quads between lower and upper inner rings
        CreateQuadIndex(m_vertexCount, m_vertexCount * 3);
        // create quads between lower outer and inner rings
        CreateQuadIndex(0, m_vertexCount);
        // create quads between upper outer and inner rings
        CreateQuadIndex(m_vertexCount * 2, m_vertexCount * 3);
    }


    // construct vertex indices for a quad stripe
    // This fully depends on the sequence of vertices in the vertex buffer
    void CreateQuadIndex(int i, int j)
    {
        int k = 0;
        while (true)
        {
            int h = k;
            k = (h + 1) % m_vertexCount;
            m_indices.Append(new ushort[] { (ushort) (h + i), (ushort)(h + j), (ushort)(k + j), (ushort)(k + i) });
            m_texCoords.m_appData.AddRange (m_quadTexCoords);
            if (k == 0)
                break;
        }
    }

}

// =================================================================================================
