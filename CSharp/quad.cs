using System;

// =================================================================================================

public class Quad
{
    public VertexBuffer m_vertices;
    public TexCoordBuffer m_texCoords;
    public Texture m_texture;
    public Vector m_color;
    public VAO m_vao;
    public bool m_useVAO;

    public Quad(bool useVAO = true) 
    {
        m_vertices = new VertexBuffer();
        m_texCoords = new TexCoordBuffer();
        m_useVAO = useVAO;
    }

    public Quad(Vector[] vertices, Texture texture = null, Vector color = null)
    {
        m_vertices = new VertexBuffer();
        m_texCoords = new TexCoordBuffer();
        m_vertices.Init();
        m_texCoords.Init();
        foreach (Vector v in vertices)
            m_vertices.Append (v);
        m_texture = texture;
        m_color = (color == null) ? new Vector (1,1,1) : color;
    }


    public void Init (Vector[] vertices, Texture texture = null, Vector color = null)
    {
        m_vertices.Init();
        foreach (Vector v in vertices)
            m_vertices.Append(v);
        m_texture = texture;
        m_color = (color == null) ? new Vector(1, 1, 1) : color;
    }


    public void Create()
    {
        CreateTexCoords();
        CreateVAO();
    }


    public void Destroy()
    {
        m_vao.Destroy();
        Globals.textureHandler.Remove(m_texture);
    }

    public void SetTexture(Texture texture)
    {
        m_texture = texture;
    }

    public void SetColor(Vector color)
    {
        m_color = color;
    }

    public void CreateTexCoords()
    {
        if (!Equals (m_texture, null) && (m_texture.WrapMode() == GL.REPEAT))
        {
            foreach (Vector v in m_vertices.m_appData)
                   m_texCoords.Append(new TexCoord(v.X, v.Z));
        }
        else
        {
            m_texCoords.Append(new TexCoord(0, 1));
            m_texCoords.Append(new TexCoord(0, 0));
            m_texCoords.Append(new TexCoord(1, 0));
            m_texCoords.Append(new TexCoord(1, 1));
        }
    }


    public void CreateVAO()
    {
        if (m_vertices.m_appData.Count > 0)
        {
            m_vao = new VAO();
            m_vao.Init(GL.QUADS, m_texture, m_color);
            m_vertices.Setup();
            m_texCoords.Setup();
            m_vao.Enable();
            m_vao.AddVertexBuffer("Vertex", m_vertices.GLData(), m_vertices.GLDataLength * 4, GL.FLOAT, 3);
            m_vao.AddVertexBuffer("TexCoord", m_texCoords.GLData(), m_texCoords.GLDataLength * 4, GL.FLOAT, 2);
            m_vao.Disable();
        }
    }


    public void Render()
    {
        if (m_vao.IsValid())
        {
            m_vao.SetTexture(m_texture);
            m_vao.SetColor(m_color);
            m_vao.Render(true);
        }
    }


    // fill 2D area defined by x and y components of vertices with color color
    public void Fill(Vector color, float alpha = 1.0f)
    {
        if (m_useVAO)
        {
            m_vao.SetTexture(null);
            m_vao.SetColor(color);
            m_vao.Render();
        }
        else
        {
            color.A = alpha;
            GL.Disable(GL.TEXTURE_2D);
            GL.Begin(SharpGL.Enumerations.BeginMode.Quads);
            foreach (Vector v in m_vertices.m_appData)
            {
                GL.Color4fv(color);
                GL.Vertex2f(v.X, v.Y);
            }
            GL.End();
        }
    }

}

// =================================================================================================
