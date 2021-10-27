using System;
using System.Collections.Generic;

// =================================================================================================
// Render a reticle on the scene. Requires an orthogonal (not perspective) projection.
// Reticle renderer needs to be called after the 3D renderer is done.

public class Reticle : Quad
{
    public string[] m_textureNames;
    public List<Texture> m_textures;

    public Reticle() : base(new Vector[] { new Vector(-0.125f, -0.125f, 0.0f), new Vector(-0.125f, +0.125f, 0.0f), new Vector(+0.125f, +0.125f, 0.0f), new Vector(+0.125f, -0.125f, 0.0f) })
    {
        m_textureNames = new string[] { "reticle-darkgreen.png", "reticle-lightgreen.png" };
        CreateTextures();
    }


    public new void Create()
    {
        base.Create();
        CreateTextures();
    }


    public new void Destroy()
    {
        base.Destroy();
    }


    public void CreateTextures()
    {
        m_textures = Globals.textureHandler.CreateTextures(m_textureNames);
    }


    public new void Render()
    {
        if (m_textures.Count == 0)
            return;
        SetTexture(m_textures[Convert.ToInt32(Globals.gameItems.m_viewer.ReadyToFire())]);
        GL.DepthFunc(GL.ALWAYS);
        GL.Disable(GL.CULL_FACE);
        GL.PushMatrix();
        GL.Translate(0.5f, 0.5f, 0f);
        GL.Scale(0.125f / Globals.renderer.m_aspectRatio, 0.125f, 1.0f);
        base.Render();
        GL.PopMatrix();
        GL.Enable(GL.CULL_FACE);
        GL.DepthFunc(GL.LESS);
    }

}

// =================================================================================================
