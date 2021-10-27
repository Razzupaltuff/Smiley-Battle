using System;
using System.Collections.Generic;

// =================================================================================================
// Data buffer handling as support for vertex buffer operations.
// Interface classes between python and OpenGL representations of rendering data
// Supplies iterators, assignment and indexing operatores and transparent data conversion to OpenGL
// ready format (Setup() method)

public class VertexDataBuffer<APPDATA_T, GLDATA_T>
{
    public List<APPDATA_T> m_appData;
    public GLDATA_T[] m_glData;
    public int m_componentCount;

    public VertexDataBuffer(int componentCount = 1)
    {
        m_componentCount = componentCount;
        Init();
    }

    public void Init ()
    {
        m_appData = new List<APPDATA_T>();
    }
    public virtual GLDATA_T[] Setup() { return null; }


    public virtual int GLDataSize { get => 0; }

    /**/
    public byte[] GLData()
    {
        byte[] buffer = new byte[m_glData.Length * GLDataSize];
        Buffer.BlockCopy(m_glData, 0, buffer, 0, buffer.Length);
        return buffer;
    }
    /**/
    /*
    public object GLData()
    {
        return m_glData;
    }
    */
    public int AppDataLength { get => m_appData.Count; }


    public int GLDataLength { get => m_glData.Length; }


    public void Append(APPDATA_T data)
    {
        m_appData.Add(data);
    }

}

// =================================================================================================
// Buffer for vertex data (4D xyzw vector of type numpy.float32). Also used for normal data.
// A pre-populated data buffer can be passed to the constructor

public class VertexBuffer : VertexDataBuffer<Vector, float>
{
    public VertexBuffer() : base(3) { }

    public override int GLDataSize { get => sizeof (float); }

    // Create a densely packed numpy array from the vertex data
    public override float[] Setup()
    {
        m_glData = new float[m_appData.Count * 3];
        int n = 0;
        foreach (Vector v in m_appData)
        {
            m_glData[n++] = v.X;
            m_glData[n++] = v.Y;
            m_glData[n++] = v.Z;
        }
        return m_glData;
    }
}

// =================================================================================================
// Buffer for texture coordinate data (2D uv vector). Also used for color information
// A pre-populated data buffer can be passed to the constructor

public class TexCoordBuffer : VertexDataBuffer<TexCoord, float>
{
    public TexCoordBuffer() : base(2) { }

    public override int GLDataSize { get => sizeof(float); }

    // Create a densely packed numpy array from the vertex data
    public override float[] Setup()
    {
        m_glData = new float[m_appData.Count * 2];
        int n = 0;
        foreach (TexCoord tc in m_appData)
        {
            m_glData[n++] = tc.U;
            m_glData[n++] = tc.V;
        }
        return m_glData;
    }
}

// =================================================================================================
// Buffer for color data (4D rgba vector of type numpy.float32). 
// A pre-populated data buffer can be passed to the constructor

public class ColorBuffer : VertexDataBuffer<Vector, float>
{
public ColorBuffer() : base(4) { }

    public override int GLDataSize { get => sizeof(float); }

    // Create a densely packed numpy array from the vertex data
    public override float[] Setup()
    {
        m_glData = new float [m_appData.Count * 4];
        int n = 0;
        foreach (Vector v in m_appData)
        {
            m_glData[n++] = v.X;
            m_glData[n++] = v.Y;
            m_glData[n++] = v.Z;
            m_glData[n++] = v.W;
        }
        return m_glData;
    }
}

// =================================================================================================
// Buffer for index data (n-tuples of integer values). 
// Requires an additional componentCount parameter, as index count depends on the vertex count of the 
// primitive being rendered (quad: 4, triangle: 3, line: 2, point: 1)

public class IndexBuffer : VertexDataBuffer<ushort[], ushort>
{
    public IndexBuffer(int componentCount = 1) : base(componentCount) { }

    public override int GLDataSize { get => sizeof(ushort); }

    // Create a densely packed numpy array from the vertex data
    public override ushort[] Setup()
    {
        m_glData = new ushort [m_appData.Count * m_componentCount];
        int n = 0;
        foreach(ushort[] a in m_appData)
            foreach (ushort i in a)
            m_glData[n++] = i;
        return m_glData;
    }

}

// =================================================================================================
