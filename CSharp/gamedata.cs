using System;
using System.Collections.Generic;
using SDL2;

// =================================================================================================

public class GameData
{
    public string m_resourceFolder;
    public string m_textureFolder;
    public string m_soundFolder;
    public string m_mapFolder;
    public string[] m_playerColors;
    public List<int> m_availableColors;
    public string[] m_playerMoods;

    public SortedDictionary<string, int> m_colorIndices;
    public SortedDictionary<string, Vector> m_colorValues;
    public SortedDictionary<string, Texture> m_textures;

    public int m_fireMode;
    public int m_fireDelay;
    public int m_healDelay;
    public int m_respawnDelay;
    public int m_immunityDuration;
    public int m_frozenTimeout;
    public float m_projectileSpeed;
    public float m_projectileSize;
    public bool m_wigglePlayers;
    public bool m_wiggleViewer;
    public int m_pointsForKill;
    public int m_gameTime;
    public int m_frameCap;
    public int m_minFrameTime;
    public bool m_isNetGame;
    public bool m_suspend;
    public bool m_run;


    public GameData()
    {
        m_resourceFolder = "resources\\";
        m_textureFolder = m_resourceFolder + "textures\\";
        m_soundFolder = m_resourceFolder + "sounds\\";
        m_mapFolder = "maps\\";

        m_playerColors = new string[]
        {
        "white", "black", "yellow", "gold",
        "orange", "red", "darkred", "pink",
        "purple", "lightgreen", "darkgreen", "lightblue",
        "blue", "darkblue", "lightgray", "darkgray"
            // "dead-black", "dead-white"
        };
        m_colorValues = new SortedDictionary<string, Vector>();

        m_colorValues.Add("white", new Vector(255, 255, 255) / 255.0f);
        m_colorValues.Add("black", new Vector(0, 0, 0));
        m_colorValues.Add("yellow", new Vector(255, 255, 0) / 255.0f);
        m_colorValues.Add("gold", new Vector(255, 217, 0) / 255.0f);
        m_colorValues.Add("orange", new Vector(255, 140, 0) / 255.0f);
        m_colorValues.Add("red", new Vector(217, 26, 0) / 255.0f);
        m_colorValues.Add("darkred", new Vector(128, 26, 0) / 255.0f);
        m_colorValues.Add("pink", new Vector(255, 127, 255) / 255.0f);
        m_colorValues.Add("purple", new Vector(153, 0, 255) / 255.0f);
        m_colorValues.Add("lightgreen", new Vector(102, 229, 0) / 255.0f);
        m_colorValues.Add("darkgreen", new Vector(0, 153, 38) / 255.0f);
        m_colorValues.Add("lightblue", new Vector(0, 204, 255) / 255.0f);
        m_colorValues.Add("blue", new Vector(0, 128, 255) / 255.0f);
        m_colorValues.Add("darkblue", new Vector(0, 26, 255) / 255.0f);
        m_colorValues.Add("lightgray", new Vector(172, 176, 176) / 255.0f);
        m_colorValues.Add("darkgray", new Vector(112, 112, 112) / 255.0f);

        m_colorIndices = new SortedDictionary<string, int> ();
        m_availableColors = new List<int>();
        for (int i = 0; i < m_playerColors.Length; i++)
        {
            m_availableColors.Add(i);
            m_colorIndices.Add(m_playerColors[i], i);
        }
        m_playerMoods = new string[] { "-sad",  "-neutral",  "-happy" };
        CreatePlayerTextures();

        m_fireMode = Globals.argHandler.IntVal("firemode", 0, 0);                  // limit fire rate to one short per 500 ms (2 shots/s)
        m_fireDelay = Globals.argHandler.IntVal("firedelay", 0, 250);                  // limit fire rate to one short per 500 ms (2 shots/s)
        m_healDelay = Globals.argHandler.IntVal("healdelay", 0, 5000);
        m_respawnDelay = Globals.argHandler.IntVal("respawndelay", 0, 5000);           // time [ms] between disappearing and reappearing after death
        m_immunityDuration = Globals.argHandler.IntVal("immunityduration", 0, 3000);   // duration [ms] of immunity after having respawned to allow for reorientation
        m_frozenTimeout = 5000;
        m_projectileSpeed = Globals.argHandler.FloatVal("projectilespeed", 0, 0.2f);
        m_projectileSize = Math.Min(1.0f, Globals.argHandler.FloatVal("projectilesize", 0, 0.3f));
        m_wigglePlayers = Globals.argHandler.BoolVal("wiggleplayers", 0, false);
        m_wiggleViewer = Globals.argHandler.BoolVal("wiggleviewer", 0, false);
        m_pointsForKill = Globals.argHandler.IntVal("pointsforkill", 0, 1);

        m_gameTime = (int) SDL.SDL_GetTicks();
        m_frameCap = Globals.argHandler.IntVal("framecap", 0, 240); // fps
        m_minFrameTime = (m_frameCap > 0) ? 1000 / m_frameCap : 0;
        m_isNetGame = false;
        m_suspend = false;
        m_run = true;
    }


    void CreatePlayerTextures()
    {
        m_textures = new SortedDictionary<string, Texture>();
        foreach (string color in m_playerColors)
        {
            foreach (string mood in m_playerMoods)
            {
                string skinName = m_textureFolder + "smiley-" + color + ".png";
                string faceName = m_textureFolder + "smileyface-" + color + mood + ".png";
                string noFile = "";
                Cubemap texture = Globals.textureHandler.GetCubemap();
                string[] fileNames = { skinName, noFile, noFile, noFile, noFile, faceName };
                texture.CreateFromFile(fileNames, false);
                // create test texturing with different colors on each side of a sphere
                // texture.CreateFromFile ([m_textureFolder + "smiley-white.png", 
                //                          m_textureFolder + "smiley-lightgreen.png", 
                //                          m_textureFolder + "smiley-red.png", 
                //                          m_textureFolder + "smiley-blue.png", 
                //                          m_textureFolder + "smiley-gold.png", 
                //                          m_textureFolder + "smileyface-black-happy.png"], 
                //                         false)
                m_textures.Add(color + mood, texture);
            }
        }
    }

   
    public bool GetPlayerColorValue(Player player, out Vector colorValue, out string color, bool whiteForBlack = false)
    {
        color = GetColor(player.GetColorIndex());
        if (whiteForBlack)
        {
            if (color == "black")
                color = "white";
            else
                whiteForBlack = false;
        }
        colorValue = m_colorValues.ContainsKey(color) ? m_colorValues[color].Clone () : new Vector(1, 1, 1);
        return whiteForBlack;
    }
    
    // randomly select a color index #include "the available color indices
    public int GetColorIndex()
    {
        // return "white"
        if (m_availableColors.Count == 0)
            return -1;
        int i = Globals.rand.Next (0, m_availableColors.Count);
        int colorIndex = m_availableColors[i];
        m_availableColors.RemoveAt(i);
        return colorIndex;
    }


    public int ReplaceColorIndex(int oldIndex, int newIndex)
    {
        if (newIndex != oldIndex)
        {
            ReturnColorIndex(oldIndex);
            RemoveColorIndex(newIndex);
        }
        return newIndex;
    }


    public string GetColor(int colorIndex)
    {
        return (colorIndex >= 0) ? m_playerColors[colorIndex] : "";
    }


    public Vector GetColorValue(string color)
    {
        return m_colorValues.ContainsKey(color) ? m_colorValues[color] : new Vector(1, 1, 1);
    }


    public void RemoveColorIndex(int colorIndex)
    {
        if (colorIndex >= 0)
            m_availableColors.Remove(colorIndex);
    }

    public void ReturnColorIndex(int colorIndex)
    {
        if (colorIndex >= 0)
            m_availableColors.Add(colorIndex);
    }

    public bool ColorIsAvailable(int colorIndex)
    {
        return m_availableColors.Contains(colorIndex);
    }

}

// =================================================================================================

