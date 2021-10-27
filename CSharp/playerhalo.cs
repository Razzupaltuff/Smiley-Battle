using System;

// =================================================================================================
// Shadow for players (smileys). Just a 2D texture rendered near the ground

public class PlayerHalo : Torus
{
    public float m_tolerance;

    public PlayerHalo()
        : base(null, new string[] { "white.png" }, Globals.gameData.GetColorValue("gold"))
    {
        m_tolerance = 0.0001f;
    }

    public void Render(float offset)
    {
        GL.PushMatrix();
        GL.Disable(GL.CULL_FACE);
        GL.Translate(0.0f, offset + m_tolerance, 0.0f);
        GL.Scale(0.667f, 1.0f, 0.667f);
        base.Render();
        GL.Enable(GL.CULL_FACE);
        GL.PopMatrix();
    }

}

// =================================================================================================
