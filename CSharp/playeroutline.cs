using System;
using System.Collections.Generic;

// =================================================================================================
// Outline for players (smileys). The outline is created by rendering the backwards facing faces of
// the sphere in black && a tad bigger than the player smileys.
// The outline is rendered after the smiley to improve performance, since the z culling will discard
// most of the render data (pixels) early

public class PlayerOutline
{
    public Mesh m_mesh;
    public float m_scale;
    public string[] m_textureNames;
    public List<Texture> m_textures;

    public float Scale()
    {
        return m_scale;
    }

    public PlayerOutline()
    {
        m_scale = 1.02f;
        m_textureNames = new string[] { "black.png", "white.png" };
    }



    public void Create(Mesh mesh)
    {
        m_mesh = mesh;
        m_textures = Globals.textureHandler.CreateCubemaps(m_textureNames);
    }


    public void Render(float size, int colorIndex = 0)
    {
        GL.PushMatrix();
        GL.Scale(size, size, size);
        GL.CullFace(GL.FRONT);
        m_mesh.PushTexture(m_textures[colorIndex]);
        m_mesh.m_vao.SetMinBrightness(0.9f);
        m_mesh.Render();
        m_mesh.m_vao.SetMinBrightness(0.0f);
        m_mesh.PopTexture();
        GL.CullFace(GL.BACK);
        GL.PopMatrix();
    }
}

// =================================================================================================
