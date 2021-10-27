using System;
using System.Collections.Generic;

// =================================================================================================

public class VAOHandler
{
    List<VAO> m_vaos;

    public VAOHandler()
    {
        m_vaos = new List<VAO>();
    }

    ~VAOHandler()
    {
        Destroy();
    }


    public void Register (VAO vao)
    {
        m_vaos.Add(vao);
    }

    public bool Unregister(VAO vao)
    {
        for (int i = 0; i < m_vaos.Count; i++)
            if (m_vaos [i] == vao)
            {
                m_vaos.RemoveAt(i);
                return true;
            }
        return false;
    }


    public void Destroy ()
        {
        for (int i = m_vaos.Count; i > 0;)
            m_vaos [--i].Destroy();
    }
}

// =================================================================================================
// "Premium version of" OpenGL vertex array objects. VAO instances offer methods to convert python
// lists into the corresponding lists of OpenGL items (vertices, normals, texture coordinates, etc)
// The current implementation requires a fixed order of array buffer creation to comply with the 
// corresponding layout positions in the shaders implemented here.
// Currently offers shaders for cubemap and regular (2D) texturing.
// Implements loading of varying textures, so an application item derived from or using a VAO instance
// (e.g. an ico sphere) can be reused by different other application items that require different 
// texturing. This implementation basically allows for reusing one single ico sphere instance whereever
// a sphere is needed.
// Supports indexed and non indexed vertex buffer objects.
//
// // Due to the current shader implementation (fixed position layout), buffers need to be passed in a
// fixed sequence: vertices, colors, ...
// TODO: Expand shader for all kinds of inputs (texture coordinates, normals)
// See also https://qastack.com.de/programming/8704801/glvertexattribpointer-clarification

public class VAO
{
    List<VBO> m_dataBuffers;
    VBO m_indexBuffer;
    uint[] m_handle;
    uint m_shape;
    Texture m_texture;
    Vector m_color;
    float m_minBrightness;


    public VAO()
    {
        Globals.vaoHandler.Register(this);
        m_color = new Vector(1, 1, 1);
        m_dataBuffers = new List<VBO>();
        m_handle = new uint[1];
        GL.GenVertexArrays(1, m_handle);
    }

    public uint Handle { get => (m_handle == null) ? 0 : m_handle[0]; }

    ~VAO()
    {
        Destroy();
    }


    public bool IsValid()
    {
        return Handle != 0;
    }

    public void Enable()
    {
        GL.BindVertexArray(Handle);
    }

    public void Disable()
    {
        if (Handle != 0)
            GL.BindVertexArray(0);
    }

    public void SetTexture(Texture texture)
    {
        m_texture = texture;
    }

    public void SetColor(Vector color)
    {
        m_color = color;
    }

    public void SetMinBrightness(float minBrightness)
    {
        m_minBrightness = minBrightness;
    }

    public void ResetMinBrightness()
    {
        m_minBrightness = 0;
    }

    public Texture EnableTexture() {
        if (Equals (m_texture, null))
            return null;
        m_texture.Enable();
        return m_texture;
    }

    public void DisableTexture()
    {
        if (!Equals (m_texture, null))
            m_texture.Disable();
    }


    public void Init(uint shape, Texture texture, Vector color)
    {
        m_shape = shape;
        m_texture = texture;
        m_color = color;
        m_minBrightness = 0.0f;
    }


    public void Destroy()
    {
        Disable();
        foreach (VBO vbo in m_dataBuffers)
            vbo.Destroy();
        if (Handle > 0)
        {
            GL.DeleteVertexArrays(1, m_handle);
            m_handle[0] = 0;
        }
        Globals.vaoHandler.Unregister(this);
    }


    // add a vertex or index data buffer
    public void AddBuffer(String type, byte[] /*object*/ data, int dataSize, uint componentType, int componentCount)
    {
        if (type == "Index")
            AddIndexBuffer(data, dataSize, componentType);
        else
            AddVertexBuffer(type, data, dataSize, componentType, componentCount);
    }


    public void AddVertexBuffer(String type, byte[] /*object*/ data, int dataSize, uint componentType, int componentCount)
    {
        VBO buffer = new VBO();
        buffer.Create(type, GL.ARRAY_BUFFER, m_dataBuffers.Count, data, dataSize, componentType, componentCount);
        m_dataBuffers.Add(buffer);
    }


    public void AddIndexBuffer(byte[] /*object*/ data, int dataSize, uint componentType)
    {
        m_indexBuffer = new VBO();
        m_indexBuffer.Create("Index", GL.ELEMENT_ARRAY_BUFFER, -1, data, dataSize, componentType);
    }


    public void Render(bool useShader = true)
    {
        int shaderId = Globals.shaderHandler.SelectShader(useShader, EnableTexture());
        Globals.shaderHandler.Shader(shaderId).SetUniformVector("fillColor", m_color);
        if (shaderId == 0)
        {
            Globals.shaderHandler.Shader(shaderId).SetUniformFloat("minBrightness", m_minBrightness);
        }
        Enable();
        if ((m_indexBuffer == null) || (m_indexBuffer.m_data == null))
            GL.DrawArrays(m_shape, 0, m_dataBuffers[0].m_itemCount); // draw non indexed arrays
        else
            GL.DrawElements(m_shape, m_indexBuffer.m_itemCount, m_indexBuffer.m_componentType, IntPtr.Zero); // draw using an index buffer
        Disable();
        if (shaderId > -1)
            GL.UseProgram(0);
        DisableTexture();
    }

}

// =================================================================================================
