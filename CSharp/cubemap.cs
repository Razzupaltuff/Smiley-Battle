using System;

// =================================================================================================
// Load cubemap textures from file and generate an OpenGL cubemap 

public class Cubemap : Texture
{
    public Cubemap(): base(GL.TEXTURE_CUBE_MAP, GL.CLAMP_TO_EDGE) { }


    public override void SetParams()
    {
        GL.TexParameter(GL.TEXTURE_CUBE_MAP, GL.TEXTURE_MAG_FILTER, GL.LINEAR);
        GL.TexParameter(GL.TEXTURE_CUBE_MAP, GL.TEXTURE_MIN_FILTER, GL.LINEAR);
        GL.TexParameter(GL.TEXTURE_CUBE_MAP, GL.TEXTURE_WRAP_S, GL.CLAMP_TO_EDGE);
        GL.TexParameter(GL.TEXTURE_CUBE_MAP, GL.TEXTURE_WRAP_T, GL.CLAMP_TO_EDGE);
        GL.TexParameter(GL.TEXTURE_CUBE_MAP, GL.TEXTURE_WRAP_R, GL.CLAMP_TO_EDGE);
    }


    public override void Deploy(int bufferIndex = 0)
    {
        if (Available())
        {
            Bind();
            SetParams();
            int i, l = m_buffers.Count;
            // put the available textures on the cubemap as far as possible and put the last texture on any remaining cubemap faces
            // Reguar case six textures: One texture for each cubemap face
            // Special case one textures: all cubemap faces bear the same texture
            // Special case two textures: first texture goes to first 5 cubemap faces, 2nd texture goes to 6th cubemap face. Special case for smileys with a uniform skin and a face                    
            TextureBuffer texBuf = null;
            for (i = 0; i < l; i++)
            {
                texBuf = m_buffers[i];
                GL.TexImage2D((uint) (GL.TEXTURE_CUBE_MAP_POSITIVE_X + i), 0, texBuf.m_internalFormat, texBuf.m_width, texBuf.m_height, 0, texBuf.m_format, GL.UNSIGNED_BYTE, texBuf.m_data);
            }
            for (; i < 6; i++)
                GL.TexImage2D((uint) (GL.TEXTURE_CUBE_MAP_POSITIVE_X + i), 0, texBuf.m_internalFormat, texBuf.m_width, texBuf.m_height, 0, texBuf.m_format, GL.UNSIGNED_BYTE, texBuf.m_data);
            Release();
        }
    }

}

// =================================================================================================
