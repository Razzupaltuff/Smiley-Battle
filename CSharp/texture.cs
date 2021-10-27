using System;
using System.Collections.Generic;
using SDL2;

// =================================================================================================
// texture handling classes

public class AbstractTexture
{
    public virtual bool Create() { return false;  }

    public virtual void Destroy() { }

    public virtual bool Available() { return false;  }

    public virtual void Bind() { }

    public virtual void Release() { }

    public virtual void SetParams() { }

    public virtual void Deploy(int bufferIndex = 0) { }

    public virtual void Enable(int tmu = 0) { }

    public virtual void Disable() { }

    public virtual bool Load(string[] fileNames, bool flipVertically = true) { return false; }
}

// =================================================================================================
// texture handling: Loading from file, parameterization and sending to OpenGL driver, 
// enabling for rendering
// Base class for higher level textures (e.g. cubemaps)

public class Texture : AbstractTexture
{
    public uint[] m_handle;
    public uint m_type;
    public uint m_wrapMode;
    public bool m_useMipMaps;
    public List<TextureBuffer> m_buffers;
    public string[] m_fileNames;

    public Texture()
    {
        m_handle = new uint[1];
        m_buffers = new List<TextureBuffer>();
    }

    public Texture(uint type, uint wrapMode)
    {
        m_handle = new uint[1];
        m_type = type;
        m_wrapMode = wrapMode;
        m_buffers = new List<TextureBuffer>();
    }

    ~Texture()
    {
        Destroy();
    }

    public static bool operator ==(Texture a, Texture b)
    {
        return a.Handle == b.Handle;
    }

    public static bool operator !=(Texture a, Texture b)
    {
        return a.Handle != b.Handle;
    }

    public override bool Equals(Object obj)
    {
        //Check for null and compare run-time types.
        if ((obj == null) || !this.GetType().Equals(obj.GetType()))
        {
            return false;
        }
        else
        {
            Texture t = (Texture)obj;
            return Handle == t.Handle;
        }
    }

    public uint Handle { get => (m_handle == null) ? 0 : m_handle[0];  }

    public override int GetHashCode ()
    {
        return (int) Handle;
    }

    public int TextureCount { get => m_buffers.Count; }


    public int GetWidth(int i = 0)
    {
        return m_buffers[i].m_width;
    }

    public int GetHeight(int i = 0)
    {
        return m_buffers[i].m_height;
    }

    public uint Type()
    {
        return m_type;
    }

    public uint WrapMode()
    {
        return m_wrapMode;
    }


    public override bool Create()
    {
        Destroy();
        GL.GenTextures(1, m_handle);
        return (m_handle[0] > 0);
    }


    public override void Destroy()
    {
        if (Handle != 0)
        {
            Release();
            GL.DeleteTextures(1, m_handle);
            m_handle[0] = 0;
        }
    }

    public override bool Available()
    {
        return (Handle > 0) && (m_buffers.Count > 0);
    }


    public override void Bind()
    {
        if (Available())
        {
            GL.BindTexture(m_type, Handle);
        }
    }


    public override void Release()
    {
        if (Available())
            GL.BindTexture(m_type, 0);
    }


    public override void SetParams()
    {
        if (m_useMipMaps)
        {
            GL.TexParameter(m_type, GL.GENERATE_MIPMAP, GL.TRUE);
            GL.TexParameter(m_type, GL.TEXTURE_MIN_FILTER, GL.LINEAR_MIPMAP_LINEAR);
            GL.TexParameter(m_type, GL.TEXTURE_MAG_FILTER, GL.LINEAR);
        }
        else
        {
            GL.TexParameter(m_type, GL.GENERATE_MIPMAP, GL.FALSE);
            GL.TexParameter(m_type, GL.TEXTURE_MIN_FILTER, GL.LINEAR);
            GL.TexParameter(m_type, GL.TEXTURE_MAG_FILTER, GL.LINEAR);
        }
        GL.TexEnv(GL.TEXTURE_ENV, GL.TEXTURE_ENV_MODE, GL.REPLACE);
    }


    public void Wrap()
    {
        GL.TexParameter(GL.TEXTURE_2D, GL.TEXTURE_WRAP_S, m_wrapMode);
        GL.TexParameter(GL.TEXTURE_2D, GL.TEXTURE_WRAP_T, m_wrapMode);
    }


    public void Enable(uint tmu = 0)
    {
        GL.ActiveTexture(GL.TEXTURE0 + tmu);
        GL.Enable(m_type);
        Bind();
        SetParams();
        Wrap();
    }


    public override void Disable()
    {
        Release();
        GL.Disable(m_type);
    }


    public override void Deploy(int bufferIndex = 0)
    {
        if (Available())
        {
            Bind();
            SetParams();
            TextureBuffer texBuf = m_buffers[bufferIndex];
            GL.TexImage2D(m_type, 0, texBuf.m_internalFormat, texBuf.m_width, texBuf.m_height, 0, texBuf.m_format, GL.UNSIGNED_BYTE, texBuf.m_data);
            Release();
        }
    }

    // --------------------------------------------------------------------------------
    // Load loads textures from file. The texture filenames are given in filenames
    // An empty filename ("") means that the previously loaded texture should be used here as well
    // This makes sense e.g. for cubemaps if several of its faces share the same texture, like e.g. spherical smileys,
    // which have a face on one side and flat color on the remaining five sides of the cubemap used to texture them.
    // So a smiley cubemap texture list could be specified here like this: ("skin.png", "", "", "", "", "face.png")
    // This would cause the skin texture to be reused for in the place of the texture data buffers at positions 2, 3, 4
    // and 5. You could also do something like ("skin.png", "", "back.png", "", "face.png") or just ("color.png", "", "", "", "", "")
    // for a uniformly textured sphere. The latter case will however be also taken into regard by the cubemap class.
    // It allows to pass a single texture which it will use for all faces of the cubemap

    public override bool Load(string[] fileNames, bool flipVertically = true)
    {
        // load texture from file
        m_fileNames = new string[fileNames.Length];
        TextureBuffer texBuf = null;
        int i = 0;
        foreach (string fileName in fileNames)
        {
            m_fileNames[i] = fileName.Substring (0);
            if (fileName.Length == 0)
            {
                TextureBuffer t = new TextureBuffer();
                t.Copy(texBuf, TextureBuffer.eCopyMode.eCopy);
                m_buffers.Add(t);
            }
            else
            {
                IntPtr image = SDL_image.IMG_Load(fileName);
                if (image == null)
                {
                    Console.Error.WriteLine("Couldn't find '{0}'", fileName);
                    return false;
                }
                texBuf = new TextureBuffer();
                texBuf.Create(image);
                m_buffers.Add(texBuf);
            }
        }
        return true;
    }


    public bool CreateFromFile(string[] fileNames, bool flipVertically = true)
    {
        if (!Create())
            return false;
        if (fileNames.Length == 0)
            return true;
        if (!Load(fileNames, flipVertically))
            return false;
        Deploy();
        return true;
    }


    public bool CreateFromSurface(IntPtr surface)
    {
        if (!Create())
            return false;
        m_buffers.Add(new TextureBuffer(surface));
        return true;
    }

}

// =================================================================================================
