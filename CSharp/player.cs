using System;
using System.Collections.Generic;

// =================================================================================================
// Player actor. Standard actor with a few extra properties: Shadow, outline, firing shots, changing
// color when hit, dieing animation

public class Player : Actor
{

    public PlayerShadow m_shadow;
    public PlayerOutline m_outline;
    public PlayerHalo m_halo;
    public Mesh m_projectileMesh;
    new public List<Texture> m_textures;
    public string[] m_moods;
    public string m_color;
    public Vector m_colorValue;
    public int m_colorIndex;
    public bool m_whiteForBlack;
    public string m_address;
    public ushort[] m_ports;
    public int m_lastMessageTime;
    public bool m_isConnected;
    public int m_score;
    public int m_kills;
    public int m_deaths;
    public Timer m_wiggleTimer;
    public int m_wiggleAngle;


    public Player(string type, int colorIndex = -1, PlayerShadow shadow = null, PlayerHalo halo = null, PlayerOutline outline = null, bool isViewer = false)
        : base(type, 3, isViewer)
    {
        m_shadow = shadow;
        m_outline = outline;
        m_halo = halo;
        m_colorIndex = -1;
        m_whiteForBlack = false;
        SetColorIndex(colorIndex);
        m_moods = new string[] { "-sad", "-neutral", "-happy" };
        m_colorValue = new Vector(1, 1, 1);
        m_color = "white";
        m_address = "127.0.0.1";
        m_ports = new ushort[] { 0, 0 };
        m_lastMessageTime = 0;    // time when the last network message #include "this player was received
        m_isConnected = false;    // remote player is connected with local player
        m_score = 0;
        m_kills = 0;
        m_deaths = 0;
        m_wiggleAngle = Globals.rand.Next(0, 180);
        m_wiggleTimer = new Timer();
    }


    public new void Destroy()
    {
        // m_shadow.Destroy();
        // m_halo.Destroy();
        base.Destroy();
    }


    public void Create(string name, Mesh mesh, int quality, SortedDictionary<string, Texture> textures, string[] textureNames, Vector position, Vector angles, float size, Camera parent = null)
    {
        SetupTextures(textures, textureNames);
        base.Create(name, mesh, quality, null, null, position, angles, size, parent);
    }

    public void SetupTextures(SortedDictionary<string, Texture> textures, string[] textureNames = null)
    {
        string color = Globals.gameData.GetColor(m_colorIndex);
        if (color != "black")
            color = "white";
        m_textures = new List<Texture>();
        foreach (string mood in m_moods)
            m_textures.Add(textures[color + mood]);
    }


    public void SetColorIndex(int colorIndex, bool replace = false)
    {
        if (replace && (m_colorIndex >= 0))
            Globals.gameData.ReturnColorIndex(m_colorIndex);
        if (replace && (colorIndex >= 0))
            Globals.gameData.RemoveColorIndex(colorIndex);
        m_colorIndex = colorIndex;
        if (m_colorIndex >= 0)
            m_whiteForBlack = Globals.gameData.GetPlayerColorValue(this, out m_colorValue, out m_color, true);
        else
        {
            m_color = "white";
            m_colorValue = new Vector(1, 1, 1);
            m_whiteForBlack = false;
        }
    }


    public override int Mood(int mood = -1)
    {
        if (mood > -1)
            return mood;
        if (IsImmune())
            return 2;
        if (m_hitPoints > 0)
            return m_hitPoints - 1;
        return 0;
    }


    public void SetProjectileMesh(Mesh mesh)
    {
        m_projectileMesh = mesh;
    }


    public override Mesh GetProjectileMesh()
    {
        return m_projectileMesh;
    }


    public void SetShadow(PlayerShadow shadow)
    {
        m_shadow = shadow;
    }


    public void SetOutline(PlayerOutline outline)
    {
        m_outline = outline;
    }


    public void SetHalo(PlayerHalo halo)
    {
        m_halo = halo;
    }


    public override int GetColorIndex()
    {
        return m_colorIndex;
    }

    public override Vector GetColorValue()
    {
        return m_colorValue;
    }


    public override string GetColor()
    {
        return m_color;
    }


    public override Vector GetPlayerColorValue()
    {
        return m_colorValue;
    }


    public void SetAddress(string address, ushort[] ports)
    {
        m_address = address;
        ports.CopyTo(m_ports, 0);
    }


    public override string GetAddress()
    {
        return m_address;
    }


    public override ushort GetPort(int i = 0)
    {
        return m_ports[i];
    }


    public void SetPort(ushort port, int i = 0)
    {
        m_ports[i] = port;
    }


    public override Texture GetTexture(int mood = -1)
    {
        return m_textures[Mood(mood)];
    }


    public override float BorderScale()
    {
        if (m_outline != null)
            return m_outline.Scale();
        return 1.0f;
    }


    public override void AddScore(int points)
    {
        m_score += points;
    }


    public override void SetScore(int score)
    {
        m_score = score;
    }


    public override int GetScore()
    {
        return m_score;
    }

    public override void AddKill()
    {
        m_kills++;
    }


    public override void AddDeath()
    {
        m_deaths++;
    }


    public override void Render(bool autoCamera = true)
    {
        if (IsHidden())
            return;
        EnableCamera();
        if (m_scale < 1.0f)
            GL.Scale(m_scale, 1.0f, m_scale);
        if (m_shadow != null)
            m_shadow.Render(GetPosition().Y);
        Wiggle();
        if ((m_halo != null) && IsImmune())
            m_halo.Render(m_size * 0.55f);
        if (m_scale < 1.0f)
            GL.Scale(1.0f, m_scale, 1.0f);
        SetTexture(GetTexture());
        m_mesh.PushColor(m_colorValue);
        base.Render(false);
        m_mesh.PopColor();
        if (m_outline != null)
            m_outline.Render(m_camera.GetSize(), m_whiteForBlack ? 1 : 0);
        DisableCamera();
    }


    public void Wiggle()
    {
        if (IsViewer() && !Globals.gameData.m_wiggleViewer)
            return;
        if (!IsPlayer() || !Globals.gameData.m_wigglePlayers)
            return;
        if (m_wiggleTimer.HasPassed(10, true))
        {
            m_wiggleAngle += 5;
            m_wiggleAngle %= 360;
        }
        GL.Translate(0, (float) Math.Sin(m_camera.Rad(m_wiggleAngle)) / 20.0f, 0);
    }


    public override void UpdateLastMessageTime()
    {
        m_lastMessageTime = Globals.gameData.m_gameTime;
    }

}

// =================================================================================================
