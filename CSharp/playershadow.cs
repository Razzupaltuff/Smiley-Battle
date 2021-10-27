using System;

// =================================================================================================
// Shadow for players (smileys). Just a 2D texture rendered near the ground

public class PlayerShadow : Quad
{
    public float m_groundClearance;


    public PlayerShadow()
      : base(
          new Vector[] { 
            new Vector(-0.5f, 0.0f, 0.5f), 
              new Vector(-0.5f, 0.0f, -0.5f), 
              new Vector(0.5f, 0.0f, -0.5f), 
              new Vector(0.5f, 0.0f, 0.5f) })
    {
        m_groundClearance = 0.0002f;
        base.SetTexture (CreateTexture());
    }


    public Texture CreateTexture()
    {
        Texture texture = Globals.textureHandler.GetTexture();
        string[] fileName = { Globals.gameData.m_textureFolder + "shadow.png" };
        if (texture.CreateFromFile(fileName))
            return texture;
        Globals.textureHandler.Remove(texture);
        return null;
    }


    public void Render(float offset)
    {
        GL.PushMatrix();
        GL.Disable(GL.CULL_FACE);
        GL.Translate(0.0f, offset + m_groundClearance, 0.0f);
        base.Render();
        GL.Enable(GL.CULL_FACE);
        GL.PopMatrix();
    }

}

// =================================================================================================
