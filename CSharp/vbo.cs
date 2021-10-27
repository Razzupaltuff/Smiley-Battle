using System;

// =================================================================================================
// OpenGL vertex buffer handling: Creation, sending attributes to OpenGL, binding for rendering

public class VBO
{
    public int m_index;
    public String m_type;
    public uint m_bufferType;
    public byte[] /*object*/ m_data;
    public uint[] m_handle;
    public int m_size;
    public int m_itemSize;
    public int m_itemCount;
    public int m_componentCount;
    public uint m_componentType;


    public VBO()
    {
        m_index = -1;
        m_type = "";
        m_bufferType = 0;
        m_data = null;
        m_size = 0;
        m_itemSize = 0;
        m_itemCount = 0;
        m_componentCount = 0;
        m_componentType = 0;
        m_handle = new uint[1];
        GL.GenBuffers(1, m_handle);
    }


    public uint Handle { get => (m_handle == null) ? 0 : m_handle[0]; }

    public void Bind()
    {
        GL.BindBuffer(m_bufferType, Handle);
    }

    public void Release()
    {
        GL.BindBuffer(m_bufferType, 0);
    }

    public void EnableAttribs()
    {
        if (m_index > -1)
            GL.EnableVertexAttribArray((uint)m_index);
    }

    public void DisableAttribs()
    {
        if (m_index > -1)
            GL.DisableVertexAttribArray((uint)m_index);
    }

    public void Describe()
    {
        if (m_index > -1)
        {
            EnableAttribs();
            GL.VertexAttribPointer((uint)m_index, m_componentCount, m_componentType, false, 0, IntPtr.Zero);
        }
    }

    int ComponentSize(uint componentType)
    {
        if (componentType == GL.FLOAT)
            return 4;
        if (componentType == GL.UNSIGNED_INT)
            return 4;
        if (componentType == GL.UNSIGNED_SHORT)
            return 2;
        return 4;
    }

    // data: buffer with OpenGL data (float or unsigned int)
    // dataSize: buffer size in bytes
    // componentType: OpenGL type of OpenGL data components (GL_FLOAT or GL_UNSIGNED_INT)
    // componentCount: Number of components of the primitives represented by the render data (3 for 3D vectors, 2 for texture coords, 4 for color values, ...)
    public void Create(String type, uint bufferType, int index, byte[] /*object*/ data, int dataSize, uint componentType, int componentCount = 1)
    {
        m_type = type;
        m_bufferType = bufferType;
        m_index = index;
        m_data = data;
        m_size = dataSize;
        m_itemSize = ComponentSize(componentType) * componentCount;
        m_itemCount = dataSize / m_itemSize;
        m_componentType = componentType;
        m_componentCount = componentCount;
        Bind();
        GL.BufferData(m_bufferType, m_data, GL.STATIC_DRAW);
        Describe();
    }

    public void Destroy()
    {
        if (Handle != 0)
        {
            GL.DeleteBuffers(1, m_handle);
            m_handle[0] = 0;
        }
    }
}

// =================================================================================================
