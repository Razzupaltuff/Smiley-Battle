using System;
using System.Collections.Generic;
using SDL2;

// =================================================================================================

public class ScoreBoard
{
    string[] m_textureNames;
    List<Texture> m_textures;
    List<Texture> m_digitTextures;
    List<Quad> m_digitQuads;
    Quad m_statusBackground;
    Quad m_statusSmiley;
    bool m_coloredScore;

    public ScoreBoard()
    {
        m_textureNames = new string[]
        {
        "smileyface-mask-black-sad.png", "smileyface-mask-black-neutral.png", "smileyface-mask-black-happy.png", "smiley-mask-black.png",
        "smileyface-mask-white-sad.png", "smileyface-mask-white-neutral.png", "smileyface-mask-white-happy.png", "smiley-mask-white.png",
        "smiley-strikeout-red.png", "smiley-strikeout-yellow.png"
        };
        Create();
        CreateDigitTextures();
        CreateStatusBackground();
        CreateStatusSmiley();
        CreateDigitQuads();
        m_coloredScore = Globals.argHandler.BoolVal("coloredscore", 1, false);
    }


    public void Create()
    {
        m_textures = Globals.textureHandler.CreateTextures(m_textureNames);
    }


    public void CreateDigitTextures()
    {
        string[] digits = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };
        m_digitTextures = new List<Texture>();
        for (int i = 0; i < digits.Length; i++)
        {
            Texture texture = Globals.textureHandler.GetTexture();
            if (!texture.CreateFromSurface(SDL_ttf.TTF_RenderText_Solid(Globals.renderer.m_scoreFont, digits[i], new SDL.SDL_Color() { r = 224, g = 224, b = 224, a = 255 })))
                continue;
            texture.Create();
            texture.Deploy();
            m_digitTextures.Add(texture);
        }
    }


    public void CreateStatusBackground()
    {
        m_statusBackground = new Quad();
        float border = 0.1f;
        m_statusBackground.Init(
            new Vector[] {
                new Vector(border, border, 0.0f),
                new Vector(border, 1.0f - border, 0.0f),
                new Vector(1.0f - border, 1.0f - border, 0.0f),
                new Vector(1.0f - border, border, 0.0f) });
        m_statusBackground.Create();
    }


    public void CreateStatusSmiley()
    {
        m_statusSmiley = new Quad();
        float border = 0.2f;
        m_statusSmiley.Init(
            new Vector[] {
                new Vector(border, border, 0.0f),
                new Vector(border, 1.0f - border, 0.0f),
                new Vector(1.0f - border, 1.0f - border, 0.0f),
                new Vector(1.0f - border, border, 0.0f) });
        m_statusSmiley.Create();
    }


    Quad CreateDigitQuad(float l, float w, float h)
    {
        float border = 0.0f; //w * 0.1f
        Quad q = new Quad(
            new Vector[] {
                new Vector(l + border, h + border, 0.0f),
                new Vector(l + border, 1.0f - h - border, 0.0f),
                new Vector(l + w - border, 1.0f - h - border, 0.0f),
                new Vector(l + w - border, h + border, 0.0f) });
        q.Create();
        return q;
    }


    public void CreateDigitQuads()
    {
        Renderer.Viewport vp = Globals.renderer.SetViewport("score", 1);
        vp.m_width /= 5;     // 4 characters + one space
        Texture t = m_digitTextures[0];
        float l = 0;
        m_digitQuads = new List<Quad> ();
        for (int i = 0; i < 4; i++)
        {
            float cw = (float)t.GetWidth();
            float ch = (float)t.GetHeight();
            float ar = cw / ch;
            float h, w = ar * (float)vp.m_height;
            if (w <= (float)vp.m_width)
            {
                w = 0.225f * w / (float)vp.m_width;
                h = 1.0f;
            }
            else
            {
                w = 0.225f;
                h = (float)vp.m_width / ar;
                if (h > (float)vp.m_height)
                    w *= (float)vp.m_height / h;
                h = 1.0f;
            }
            m_digitQuads.Add(CreateDigitQuad(l, w, (1.0f - h) / 2));
            l += w;
        }
    }


    // The status icon is painted by first drawing a rectangle in the corresponding player's color,
    // then masking the corners to create a colored circle and finally drawing a smiley face on top of it.
    // If the player is currently dead, a strikeout will be painted over it
    // Black smiley get a circular mask with a white border to make them visible against the black status area background
    public void RenderStatus(Player player, int position)
    {
        Globals.renderer.SetViewport("status", position);
        Vector colorValue;
        string color;
        bool whiteForBlack = Globals.gameData.GetPlayerColorValue(player, out colorValue, out color);
        GL.DepthFunc(GL.ALWAYS);
        GL.UseProgram(0);
        m_statusBackground.Fill(colorValue);
        int textureOffset = (color == "black") ? 4 : 0;
        m_statusBackground.SetTexture(m_textures[textureOffset + 3]);
        m_statusBackground.Render();
        m_statusSmiley.SetTexture(m_textures[textureOffset + player.Mood()]);
        m_statusSmiley.Render();
        if (player.IsHidden())
        {
            if ((color == "red") || (color == "darkred"))
                m_statusSmiley.SetTexture(m_textures[9]);
            else
                m_statusSmiley.SetTexture(m_textures[8]);
            m_statusSmiley.Render();
        }
        GL.DepthFunc(GL.LESS);
    }



    int Pot10(int i)
    {
        int b = 1;
        while (b <= i)
            b *= 10;
        return b / 10;
    }


    public void RenderScore(int position, Player player, int score)
    {
        if (position > 0)
            Globals.renderer.SetViewport("score", position);
        bool whiteForBlack = true;
        Vector colorValue;
        string color;
        if (!m_coloredScore)
            colorValue = new Vector(1, 1, 1);
        else
            whiteForBlack = Globals.gameData.GetPlayerColorValue(player, out colorValue, out color, whiteForBlack);
        GL.DepthFunc(GL.ALWAYS);
        int b = Pot10(score);
        if (b < 1000)
            b = 1000;
        for (int i = 0; b > 0; b /= 10, i++)
        {
            int d = score / b;
            m_digitQuads[i].SetTexture(m_digitTextures[d]);
            m_digitQuads[i].SetColor(colorValue);
            m_digitQuads[i].Render();
            score %= b;
        }
        GL.DepthFunc(GL.LESS);
    }


    public void RenderViewerStatus()
    {
        RenderStatus(Globals.actorHandler.m_viewer, 0);
    }


    public void RenderPlayerStatus()
    {
        int position = 0;
        foreach (Actor a in Globals.actorHandler.m_actors)
            if (a.IsPlayer() && !a.IsViewer())
                RenderStatus((Player)a, ++position);
    }


    public void RenderViewerScores()
    {
        Globals.renderer.SetViewport("score", 0);
        RenderScore(-1, Globals.actorHandler.m_viewer, Globals.actorHandler.m_viewer.m_score);
        Globals.renderer.SetViewport("kills", 0);
        RenderScore(-1, Globals.actorHandler.m_viewer, Globals.actorHandler.m_viewer.m_kills);
        Globals.renderer.SetViewport("deaths", 0);
        RenderScore(-1, Globals.actorHandler.m_viewer, Globals.actorHandler.m_viewer.m_deaths);
    }


    public void RenderPlayerScores()
    {
        int position = 0;
        foreach (Actor a in Globals.actorHandler.m_actors)
            if (a.IsPlayer() && !a.IsViewer())
                RenderScore(++position, (Player)a, ((Player)a).m_score);
    }


    public void Render()
    {
        RenderViewerStatus();
        RenderPlayerStatus();
        RenderViewerScores();
        RenderPlayerScores();
        Globals.renderer.SetViewport();
    }

}

// =================================================================================================
