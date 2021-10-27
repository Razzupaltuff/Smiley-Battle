using System;
using System.Collections.Generic;

// =================================================================================================
// Mesh class definitions for basic mesh information, allowing to pass child classes to functions
// operating with or by meshes
// A mesh is defined by a set of faces, which in turn are defined by vertices, and a set of textures
// The only mesh used in Smiley Battle are ico spheres

public class AbstractMesh
{

    public virtual void Create(int quality, Texture texture, String[] textureNames)  { }

    public virtual void PushTexture(Texture texture)  { }

    public virtual void PopTexture()  { }

    public virtual Texture GetTexture()  { return null;  }

    public virtual void PushColor(Vector color)  { }

    public virtual void PopColor()  { }

    public virtual Vector GetColor()  { return null; }

    public virtual void Render()  { }
}

// =================================================================================================

public class Mesh : AbstractMesh
{
    public List<Texture> m_textures;
    public List<Vector> m_colors;
    public VertexBuffer m_vertices;
    public VertexBuffer m_normals;
    public TexCoordBuffer m_texCoords;
    public ColorBuffer m_vertexColors;
    public IndexBuffer m_indices;
    public VAO m_vao;
    public uint m_shape;
    public int m_shapeSize;

    public Mesh(uint shape = 0)
    {
        m_shape = (shape == 0) ? GL.TRIANGLES : shape;
        m_shapeSize = 3;
        m_textures = new List<Texture> ();
        m_colors = new List<Vector> ();
        m_vertices = new VertexBuffer();
        m_normals = new VertexBuffer();
        m_texCoords = new TexCoordBuffer();
        m_vertexColors = new ColorBuffer();
        m_indices = new IndexBuffer();
    }

    public override void Create(int quality, Texture texture, String[] textureNames) { }

    public void Destroy ()
    {
        m_vao.Destroy();
    }

    public int ShapeSize()
    {
        if (m_shape == GL.QUADS)
            return 4;
        if (m_shape == GL.TRIANGLES)
            return 3;
        if (m_shape == GL.LINES)
            return 2;
        return 1;
    }

    public void Init(uint shape, Texture texture = null, String[] textureNames = null, uint textureType = 0, Vector color = null)
    {
        m_shape = shape;
        m_shapeSize = ShapeSize();
        m_indices.m_componentCount = m_shapeSize;
        PushColor((color != null) ? color : new Vector (1,1,1));
        SetupTexture(texture, textureNames, textureType);
    }


    public void CreateVAO()
    {
        m_vao = new VAO();
        m_vao.Init(m_shape, GetTexture(), GetColor());
        m_vao.Enable();
        if (m_vertices.AppDataLength > 0)
        {
            m_vertices.Setup();
            m_vao.AddVertexBuffer("Vertex", m_vertices.GLData(), m_vertices.GLDataLength * 4, GL.FLOAT, 3);
        }
        if ((m_normals != null) && (m_normals.AppDataLength > 0))
        {
            m_normals.Setup();
            // in the case of an icosphere, the vertices also are the vertex normals
            m_vao.AddVertexBuffer("Normal", m_vertices.GLData(), m_vertices.GLDataLength * 4, GL.FLOAT, 3);
        }
        if ((m_texCoords != null) && (m_texCoords.AppDataLength > 0))
        {
            m_texCoords.Setup();
            m_vao.AddVertexBuffer("TexCoord", m_texCoords.GLData(), m_texCoords.GLDataLength * 4, GL.FLOAT, 2);
        }
        if ((m_vertexColors != null) && (m_vertexColors.AppDataLength > 0))
        {
            m_vertexColors.Setup();
            m_vao.AddVertexBuffer("Color", m_vertexColors.GLData(), m_vertexColors.GLDataLength * 4, GL.FLOAT, 4);
        }
        if ((m_indices != null) && (m_indices.AppDataLength > 0))
        {
            m_indices.Setup();
            m_vao.AddIndexBuffer(m_indices.GLData(), m_indices.GLDataLength * 2, GL.UNSIGNED_SHORT);
        }
        m_vao.Disable();
    }


    public void SetupTexture(Texture texture, String[] textureNames, uint textureType)
    {
        if ((textureNames != null) && (textureNames.Length > 0))
            m_textures = Globals.textureHandler.CreateByType(textureNames, textureType);
        else if (!Object.Equals (texture, null))
            m_textures.Add(texture);
    }


    public override void PushTexture(Texture texture)
    {
        if (!Object.Equals (texture, null))
            m_textures.Add(texture);
    }


    public override void PopTexture()
    {
        if (m_textures.Count > 0)
            m_textures.RemoveAt(m_textures.Count - 1);
    }


    public override Texture GetTexture()
    {
        return (m_textures.Count > 0) ? m_textures[m_textures.Count - 1] : null;
    }


    public override void PushColor(Vector color)
    {
        m_colors.Add (color);
    }


    public override void PopColor()
    {
        if (m_colors.Count > 0)
            m_colors.RemoveAt (m_colors.Count - 1);
    }


    public override Vector GetColor()
    {
        return (m_colors.Count > 0) ? m_colors [m_colors.Count - 1] : new Vector(1, 1, 1);
    }


    public bool EnableTexture()
    {
        Texture texture = GetTexture();
        if (!Object.Equals (texture, null))
            return false;
        texture.Enable();
        return true;
    }


    public void DisableTexture()
    {
        Texture texture = GetTexture();
        if (!Object.Equals (texture, null))
            texture.Disable();
    }


    public override void Render()
    {
        if (m_vao.IsValid())
        {
            SetTexture();
            SetColor();
            m_vao.Render();
        }
    }


    public void SetTexture()
    {
        m_vao.SetTexture(GetTexture());
    }


    public void SetColor()
    {
        m_vao.SetColor(GetColor());
    }

}

// =================================================================================================
