using System;
using System.Collections.Generic;

// =================================================================================================
// Very simply class for texture tracking
// Main purpose is to keep track of all texture objects in the game and return them to OpenGL in
// a well defined and controlled way at program termination without having to bother about releasing
// textures at a dozen places in the game

public class TextureHandler
{
    public List<Texture> m_textures;

    public TextureHandler()
    {
        m_textures = new List<Texture>();
    }

    ~TextureHandler()
    {
        Destroy();
    }

    public void Destroy()
    {
        foreach (Texture t in m_textures)
            t.Destroy ();
    }


    public Texture GetTexture()
    {
        Texture t = new Texture(GL.TEXTURE_2D, GL.CLAMP_TO_EDGE);
        m_textures.Add (t);
        return t;
    }


    public bool Remove(Texture texture)
    {
        if (Equals (texture, null))
            return false;
        m_textures.Remove(texture);
        texture.Destroy();
        return true;
    }


    public Cubemap GetCubemap()
    {
        Cubemap t = new Cubemap();
        m_textures.Add(t);
        return t;
    }


    public List<Texture> CreateTextures(string[] textureNames)
    {
        List<Texture> textures = new List<Texture> ();
        string[] fileName = new string[1];
        foreach (string n in textureNames)
        {
            Texture t = GetTexture();
            fileName [0] = Globals.gameData.m_textureFolder + n;
            if (!t.CreateFromFile(fileName))
                break;
            textures.Add(t);
        }
        return textures;
    }


    public List<Texture> CreateCubemaps(string[] textureNames)
    {
        List<Texture> textures = new List<Texture> ();
        string[] fileName = new string[1];
        foreach (string n in textureNames)
        {
            Cubemap t = GetCubemap();
            textures.Add(t);
            fileName [0] = Globals.gameData.m_textureFolder + n;
            if (!t.CreateFromFile(fileName))
                break;
        }
        return textures;
    }


    public List<Texture> Create(string[] textureNames, uint textureType)
    {
        return (textureType == GL.TEXTURE_CUBE_MAP) ? CreateCubemaps(textureNames) : CreateTextures(textureNames);
    }


    public List<Texture> CreateByType(string[] textureNames, uint textureType)
    {
        return Create(textureNames, textureType);
    }

}

// =================================================================================================
