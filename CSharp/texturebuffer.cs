using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using SDL2;

// =================================================================================================
// texture data buffer handling

public class TextureBuffer
{
    public enum eCopyMode
    {
        eClone,
        eMove,
        eCopy
    }
    
    public int m_width;
    public int m_height;
    public int m_componentCount;
    public uint m_internalFormat;
    public uint m_format;
    public int m_dataSize;
    public byte[] m_data;

    public TextureBuffer() 
    {
        m_data = null;
    }

    ~TextureBuffer()
    {
        m_data = null;
    }

    public TextureBuffer(IntPtr imagePtr)
    {
        Create(imagePtr);
    }


    uint SDL_PixelFormat (uint type, uint order, uint layout, uint bits, uint bytes)
    {
        return (1 << 28) | (type << 24) | (order << 20) | (layout << 16) | (bits << 8) | (bytes << 0);

    }


    public TextureBuffer Create(IntPtr imagePtr)
    {
        var image = (SDL.SDL_Surface) Marshal.PtrToStructure(imagePtr, typeof(SDL.SDL_Surface));
        m_width = image.w;
        m_height = image.h;
        m_componentCount = image.pitch / image.w;
        if (m_componentCount < 3)
        {
            imagePtr = SDL.SDL_ConvertSurfaceFormat(imagePtr, SDL_PixelFormat (6, 4, 6, 32, 4), 0);
            image = (SDL.SDL_Surface)Marshal.PtrToStructure(imagePtr, typeof(SDL.SDL_Surface));
            m_componentCount = image.pitch / image.w;
        }
        m_internalFormat = (m_componentCount == 4) ? GL.RGBA : GL.RGB;
        m_format = (m_componentCount == 4) ? GL.RGBA : GL.RGB;
        m_dataSize = m_width * m_height * m_componentCount;
        m_data = new byte [m_dataSize];
        Marshal.Copy (image.pixels, m_data, 0, m_dataSize);
        SDL.SDL_FreeSurface(imagePtr);
        return this;

    }


    public TextureBuffer Copy(TextureBuffer other, eCopyMode mode = eCopyMode.eClone)
    {
        m_width = other.m_width;
        m_height = other.m_height;
        m_componentCount = other.m_componentCount;
        m_internalFormat = other.m_internalFormat;
        m_format = other.m_format;
        m_dataSize = other.m_dataSize;
        if (mode == eCopyMode.eClone)
        {
            m_data = new byte [m_dataSize];
            other.m_data.CopyTo (m_data, 0);
        }
        else
        {
            m_data = other.m_data;
            if (mode == eCopyMode.eMove)
                other.m_data = null;
        }
        return this;
    }

}

// =================================================================================================
